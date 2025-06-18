/**
 * @file ResourceLoader.cpp
 * @brief 资源加载器实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "utils/resources/ResourceLoader.h"
#include "utils/resources/ResourceLogger.h"
#include <algorithm>
#include <sstream>

namespace Fantasy {

ResourceLoader::ResourceLoader()
    : running_(false)
    , activeThreads_(0) {
}

ResourceLoader::~ResourceLoader() {
    stopAll();
}

ResourceLoader& ResourceLoader::getInstance() {
    static ResourceLoader instance;
    return instance;
}

void ResourceLoader::registerLoader(ResourceType type, std::shared_ptr<IResourceLoader> loader) {
    if (!loader) {
        FANTASY_LOG_ERROR("Attempted to register null loader");
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    
    // 检查是否已存在相同类型的加载器
    auto it = loaders_.find(type);
    
    if (it != loaders_.end()) {
        FANTASY_LOG_WARN("Loader for type {} already registered, replacing", static_cast<int>(type));
        it->second = loader;
    } else {
        loaders_[type] = loader;
        FANTASY_LOG_INFO("Registered loader for type {}", static_cast<int>(type));
    }
}

std::shared_ptr<IResource> ResourceLoader::load(const std::filesystem::path& path, ResourceType type) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    FANTASY_LOG_DEBUG("Loading resource: {} (type: {})", path.string(), static_cast<int>(type));
    
    // 查找对应的加载器
    std::shared_ptr<IResourceLoader> loader = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = loaders_.find(type);
        if (it != loaders_.end()) {
            loader = it->second;
        }
    }
    
    if (!loader) {
        FANTASY_LOG_ERROR("No loader found for resource type {}", static_cast<int>(type));
        return nullptr;
    }
    
    // 加载资源
    auto resource = loader->load(path, type);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    if (resource) {
        FANTASY_LOG_INFO("Resource loaded successfully: {} in {}ms", path.string(), duration.count());
        
        // 记录加载统计
        ResourceLogger::getInstance().recordLoad(path.string(), type, duration.count(), true);
    } else {
        FANTASY_LOG_ERROR("Failed to load resource: {} in {}ms", path.string(), duration.count());
        
        // 记录加载失败统计
        ResourceLogger::getInstance().recordLoad(path.string(), type, duration.count(), false);
    }
    
    return resource;
}

void ResourceLoader::loadAsync(const std::filesystem::path& path,
                              ResourceType type,
                              std::function<void(std::shared_ptr<IResource>)> callback) {
    if (!running_) {
        FANTASY_LOG_ERROR("ResourceLoader not running, cannot load async");
        if (callback) callback(nullptr);
        return;
    }
    
    FANTASY_LOG_DEBUG("Queuing async load: {} (type: {})", path.string(), static_cast<int>(type));
    
    // 创建加载任务
    LoadTask task;
    task.path = path;
    task.type = type;
    task.callback = callback;
    task.cancelled = false;
    
    // 添加到队列
    {
        std::lock_guard<std::mutex> lock(mutex_);
        loadQueue_.push(task);
    }
    
    // 通知工作线程
    condition_.notify_one();
}

bool ResourceLoader::cancelAsyncLoad(const std::filesystem::path& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 查找并标记任务为已取消
    std::queue<LoadTask> tempQueue;
    bool found = false;
    
    while (!loadQueue_.empty()) {
        LoadTask task = loadQueue_.front();
        loadQueue_.pop();
        
        if (task.path == path && !task.cancelled) {
            task.cancelled = true;
            found = true;
            FANTASY_LOG_DEBUG("Cancelled async load: {}", path.string());
        }
        
        tempQueue.push(task);
    }
    
    loadQueue_ = tempQueue;
    return found;
}

void ResourceLoader::setThreadCount(size_t threadCount) {
    if (running_) {
        FANTASY_LOG_WARN("Cannot change thread count while running");
        return;
    }
    
    // 停止现有线程
    stopAll();
    
    // 启动新线程
    running_ = true;
    for (size_t i = 0; i < threadCount; ++i) {
        workerThreads_.emplace_back(&ResourceLoader::workerThread, this);
    }
    
    FANTASY_LOG_INFO("ResourceLoader started with {} threads", threadCount);
}

size_t ResourceLoader::getQueueSize() {
    std::lock_guard<std::mutex> lock(mutex_);
    return loadQueue_.size();
}

void ResourceLoader::waitForAll() {
    if (!running_) {
        return;
    }
    
    FANTASY_LOG_INFO("Waiting for all async loads to complete...");
    
    // 等待队列为空
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [this] { return loadQueue_.empty(); });
    
    FANTASY_LOG_INFO("All async loads completed");
}

void ResourceLoader::stopAll() {
    if (!running_) {
        return;
    }
    
    FANTASY_LOG_INFO("Stopping ResourceLoader...");
    
    running_ = false;
    condition_.notify_all();
    
    // 等待所有工作线程结束
    for (auto& thread : workerThreads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    workerThreads_.clear();
    
    // 清空队列
    {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!loadQueue_.empty()) {
            LoadTask& task = loadQueue_.front();
            if (task.callback) {
                task.callback(nullptr);
            }
            loadQueue_.pop();
        }
    }
    
    FANTASY_LOG_INFO("ResourceLoader stopped");
}

void ResourceLoader::workerThread() {
    activeThreads_++;
    FANTASY_LOG_DEBUG("Worker thread started (ID: {})", std::this_thread::get_id());
    
    while (running_) {
        LoadTask task;
        
        {
            std::unique_lock<std::mutex> lock(mutex_);
            condition_.wait(lock, [this] { return !running_ || !loadQueue_.empty(); });
            
            if (!running_) {
                break;
            }
            
            if (loadQueue_.empty()) {
                continue;
            }
            
            task = loadQueue_.front();
            loadQueue_.pop();
        }
        
        if (!task.cancelled) {
            processLoadTask(task);
        }
    }
    
    activeThreads_--;
    FANTASY_LOG_DEBUG("Worker thread stopped (ID: {})", std::this_thread::get_id());
}

void ResourceLoader::processLoadTask(const LoadTask& task) {
    FANTASY_LOG_DEBUG("Processing load task: {} (type: {})", task.path.string(), static_cast<int>(task.type));
    
    auto resource = load(task.path, task.type);
    
    if (task.callback) {
        task.callback(resource);
    }
}

} // namespace Fantasy

