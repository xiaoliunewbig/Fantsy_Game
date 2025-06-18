/**
 * @file ResourceSystem.h
 * @brief 资源管理系统主头文件
 * @author [pengchengkang]
 * @date 2025.06.16
 */

#pragma once

// 基础接口和类型
#include "ResourceType.h"
#include "ResourceCache.h"
#include "ResourceLoader.h"
#include "ResourceManager.h"
#include "ResourceLogger.h"
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace Fantasy {

/**
 * @brief 资源系统配置
 */
struct ResourceSystemConfig {
    std::string resourceRootDir;           ///< 资源根目录
    size_t maxCacheSize = 100 * 1024 * 1024; ///< 最大缓存大小（字节）
    int maxLoadingThreads = 4;             ///< 最大加载线程数
    bool enableLogging = true;             ///< 是否启用日志
    bool enablePerformanceMonitoring = true; ///< 是否启用性能监控
    bool enableCompression = false;        ///< 是否启用压缩
    bool enablePreloading = false;         ///< 是否启用预加载
};

/**
 * @brief 预加载优先级
 */
enum class PreloadPriority {
    LOW,        ///< 低优先级
    NORMAL,     ///< 普通优先级
    HIGH,       ///< 高优先级
    CRITICAL    ///< 关键优先级
};

/**
 * @brief 资源系统统计信息
 */
struct ResourceSystemStats {
    size_t totalResources = 0;             ///< 总资源数
    size_t loadedResources = 0;            ///< 已加载资源数
    size_t cachedResources = 0;            ///< 缓存资源数
    size_t totalMemoryUsage = 0;           ///< 总内存使用量
    double cacheHitRate = 0.0;             ///< 缓存命中率
};

/**
 * @brief 资源系统管理类
 * @details 负责管理游戏资源的加载、缓存、卸载等操作
 */
class ResourceSystem {
public:
    /**
     * @brief 获取单例实例
     * @return 资源系统实例
     */
    static ResourceSystem& getInstance();

    /**
     * @brief 初始化资源系统
     * @param config 系统配置
     * @return 是否初始化成功
     */
    bool initialize(const ResourceSystemConfig& config);

    /**
     * @brief 关闭资源系统
     */
    void shutdown();

    /**
     * @brief 加载资源
     * @param path 资源路径
     * @param type 资源类型
     * @return 加载的资源对象
     */
    std::shared_ptr<IResource> loadResource(const std::string& path, ResourceType type);

    /**
     * @brief 异步加载资源
     * @param path 资源路径
     * @param type 资源类型
     * @param callback 加载完成回调
     */
    void loadResourceAsync(const std::string& path, ResourceType type,
                          std::function<void(std::shared_ptr<IResource>)> callback);

    /**
     * @brief 卸载资源
     * @param resourceId 资源ID
     */
    void unloadResource(const std::string& resourceId);

    /**
     * @brief 重新加载资源
     * @param resourceId 资源ID
     * @return 是否重新加载成功
     */
    bool reloadResource(const std::string& resourceId);

    /**
     * @brief 获取资源
     * @param resourceId 资源ID
     * @return 资源对象
     */
    std::shared_ptr<IResource> getResource(const std::string& resourceId);

    /**
     * @brief 检查资源是否存在
     * @param resourceId 资源ID
     * @return 是否存在
     */
    bool hasResource(const std::string& resourceId);

    /**
     * @brief 预加载资源
     * @param resourcePaths 资源路径列表
     * @param priority 预加载优先级
     */
    void preloadResources(const std::vector<std::string>& resourcePaths, PreloadPriority priority);

    /**
     * @brief 获取系统统计信息
     * @return 统计信息
     */
    ResourceSystemStats getStats() const;

    /**
     * @brief 优化系统
     * @return 是否优化成功
     */
    bool optimize();

    /**
     * @brief 清理未使用的资源
     * @return 清理的资源数量
     */
    uint64_t cleanupUnusedResources();

    /**
     * @brief 验证资源完整性
     * @return 是否验证通过
     */
    bool verifyIntegrity();

private:
    ResourceSystem() = default;
    ~ResourceSystem() = default;
    ResourceSystem(const ResourceSystem&) = delete;
    ResourceSystem& operator=(const ResourceSystem&) = delete;

    // TODO: 实现以下成员变量
    // ResourceManager manager_;                    ///< 资源管理器
    // ResourceCompressor compressor_;              ///< 资源压缩器
    // ResourcePackager packager_;                  ///< 资源打包器
    // ResourceVersionControl versionControl_;      ///< 资源版本控制
    // ResourcePreloader preloader_;                ///< 资源预加载器
    // PerformanceMonitor performanceMonitor_;      ///< 性能监控器

    ResourceSystemConfig config_;                  ///< 系统配置
    bool initialized_ = false;                     ///< 是否已初始化

    // TODO: 实现以下私有方法
    // void startPerformanceMonitoring();
    // void stopPerformanceMonitoring();
    // static ResourceType inferResourceType(const std::string& path);
};

} // namespace Fantasy 