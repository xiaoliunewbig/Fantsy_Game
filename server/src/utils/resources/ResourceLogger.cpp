/**
 * @file ResourceLogger.cpp
 * @brief 资源管理系统日志类实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "include/utils/resources/ResourceLogger.h"
#include <sstream>
#include <iomanip>

namespace Fantasy {

ResourceLogger& ResourceLogger::getInstance() {
    static ResourceLogger instance;
    return instance;
}

bool ResourceLogger::initialize(const std::filesystem::path& logPath) {
    if (initialized_) {
        FANTASY_LOG_WARN("ResourceLogger already initialized");
        return true;
    }
    
    logPath_ = logPath;
    
    // 确保日志目录存在
    auto logDir = logPath_.parent_path();
    if (!logDir.empty() && !std::filesystem::exists(logDir)) {
        if (!std::filesystem::create_directories(logDir)) {
            FANTASY_LOG_ERROR("Failed to create log directory: {}", logDir.string());
            return false;
        }
    }
    
    initialized_ = true;
    FANTASY_LOG_INFO("ResourceLogger initialized with log path: {}", logPath_.string());
    
    return true;
}

void ResourceLogger::logResourceLoad(ResourceType type, 
                                   const std::filesystem::path& path,
                                   bool success,
                                   const std::string& message) {
    if (!initialized_) return;
    
    std::string typeStr = getResourceTypeString(type);
    std::string pathStr = formatResourcePath(path);
    std::string status = success ? "SUCCESS" : "FAILED";
    
    FANTASY_LOG_INFO("Resource Load [{}] {} - {}: {}", typeStr, status, pathStr, message);
}

void ResourceLogger::logResourceUnload(ResourceType type,
                                     const std::filesystem::path& path,
                                     bool success,
                                     const std::string& message) {
    if (!initialized_) return;
    
    std::string typeStr = getResourceTypeString(type);
    std::string pathStr = formatResourcePath(path);
    std::string status = success ? "SUCCESS" : "FAILED";
    
    FANTASY_LOG_INFO("Resource Unload [{}] {} - {}: {}", typeStr, status, pathStr, message);
}

void ResourceLogger::logResourceReload(ResourceType type,
                                     const std::filesystem::path& path,
                                     bool success,
                                     const std::string& message) {
    if (!initialized_) return;
    
    std::string typeStr = getResourceTypeString(type);
    std::string pathStr = formatResourcePath(path);
    std::string status = success ? "SUCCESS" : "FAILED";
    
    FANTASY_LOG_INFO("Resource Reload [{}] {} - {}: {}", typeStr, status, pathStr, message);
}

void ResourceLogger::logCacheOperation(const std::string& operation,
                                     const std::string& resourceId,
                                     bool success,
                                     const std::string& message) {
    if (!initialized_) return;
    
    std::string status = success ? "SUCCESS" : "FAILED";
    
    FANTASY_LOG_DEBUG("Cache {} [{}] {}: {}", operation, status, resourceId, message);
}

void ResourceLogger::logResourceStats(const std::string& stats) {
    if (!initialized_) return;
    
    FANTASY_LOG_INFO("Resource Statistics:\n{}", stats);
}

void ResourceLogger::logResourceError(ResourceType type,
                                    const std::filesystem::path& path,
                                    const std::string& error) {
    if (!initialized_) return;
    
    std::string typeStr = getResourceTypeString(type);
    std::string pathStr = formatResourcePath(path);
    
    FANTASY_LOG_ERROR("Resource Error [{}] {}: {}", typeStr, pathStr, error);
}

void ResourceLogger::logResourceWarning(ResourceType type,
                                      const std::filesystem::path& path,
                                      const std::string& warning) {
    if (!initialized_) return;
    
    std::string typeStr = getResourceTypeString(type);
    std::string pathStr = formatResourcePath(path);
    
    FANTASY_LOG_WARN("Resource Warning [{}] {}: {}", typeStr, pathStr, warning);
}

void ResourceLogger::logResourceDependency(const std::string& resourceId,
                                         const std::string& dependencyId,
                                         const std::string& type) {
    if (!initialized_) return;
    
    FANTASY_LOG_DEBUG("Resource Dependency: {} -> {} ({})", resourceId, dependencyId, type);
}

void ResourceLogger::logResourcePerformance(ResourceType type,
                                          const std::filesystem::path& path,
                                          uint64_t loadTime,
                                          uint64_t memoryUsage) {
    if (!initialized_) return;
    
    std::string typeStr = getResourceTypeString(type);
    std::string pathStr = formatResourcePath(path);
    
    FANTASY_LOG_DEBUG("Resource Performance [{}] {}: {}ms, {}MB", 
                      typeStr, pathStr, loadTime, memoryUsage / (1024 * 1024));
}

void ResourceLogger::recordLoad(const std::string& path, ResourceType type, uint64_t loadTime, bool success) {
    if (!initialized_) return;
    
    std::string typeStr = getResourceTypeString(type);
    std::string status = success ? "SUCCESS" : "FAILED";
    
    FANTASY_LOG_DEBUG("Load Record: {} [{}] {}ms - {}", path, typeStr, loadTime, status);
}

void ResourceLogger::recordCachePut(const std::string& resourceId, ResourceType type, std::uintmax_t size) {
    if (!initialized_) return;
    
    std::string typeStr = getResourceTypeString(type);
    
    FANTASY_LOG_DEBUG("Cache Put: {} [{}] {}MB", resourceId, typeStr, size / (1024 * 1024));
}

void ResourceLogger::recordCacheHit(const std::string& resourceId, ResourceType type) {
    if (!initialized_) return;
    
    std::string typeStr = getResourceTypeString(type);
    
    FANTASY_LOG_DEBUG("Cache Hit: {} [{}]", resourceId, typeStr);
}

void ResourceLogger::recordCacheMiss(const std::string& resourceId, ResourceType type) {
    if (!initialized_) return;
    
    std::string typeStr = getResourceTypeString(type);
    
    FANTASY_LOG_DEBUG("Cache Miss: {} [{}]", resourceId, typeStr);
}

std::string ResourceLogger::getResourceTypeString(ResourceType type) const {
    switch (type) {
        case ResourceType::CONFIG: return "CONFIG";
        case ResourceType::MAP: return "MAP";
        case ResourceType::SCRIPT: return "SCRIPT";
        case ResourceType::TEXTURE: return "TEXTURE";
        case ResourceType::MODEL: return "MODEL";
        case ResourceType::SOUND: return "SOUND";
        case ResourceType::MUSIC: return "MUSIC";
        case ResourceType::FONT: return "FONT";
        case ResourceType::SHADER: return "SHADER";
        case ResourceType::ANIMATION: return "ANIMATION";
        case ResourceType::UNKNOWN: return "UNKNOWN";
        default: return "UNKNOWN";
    }
}

std::string ResourceLogger::formatResourcePath(const std::filesystem::path& path) const {
    // 将路径转换为字符串，并限制长度
    std::string pathStr = path.string();
    
    // 如果路径太长，截断并添加省略号
    const size_t maxLength = 80;
    if (pathStr.length() > maxLength) {
        pathStr = "..." + pathStr.substr(pathStr.length() - maxLength + 3);
    }
    
    return pathStr;
}

} // namespace Fantasy 