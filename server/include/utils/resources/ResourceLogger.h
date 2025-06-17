/**
 * @file ResourceLogger.h
 * @brief 资源管理系统日志类
 * @author [pengchengkang]
 * @date 2025.06.16
 */

#pragma once

#include "include/utils/LogSys/Logger.h"
#include "ResourceType.h"
#include <string>
#include <filesystem>

namespace Fantasy {

/**
 * @brief 资源管理系统日志类
 * @details 提供资源管理系统专用的日志功能
 */
class ResourceLogger {
public:
    /**
     * @brief 获取单例实例
     * @return ResourceLogger实例引用
     */
    static ResourceLogger& getInstance();

    /**
     * @brief 初始化日志系统
     * @param logPath 日志文件路径
     * @return 是否初始化成功
     */
    bool initialize(const std::filesystem::path& logPath);

    /**
     * @brief 记录资源加载日志
     * @param type 资源类型
     * @param path 资源路径
     * @param success 是否成功
     * @param message 附加信息
     */
    void logResourceLoad(ResourceType type, 
                        const std::filesystem::path& path,
                        bool success,
                        const std::string& message = "");

    /**
     * @brief 记录资源卸载日志
     * @param type 资源类型
     * @param path 资源路径
     * @param success 是否成功
     * @param message 附加信息
     */
    void logResourceUnload(ResourceType type,
                          const std::filesystem::path& path,
                          bool success,
                          const std::string& message = "");

    /**
     * @brief 记录资源重载日志
     * @param type 资源类型
     * @param path 资源路径
     * @param success 是否成功
     * @param message 附加信息
     */
    void logResourceReload(ResourceType type,
                          const std::filesystem::path& path,
                          bool success,
                          const std::string& message = "");

    /**
     * @brief 记录缓存操作日志
     * @param operation 操作类型
     * @param resourceId 资源ID
     * @param success 是否成功
     * @param message 附加信息
     */
    void logCacheOperation(const std::string& operation,
                          const std::string& resourceId,
                          bool success,
                          const std::string& message = "");

    /**
     * @brief 记录资源统计信息
     * @param stats 统计信息
     */
    void logResourceStats(const std::string& stats);

    /**
     * @brief 记录资源错误
     * @param type 资源类型
     * @param path 资源路径
     * @param error 错误信息
     */
    void logResourceError(ResourceType type,
                         const std::filesystem::path& path,
                         const std::string& error);

    /**
     * @brief 记录资源警告
     * @param type 资源类型
     * @param path 资源路径
     * @param warning 警告信息
     */
    void logResourceWarning(ResourceType type,
                          const std::filesystem::path& path,
                          const std::string& warning);

    /**
     * @brief 记录资源依赖关系
     * @param resourceId 资源ID
     * @param dependencyId 依赖资源ID
     * @param type 依赖类型
     */
    void logResourceDependency(const std::string& resourceId,
                              const std::string& dependencyId,
                              const std::string& type);

    /**
     * @brief 记录资源性能信息
     * @param type 资源类型
     * @param path 资源路径
     * @param loadTime 加载时间（毫秒）
     * @param memoryUsage 内存使用（字节）
     */
    void logResourcePerformance(ResourceType type,
                               const std::filesystem::path& path,
                               uint64_t loadTime,
                               uint64_t memoryUsage);

    /**
     * @brief 记录资源加载统计
     * @param path 资源路径
     * @param type 资源类型
     * @param loadTime 加载时间（毫秒）
     * @param success 是否成功
     */
    void recordLoad(const std::string& path, ResourceType type, uint64_t loadTime, bool success);

    /**
     * @brief 记录缓存放入统计
     * @param resourceId 资源ID
     * @param type 资源类型
     * @param size 资源大小
     */
    void recordCachePut(const std::string& resourceId, ResourceType type, std::uintmax_t size);

    /**
     * @brief 记录缓存命中统计
     * @param resourceId 资源ID
     * @param type 资源类型
     */
    void recordCacheHit(const std::string& resourceId, ResourceType type);

    /**
     * @brief 记录缓存未命中统计
     * @param resourceId 资源ID
     * @param type 资源类型
     */
    void recordCacheMiss(const std::string& resourceId, ResourceType type);

private:
    ResourceLogger() = default;
    ~ResourceLogger() = default;

    // 禁用拷贝和移动
    ResourceLogger(const ResourceLogger&) = delete;
    ResourceLogger& operator=(const ResourceLogger&) = delete;
    ResourceLogger(ResourceLogger&&) = delete;
    ResourceLogger& operator=(ResourceLogger&&) = delete;

    /**
     * @brief 获取资源类型字符串
     * @param type 资源类型
     * @return 资源类型字符串
     */
    std::string getResourceTypeString(ResourceType type) const;

    /**
     * @brief 格式化资源路径
     * @param path 资源路径
     * @return 格式化后的路径字符串
     */
    std::string formatResourcePath(const std::filesystem::path& path) const;

    bool initialized_ = false;                    ///< 初始化标志
    std::filesystem::path logPath_;              ///< 日志文件路径
};

// 定义便捷的日志宏
#define RESOURCE_LOG_LOAD(type, path, success, ...) \
    ResourceLogger::getInstance().logResourceLoad(type, path, success, ##__VA_ARGS__)

#define RESOURCE_LOG_UNLOAD(type, path, success, ...) \
    ResourceLogger::getInstance().logResourceUnload(type, path, success, ##__VA_ARGS__)

#define RESOURCE_LOG_RELOAD(type, path, success, ...) \
    ResourceLogger::getInstance().logResourceReload(type, path, success, ##__VA_ARGS__)

#define RESOURCE_LOG_CACHE(op, id, success, ...) \
    ResourceLogger::getInstance().logCacheOperation(op, id, success, ##__VA_ARGS__)

#define RESOURCE_LOG_STATS(stats) \
    ResourceLogger::getInstance().logResourceStats(stats)

#define RESOURCE_LOG_ERROR(type, path, error) \
    ResourceLogger::getInstance().logResourceError(type, path, error)

#define RESOURCE_LOG_WARNING(type, path, warning) \
    ResourceLogger::getInstance().logResourceWarning(type, path, warning)

#define RESOURCE_LOG_DEPENDENCY(resId, depId, type) \
    ResourceLogger::getInstance().logResourceDependency(resId, depId, type)

#define RESOURCE_LOG_PERFORMANCE(type, path, loadTime, memoryUsage) \
    ResourceLogger::getInstance().logResourcePerformance(type, path, loadTime, memoryUsage)

} // namespace Fantasy 