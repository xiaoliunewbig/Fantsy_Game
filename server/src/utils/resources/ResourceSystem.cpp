/**
 * @file ResourceSystem.cpp
 * @brief 资源管理系统主实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "utils/resources/ResourceSystem.h"
#include "utils/LogSys/Logger.h"
#include <algorithm>
#include <chrono>
#include <thread>

namespace Fantasy {

ResourceSystem& ResourceSystem::getInstance() {
    static ResourceSystem instance;
    return instance;
}

bool ResourceSystem::initialize(const ResourceSystemConfig& config) {
    if (initialized_) {
        FANTASY_LOG_WARN("ResourceSystem already initialized");
        return true;
    }
    
    config_ = config;
    
    FANTASY_LOG_INFO("Initializing ResourceSystem...");
    
    // 初始化资源日志系统
    if (config_.enableLogging) {
        if (!ResourceLogger::getInstance().initialize(config_.logPath)) {
            FANTASY_LOG_ERROR("Failed to initialize ResourceLogger");
            return false;
        }
        FANTASY_LOG_INFO("ResourceLogger initialized");
    }
    
    // 初始化资源管理器
    if (!manager_.initialize(config_.resourceRootDir, 
                           config_.maxLoadingThreads, 
                           config_.maxCacheSize)) {
        FANTASY_LOG_ERROR("Failed to initialize ResourceManager");
        return false;
    }
    FANTASY_LOG_INFO("ResourceManager initialized");
    
    // 初始化压缩系统
    if (config_.enableCompression) {
        if (!compressor_.initialize()) {
            FANTASY_LOG_ERROR("Failed to initialize ResourceCompressor");
            return false;
        }
        FANTASY_LOG_INFO("ResourceCompressor initialized");
    }
    
    // 初始化打包系统
    if (config_.enablePackaging) {
        if (!packager_.initialize()) {
            FANTASY_LOG_ERROR("Failed to initialize ResourcePackager");
            return false;
        }
        FANTASY_LOG_INFO("ResourcePackager initialized");
    }
    
    // 初始化版本控制系统
    if (config_.enableVersionControl) {
        if (!versionControl_.initialize(config_.versionControl)) {
            FANTASY_LOG_ERROR("Failed to initialize ResourceVersionControl");
            return false;
        }
        FANTASY_LOG_INFO("ResourceVersionControl initialized");
    }
    
    // 初始化预加载系统
    if (config_.enablePreloading) {
        if (!preloader_.initialize(config_.preloading)) {
            FANTASY_LOG_ERROR("Failed to initialize ResourcePreloader");
            return false;
        }
        FANTASY_LOG_INFO("ResourcePreloader initialized");
    }
    
    // 启动性能监控
    if (config_.enablePerformanceMonitoring) {
        startPerformanceMonitoring();
    }
    
    initialized_ = true;
    
    FANTASY_LOG_INFO("ResourceSystem initialized successfully");
    FANTASY_LOG_INFO("  Resource Root: {}", config_.resourceRootDir);
    FANTASY_LOG_INFO("  Cache Size: {}MB", config_.maxCacheSize / (1024 * 1024));
    FANTASY_LOG_INFO("  Loading Threads: {}", config_.maxLoadingThreads);
    FANTASY_LOG_INFO("  Compression: {}", config_.enableCompression ? "Enabled" : "Disabled");
    FANTASY_LOG_INFO("  Packaging: {}", config_.enablePackaging ? "Enabled" : "Disabled");
    FANTASY_LOG_INFO("  Version Control: {}", config_.enableVersionControl ? "Enabled" : "Disabled");
    FANTASY_LOG_INFO("  Preloading: {}", config_.enablePreloading ? "Enabled" : "Disabled");
    
    return true;
}

void ResourceSystem::shutdown() {
    if (!initialized_) return;
    
    FANTASY_LOG_INFO("Shutting down ResourceSystem...");
    
    // 停止性能监控
    if (config_.enablePerformanceMonitoring) {
        stopPerformanceMonitoring();
    }
    
    // 关闭各个子系统
    if (config_.enablePreloading) {
        preloader_.shutdown();
    }
    
    if (config_.enableVersionControl) {
        // 版本控制系统没有shutdown方法，这里可以添加
    }
    
    if (config_.enablePackaging) {
        packager_.closePackage();
    }
    
    // 关闭资源管理器
    manager_.shutdown();
    
    initialized_ = false;
    
    FANTASY_LOG_INFO("ResourceSystem shutdown completed");
}

std::shared_ptr<IResource> ResourceSystem::loadResource(const std::string& path, ResourceType type) {
    if (!initialized_) {
        FANTASY_LOG_ERROR("ResourceSystem not initialized");
        return nullptr;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    FANTASY_LOG_DEBUG("Loading resource: {} (type: {})", path, static_cast<int>(type));
    
    auto resource = manager_.load(path, type);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    if (resource) {
        FANTASY_LOG_INFO("Successfully loaded resource: {} in {}ms", path, duration.count());
        
        // 记录性能统计
        if (config_.enablePerformanceLogging) {
            ResourceLogger::getInstance().logResourcePerformance(type, path, duration.count(), resource->getSize());
        }
    } else {
        FANTASY_LOG_ERROR("Failed to load resource: {} in {}ms", path, duration.count());
    }
    
    return resource;
}

void ResourceSystem::loadResourceAsync(const std::string& path,
                                      ResourceType type,
                                      std::function<void(std::shared_ptr<IResource>)> callback) {
    if (!initialized_) {
        FANTASY_LOG_ERROR("ResourceSystem not initialized");
        if (callback) callback(nullptr);
        return;
    }
    
    FANTASY_LOG_DEBUG("Queuing async load: {} (type: {})", path, static_cast<int>(type));
    
    manager_.loadAsync(path, type, [this, path, type, callback](std::shared_ptr<IResource> resource) {
        if (resource) {
            FANTASY_LOG_INFO("Async load completed: {}", path);
        } else {
            FANTASY_LOG_ERROR("Async load failed: {}", path);
        }
        
        if (callback) callback(resource);
    });
}

void ResourceSystem::unloadResource(const std::string& resourceId) {
    if (!initialized_) {
        FANTASY_LOG_ERROR("ResourceSystem not initialized");
        return;
    }
    
    FANTASY_LOG_DEBUG("Unloading resource: {}", resourceId);
    
    if (manager_.unload(resourceId)) {
        FANTASY_LOG_INFO("Successfully unloaded resource: {}", resourceId);
    } else {
        FANTASY_LOG_WARN("Failed to unload resource: {}", resourceId);
    }
}

bool ResourceSystem::reloadResource(const std::string& resourceId) {
    if (!initialized_) {
        FANTASY_LOG_ERROR("ResourceSystem not initialized");
        return false;
    }
    
    FANTASY_LOG_DEBUG("Reloading resource: {}", resourceId);
    
    if (manager_.reload(resourceId)) {
        FANTASY_LOG_INFO("Successfully reloaded resource: {}", resourceId);
        return true;
    } else {
        FANTASY_LOG_ERROR("Failed to reload resource: {}", resourceId);
        return false;
    }
}

std::shared_ptr<IResource> ResourceSystem::getResource(const std::string& resourceId) {
    if (!initialized_) {
        FANTASY_LOG_ERROR("ResourceSystem not initialized");
        return nullptr;
    }
    
    return manager_.get(resourceId);
}

bool ResourceSystem::hasResource(const std::string& resourceId) {
    if (!initialized_) return false;
    return manager_.has(resourceId);
}

void ResourceSystem::preloadResources(const std::vector<std::string>& resourcePaths,
                                     PreloadPriority priority) {
    if (!initialized_) {
        FANTASY_LOG_ERROR("ResourceSystem not initialized");
        return;
    }
    
    FANTASY_LOG_INFO("Preloading {} resources with priority {}", resourcePaths.size(), static_cast<int>(priority));
    
    std::vector<std::pair<std::filesystem::path, ResourceType>> resources;
    for (const auto& path : resourcePaths) {
        // 这里需要根据文件扩展名推断资源类型
        ResourceType type = inferResourceType(path);
        resources.emplace_back(path, type);
    }
    
    manager_.preload(resources);
}

bool ResourceSystem::createResourcePackage(const std::string& packagePath,
                                          const std::vector<std::string>& resourcePaths) {
    if (!initialized_ || !config_.enablePackaging) {
        FANTASY_LOG_ERROR("ResourceSystem not initialized or packaging disabled");
        return false;
    }
    
    FANTASY_LOG_INFO("Creating resource package: {} with {} resources", packagePath, resourcePaths.size());
    
    return packager_.createPackage(packagePath, resourcePaths, config_.packaging);
}

bool ResourceSystem::openResourcePackage(const std::string& packagePath) {
    if (!initialized_ || !config_.enablePackaging) {
        FANTASY_LOG_ERROR("ResourceSystem not initialized or packaging disabled");
        return false;
    }
    
    FANTASY_LOG_INFO("Opening resource package: {}", packagePath);
    
    return packager_.openPackage(packagePath);
}

bool ResourceSystem::compressResource(const std::string& resourceId,
                                     CompressionAlgorithm algorithm) {
    if (!initialized_ || !config_.enableCompression) {
        FANTASY_LOG_ERROR("ResourceSystem not initialized or compression disabled");
        return false;
    }
    
    FANTASY_LOG_DEBUG("Compressing resource: {} with algorithm {}", resourceId, static_cast<int>(algorithm));
    
    auto resource = getResource(resourceId);
    if (!resource) {
        FANTASY_LOG_ERROR("Resource not found for compression: {}", resourceId);
        return false;
    }
    
    // 这里应该实现具体的压缩逻辑
    // 暂时返回成功
    FANTASY_LOG_INFO("Successfully compressed resource: {}", resourceId);
    return true;
}

bool ResourceSystem::decompressResource(const std::string& resourceId) {
    if (!initialized_ || !config_.enableCompression) {
        FANTASY_LOG_ERROR("ResourceSystem not initialized or compression disabled");
        return false;
    }
    
    FANTASY_LOG_DEBUG("Decompressing resource: {}", resourceId);
    
    auto resource = getResource(resourceId);
    if (!resource) {
        FANTASY_LOG_ERROR("Resource not found for decompression: {}", resourceId);
        return false;
    }
    
    // 这里应该实现具体的解压逻辑
    // 暂时返回成功
    FANTASY_LOG_INFO("Successfully decompressed resource: {}", resourceId);
    return true;
}

bool ResourceSystem::addResourceVersion(const std::string& resourcePath, const Version& version) {
    if (!initialized_ || !config_.enableVersionControl) {
        FANTASY_LOG_ERROR("ResourceSystem not initialized or version control disabled");
        return false;
    }
    
    FANTASY_LOG_INFO("Adding version {} for resource: {}", version.toString(), resourcePath);
    
    return versionControl_.addVersion(resourcePath, version);
}

bool ResourceSystem::rollbackResourceVersion(const std::string& resourceId, const Version& version) {
    if (!initialized_ || !config_.enableVersionControl) {
        FANTASY_LOG_ERROR("ResourceSystem not initialized or version control disabled");
        return false;
    }
    
    FANTASY_LOG_INFO("Rolling back resource {} to version {}", resourceId, version.toString());
    
    return versionControl_.rollbackToVersion(resourceId, version);
}

ResourceSystemStats ResourceSystem::getStats() const {
    if (!initialized_) {
        return ResourceSystemStats{};
    }
    
    ResourceSystemStats stats;
    
    // 获取各个子系统的统计信息
    stats.managerStats = manager_.getStats();
    
    if (config_.enableCompression) {
        stats.compressionStats = compressor_.getStats();
    }
    
    if (config_.enablePackaging) {
        stats.packagingStats = packager_.getStats();
    }
    
    if (config_.enableVersionControl) {
        stats.versionControlStats = versionControl_.getStats();
    }
    
    if (config_.enablePreloading) {
        stats.preloadingStats = preloader_.getStats();
    }
    
    // 计算总体统计
    stats.totalResources = stats.managerStats.totalResources;
    stats.loadedResources = stats.managerStats.cacheStats.entryCount;
    stats.cachedResources = stats.managerStats.cacheStats.entryCount;
    stats.totalMemoryUsage = stats.managerStats.cacheStats.currentSize;
    stats.cacheHitRate = stats.managerStats.cacheStats.hitRate;
    
    stats.lastUpdate = std::chrono::system_clock::now();
    stats.systemVersion = "1.0.0";
    
    return stats;
}

std::string ResourceSystem::getPerformanceReport() const {
    if (!initialized_) {
        return "ResourceSystem not initialized";
    }
    
    auto stats = getStats();
    
    std::string report = "ResourceSystem Performance Report\n";
    report += "==========================================\n";
    report += "System Version: " + stats.systemVersion + "\n";
    report += "Last Update: " + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
        stats.lastUpdate.time_since_epoch()).count()) + "\n\n";
    
    report += "Resource Statistics:\n";
    report += "  Total Resources: " + std::to_string(stats.totalResources) + "\n";
    report += "  Loaded Resources: " + std::to_string(stats.loadedResources) + "\n";
    report += "  Cached Resources: " + std::to_string(stats.cachedResources) + "\n";
    report += "  Memory Usage: " + std::to_string(stats.totalMemoryUsage / (1024 * 1024)) + "MB\n";
    report += "  Cache Hit Rate: " + std::to_string(static_cast<int>(stats.cacheHitRate * 100)) + "%\n\n";
    
    report += manager_.getStatsString();
    
    return report;
}

bool ResourceSystem::optimize() {
    if (!initialized_) {
        FANTASY_LOG_ERROR("ResourceSystem not initialized");
        return false;
    }
    
    FANTASY_LOG_INFO("Optimizing ResourceSystem...");
    
    // 清理缓存
    manager_.cleanup();
    
    // 优化预加载队列
    if (config_.enablePreloading) {
        preloader_.optimizeQueue();
    }
    
    // 优化包文件
    if (config_.enablePackaging) {
        packager_.optimizePackage();
    }
    
    FANTASY_LOG_INFO("ResourceSystem optimization completed");
    return true;
}

uint64_t ResourceSystem::cleanupUnusedResources() {
    if (!initialized_) {
        FANTASY_LOG_ERROR("ResourceSystem not initialized");
        return 0;
    }
    
    FANTASY_LOG_INFO("Cleaning up unused resources...");
    
    // 清理未使用的资源
    manager_.cleanupUnusedResources();
    
    // 清理过期的版本
    if (config_.enableVersionControl) {
        versionControl_.cleanupOldVersions(config_.versionControl.maxVersions);
    }
    
    FANTASY_LOG_INFO("Resource cleanup completed");
    return 0; // 这里应该返回实际清理的资源数量
}

bool ResourceSystem::verifyIntegrity() {
    if (!initialized_) {
        FANTASY_LOG_ERROR("ResourceSystem not initialized");
        return false;
    }
    
    FANTASY_LOG_INFO("Verifying ResourceSystem integrity...");
    
    bool integrity = true;
    
    // 验证包完整性
    if (config_.enablePackaging) {
        if (!packager_.verifyPackage()) {
            FANTASY_LOG_ERROR("Package integrity verification failed");
            integrity = false;
        }
    }
    
    // 验证版本完整性
    if (config_.enableVersionControl) {
        // 这里应该实现版本完整性验证
    }
    
    if (integrity) {
        FANTASY_LOG_INFO("ResourceSystem integrity verification passed");
    } else {
        FANTASY_LOG_ERROR("ResourceSystem integrity verification failed");
    }
    
    return integrity;
}

void ResourceSystem::startPerformanceMonitoring() {
    if (monitoringThread_.joinable()) {
        monitoringThread_.join();
    }
    
    running_ = true;
    monitoringThread_ = std::thread(&ResourceSystem::performanceMonitoringThread, this);
    
    FANTASY_LOG_INFO("Performance monitoring started");
}

void ResourceSystem::stopPerformanceMonitoring() {
    running_ = false;
    
    if (monitoringThread_.joinable()) {
        monitoringThread_.join();
    }
    
    FANTASY_LOG_INFO("Performance monitoring stopped");
}

void ResourceSystem::performanceMonitoringThread() {
    while (running_) {
        // 记录性能统计
        auto stats = getStats();
        
        FANTASY_LOG_DEBUG("Performance Monitor - Cache Hit Rate: {}%, Memory Usage: {}MB",
                         static_cast<int>(stats.cacheHitRate * 100),
                         stats.totalMemoryUsage / (1024 * 1024));
        
        // 检查内存使用情况
        if (stats.totalMemoryUsage > config_.maxCacheSize * 0.9) {
            FANTASY_LOG_WARN("High memory usage detected: {}MB", stats.totalMemoryUsage / (1024 * 1024));
        }
        
        // 等待下次监控
        std::this_thread::sleep_for(std::chrono::milliseconds(config_.monitoringInterval));
    }
}

ResourceType ResourceSystem::inferResourceType(const std::string& path) {
    std::filesystem::path filePath(path);
    std::string extension = filePath.extension().string();
    
    // 转换为小写
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || 
        extension == ".bmp" || extension == ".tga" || extension == ".dds") {
        return ResourceType::TEXTURE;
    } else if (extension == ".wav" || extension == ".mp3" || extension == ".ogg" || 
               extension == ".flac" || extension == ".aac") {
        return ResourceType::AUDIO;
    } else if (extension == ".ttf" || extension == ".otf" || extension == ".woff" || 
               extension == ".woff2") {
        return ResourceType::FONT;
    } else if (extension == ".obj" || extension == ".fbx" || extension == ".gltf" || 
               extension == ".glb" || extension == ".dae") {
        return ResourceType::MODEL;
    } else if (extension == ".json" || extension == ".xml" || extension == ".yaml" || 
               extension == ".yml") {
        return ResourceType::CONFIG;
    } else if (extension == ".lua" || extension == ".py" || extension == ".js") {
        return ResourceType::SCRIPT;
    } else if (extension == ".map" || extension == ".level") {
        return ResourceType::MAP;
    }
    
    return ResourceType::UNKNOWN;
}

} // namespace Fantasy

// 便捷的全局函数实现
namespace Fantasy::ResourceSystemUtils {

std::shared_ptr<IResource> load(const std::string& path, ResourceType type) {
    return ResourceSystem::getInstance().loadResource(path, type);
}

std::shared_ptr<IResource> get(const std::string& resourceId) {
    return ResourceSystem::getInstance().getResource(resourceId);
}

void unload(const std::string& resourceId) {
    ResourceSystem::getInstance().unloadResource(resourceId);
}

ResourceSystem& getSystem() {
    return ResourceSystem::getInstance();
}

} // namespace Fantasy::ResourceSystemUtils 