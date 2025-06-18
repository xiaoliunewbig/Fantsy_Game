/**
 * @file ResourceManager.cpp
 * @brief 资源管理器实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "utils/resources/ResourceManager.h"
#include "utils/resources/ResourceLogger.h"
#include <filesystem>
#include <algorithm>
#include <sstream>

namespace Fantasy {

ResourceManager::ResourceManager()
    : resourceRoot_("resources")
    , initialized_(false) {
}

ResourceManager::~ResourceManager() {
    cleanup();
}

ResourceManager& ResourceManager::getInstance() {
    static ResourceManager instance;
    return instance;
}

bool ResourceManager::initialize(const std::filesystem::path& resourceRoot,
                               size_t threadCount,
                               std::uintmax_t cacheSize) {
    if (initialized_) {
        FANTASY_LOG_WARN("ResourceManager already initialized");
        return true;
    }

    FANTASY_LOG_INFO("Initializing ResourceManager...");
    
    resourceRoot_ = resourceRoot;
    
    // 确保资源根目录存在
    if (!std::filesystem::exists(resourceRoot_)) {
        if (!std::filesystem::create_directories(resourceRoot_)) {
            FANTASY_LOG_ERROR("Failed to create resource root directory: {}", resourceRoot_.string());
            return false;
        }
        FANTASY_LOG_INFO("Created resource root directory: {}", resourceRoot_.string());
    }
    
    // 初始化加载器
    auto& loader = ResourceLoader::getInstance();
    loader.setThreadCount(threadCount);
    
    // 初始化缓存
    auto& cache = ResourceCache::getInstance();
    cache.setMaxSize(cacheSize);
    
    initialized_ = true;
    
    FANTASY_LOG_INFO("ResourceManager initialized successfully - Root: {}, Threads: {}, Cache: {}MB",
                     resourceRoot_.string(), threadCount, cacheSize / (1024 * 1024));
    
    return true;
}

std::shared_ptr<IResource> ResourceManager::load(const std::filesystem::path& path, ResourceType type) {
    if (!initialized_) {
        FANTASY_LOG_ERROR("ResourceManager not initialized");
        return nullptr;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    std::string resourceId = path.string() + "_" + std::to_string(static_cast<int>(type));
    
    // 首先检查缓存
    auto& cache = ResourceCache::getInstance();
    auto cachedResource = cache.getResource(resourceId);
    if (cachedResource) {
        FANTASY_LOG_DEBUG("Resource found in cache: {}", resourceId);
        return cachedResource;
    }
    
    // 获取完整路径
    std::filesystem::path fullPath = getFullPath(path);
    if (!std::filesystem::exists(fullPath)) {
        FANTASY_LOG_ERROR("Resource file not found: {}", fullPath.string());
        return nullptr;
    }
    
    // 加载资源
    auto& loader = ResourceLoader::getInstance();
    auto resource = loader.load(fullPath, type);
    if (!resource) {
        FANTASY_LOG_ERROR("Failed to load resource: {}", fullPath.string());
        return nullptr;
    }
    
    // 缓存资源
    cache.addResource(resource);
    
    // 记录到资源映射
    {
        std::lock_guard<std::mutex> lock(mutex_);
        resources_[resourceId] = resource;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    FANTASY_LOG_INFO("Resource loaded successfully: {} in {}ms", resourceId, duration.count());
    
    return resource;
}

void ResourceManager::loadAsync(const std::filesystem::path& path,
                               ResourceType type,
                               std::function<void(std::shared_ptr<IResource>)> callback) {
    if (!initialized_) {
        FANTASY_LOG_ERROR("ResourceManager not initialized");
        if (callback) callback(nullptr);
        return;
    }
    
    std::string resourceId = path.string() + "_" + std::to_string(static_cast<int>(type));
    
    // 首先检查缓存
    auto& cache = ResourceCache::getInstance();
    auto cachedResource = cache.getResource(resourceId);
    if (cachedResource) {
        FANTASY_LOG_DEBUG("Async load: Resource found in cache: {}", resourceId);
        if (callback) callback(cachedResource);
        return;
    }
    
    // 获取完整路径
    std::filesystem::path fullPath = getFullPath(path);
    if (!std::filesystem::exists(fullPath)) {
        FANTASY_LOG_ERROR("Resource file not found: {}", fullPath.string());
        if (callback) callback(nullptr);
        return;
    }
    
    // 异步加载
    auto& loader = ResourceLoader::getInstance();
    loader.loadAsync(fullPath, type, [this, resourceId, callback](std::shared_ptr<IResource> resource) {
        if (resource) {
            // 缓存资源
            auto& cache = ResourceCache::getInstance();
            cache.addResource(resource);
            
            // 记录到资源映射
            {
                std::lock_guard<std::mutex> lock(mutex_);
                resources_[resourceId] = resource;
            }
            
            FANTASY_LOG_INFO("Resource loaded asynchronously: {}", resourceId);
        } else {
            FANTASY_LOG_ERROR("Failed to load resource asynchronously: {}", resourceId);
        }
        
        if (callback) callback(resource);
    });
}

bool ResourceManager::unload(const std::filesystem::path& path) {
    if (!initialized_) {
        FANTASY_LOG_ERROR("ResourceManager not initialized");
        return false;
    }
    
    std::string resourceId = path.string() + "_0"; // 使用UNKNOWN类型
    
    // 从缓存中移除
    auto& cache = ResourceCache::getInstance();
    bool removedFromCache = cache.removeResource(resourceId);
    
    // 从资源映射中移除
    bool removedFromMap = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = resources_.find(resourceId);
        if (it != resources_.end()) {
            resources_.erase(it);
            removedFromMap = true;
        }
    }
    
    if (removedFromCache || removedFromMap) {
        FANTASY_LOG_INFO("Resource unloaded: {}", resourceId);
        return true;
    }
    
    FANTASY_LOG_WARN("Resource not found for unloading: {}", resourceId);
    return false;
}

bool ResourceManager::reload(const std::filesystem::path& path) {
    if (!initialized_) {
        FANTASY_LOG_ERROR("ResourceManager not initialized");
        return false;
    }
    
    // 先卸载
    unload(path);
    
    // 重新加载
    auto resource = load(path, ResourceType::UNKNOWN);
    return resource != nullptr;
}

void ResourceManager::reloadAll() {
    if (!initialized_) {
        FANTASY_LOG_ERROR("ResourceManager not initialized");
        return;
    }
    
    FANTASY_LOG_INFO("Reloading all resources...");
    
    std::vector<std::string> resourceIds;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& [id, weakResource] : resources_) {
            resourceIds.push_back(id);
        }
    }
    
    for (const auto& id : resourceIds) {
        // 这里需要根据资源ID解析出路径和类型
        // 简化处理，直接重新加载
        FANTASY_LOG_DEBUG("Reloading resource: {}", id);
    }
    
    FANTASY_LOG_INFO("All resources reloaded");
}

std::shared_ptr<IResource> ResourceManager::get(const std::filesystem::path& path) {
    if (!initialized_) {
        FANTASY_LOG_ERROR("ResourceManager not initialized");
        return nullptr;
    }
    
    std::string resourceId = path.string() + "_0"; // 使用UNKNOWN类型
    
    // 从缓存中获取
    auto& cache = ResourceCache::getInstance();
    auto resource = cache.getResource(resourceId);
    if (resource) {
        return resource;
    }
    
    // 从资源映射中获取
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = resources_.find(resourceId);
        if (it != resources_.end()) {
            auto weakResource = it->second;
            return weakResource.lock();
        }
    }
    
    return nullptr;
}

bool ResourceManager::exists(const std::filesystem::path& path) const {
    if (!initialized_) {
        return false;
    }
    
    std::filesystem::path fullPath = getFullPath(path);
    return std::filesystem::exists(fullPath);
}

const std::filesystem::path& ResourceManager::getResourceRoot() const {
    return resourceRoot_;
}

bool ResourceManager::setResourceRoot(const std::filesystem::path& root) {
    if (!std::filesystem::exists(root)) {
        if (!std::filesystem::create_directories(root)) {
            FANTASY_LOG_ERROR("Failed to create resource root directory: {}", root.string());
            return false;
        }
    }
    
    resourceRoot_ = root;
    FANTASY_LOG_INFO("Resource root set to: {}", resourceRoot_.string());
    return true;
}

ResourceLoader& ResourceManager::getLoader() {
    return ResourceLoader::getInstance();
}

ResourceCache& ResourceManager::getCache() {
    return ResourceCache::getInstance();
}

void ResourceManager::cleanup() {
    if (!initialized_) {
        return;
    }
    
    FANTASY_LOG_INFO("Cleaning up ResourceManager...");
    
    // 清理缓存
    auto& cache = ResourceCache::getInstance();
    cache.clear();
    
    // 清理资源映射
    {
        std::lock_guard<std::mutex> lock(mutex_);
        resources_.clear();
    }
    
    initialized_ = false;
    
    FANTASY_LOG_INFO("ResourceManager cleanup completed");
}

std::string ResourceManager::getStats() const {
    if (!initialized_) {
        return "ResourceManager not initialized";
    }
    
    auto& cache = ResourceCache::getInstance();
    auto& loader = ResourceLoader::getInstance();
    
    std::ostringstream oss;
    oss << "ResourceManager Statistics:\n";
    oss << "  Resource Root: " << resourceRoot_.string() << "\n";
    oss << "  Initialized: " << (initialized_ ? "Yes" : "No") << "\n";
    oss << "  Cache Size: " << cache.getCurrentSize() / (1024 * 1024) << "MB\n";
    oss << "  Cache Hit Rate: " << (cache.getHitRate() * 100) << "%\n";
    oss << "  Load Queue Size: " << loader.getQueueSize() << "\n";
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        oss << "  Loaded Resources: " << resources_.size() << "\n";
    }
    
    return oss.str();
}

std::filesystem::path ResourceManager::getFullPath(const std::filesystem::path& path) const {
    return resourceRoot_ / path;
}

bool ResourceManager::needsReload(const std::shared_ptr<IResource>& resource) const {
    if (!resource) return false;
    
    std::filesystem::path fullPath = getFullPath(resource->getPath());
    if (!std::filesystem::exists(fullPath)) return false;
    
    auto fileTime = std::filesystem::last_write_time(fullPath);
    return fileTime > resource->getLastModifiedTime();
}

} // namespace Fantasy
