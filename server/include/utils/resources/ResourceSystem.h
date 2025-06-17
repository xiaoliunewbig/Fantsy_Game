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

// 具体资源类型
#include "ConfigResource.h"
#include "MapResource.h"
#include "ScriptResource.h"
#include "TextureResource.h"
#include "ModelResource.h"
#include "AudioResource.h"
#include "FontResource.h"

// 高级功能
#include "ResourceCompressor.h"
#include "ResourcePackager.h"
#include "ResourceVersionControl.h"
#include "ResourcePreloader.h"

namespace Fantasy {

/**
 * @brief 资源系统配置
 */
struct ResourceSystemConfig {
    // 基础配置
    std::string resourceRootDir;        ///< 资源根目录
    uint32_t maxCacheSize = 1024 * 1024 * 1024; ///< 最大缓存大小（字节）
    uint32_t maxLoadingThreads = 4;     ///< 最大加载线程数
    uint32_t cacheExpirationTime = 3600; ///< 缓存过期时间（秒）

    // 日志配置
    std::string logPath;                ///< 日志文件路径
    bool enableLogging = true;          ///< 启用日志
    bool enablePerformanceLogging = true; ///< 启用性能日志

    // 压缩配置
    CompressionOptions compression;     ///< 压缩选项
    bool enableCompression = true;      ///< 启用压缩

    // 打包配置
    PackageOptions packaging;           ///< 打包选项
    bool enablePackaging = true;        ///< 启用打包

    // 版本控制配置
    VersionControlOptions versionControl; ///< 版本控制选项
    bool enableVersionControl = true;   ///< 启用版本控制

    // 预加载配置
    PreloadConfig preloading;           ///< 预加载配置
    bool enablePreloading = true;       ///< 启用预加载

    // 性能监控配置
    bool enablePerformanceMonitoring = true; ///< 启用性能监控
    uint32_t monitoringInterval = 5000; ///< 监控间隔（毫秒）
};

/**
 * @brief 资源系统统计信息
 */
struct ResourceSystemStats {
    // 基础统计
    uint64_t totalResources;            ///< 总资源数
    uint64_t loadedResources;           ///< 已加载资源数
    uint64_t cachedResources;           ///< 缓存资源数
    uint64_t totalMemoryUsage;          ///< 总内存使用
    float cacheHitRate;                 ///< 缓存命中率

    // 性能统计
    uint64_t totalLoadTime;             ///< 总加载时间
    uint64_t averageLoadTime;           ///< 平均加载时间
    uint64_t totalLoadCount;            ///< 总加载次数
    uint64_t failedLoadCount;           ///< 失败加载次数

    // 各类型统计
    std::unordered_map<ResourceType, uint64_t> typeStats; ///< 各类型统计

    // 系统统计
    std::chrono::system_clock::time_point lastUpdate; ///< 最后更新时间
    std::string systemVersion;          ///< 系统版本
};

/**
 * @brief 资源系统主类
 * @details 整合所有资源管理功能的主控制器
 */
class ResourceSystem {
public:
    /**
     * @brief 获取单例实例
     * @return ResourceSystem实例引用
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
     * @return 资源指针
     */
    std::shared_ptr<IResource> loadResource(const std::string& path, ResourceType type);

    /**
     * @brief 异步加载资源
     * @param path 资源路径
     * @param type 资源类型
     * @param callback 回调函数
     */
    void loadResourceAsync(const std::string& path,
                          ResourceType type,
                          std::function<void(std::shared_ptr<IResource>)> callback);

    /**
     * @brief 卸载资源
     * @param resourceId 资源ID
     */
    void unloadResource(const std::string& resourceId);

    /**
     * @brief 重载资源
     * @param resourceId 资源ID
     * @return 是否成功
     */
    bool reloadResource(const std::string& resourceId);

    /**
     * @brief 获取资源
     * @param resourceId 资源ID
     * @return 资源指针
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
     * @param priority 优先级
     */
    void preloadResources(const std::vector<std::string>& resourcePaths,
                         PreloadPriority priority = PreloadPriority::NORMAL);

    /**
     * @brief 创建资源包
     * @param packagePath 包文件路径
     * @param resourcePaths 资源路径列表
     * @return 是否成功
     */
    bool createResourcePackage(const std::string& packagePath,
                              const std::vector<std::string>& resourcePaths);

    /**
     * @brief 打开资源包
     * @param packagePath 包文件路径
     * @return 是否成功
     */
    bool openResourcePackage(const std::string& packagePath);

    /**
     * @brief 压缩资源
     * @param resourceId 资源ID
     * @param algorithm 压缩算法
     * @return 是否成功
     */
    bool compressResource(const std::string& resourceId,
                         CompressionAlgorithm algorithm = CompressionAlgorithm::ZSTD);

    /**
     * @brief 解压资源
     * @param resourceId 资源ID
     * @return 是否成功
     */
    bool decompressResource(const std::string& resourceId);

    /**
     * @brief 添加资源版本
     * @param resourcePath 资源路径
     * @param version 版本号
     * @return 是否成功
     */
    bool addResourceVersion(const std::string& resourcePath, const Version& version);

    /**
     * @brief 回滚资源版本
     * @param resourceId 资源ID
     * @param version 目标版本
     * @return 是否成功
     */
    bool rollbackResourceVersion(const std::string& resourceId, const Version& version);

    /**
     * @brief 获取系统统计信息
     * @return 统计信息
     */
    ResourceSystemStats getStats() const;

    /**
     * @brief 获取性能报告
     * @return 性能报告字符串
     */
    std::string getPerformanceReport() const;

    /**
     * @brief 优化系统
     * @return 是否成功
     */
    bool optimize();

    /**
     * @brief 清理未使用的资源
     * @return 清理的资源数量
     */
    uint64_t cleanupUnusedResources();

    /**
     * @brief 验证系统完整性
     * @return 是否完整
     */
    bool verifyIntegrity();

    /**
     * @brief 导出系统配置
     * @param outputPath 输出路径
     * @return 是否成功
     */
    bool exportConfig(const std::string& outputPath);

    /**
     * @brief 导入系统配置
     * @param inputPath 输入路径
     * @return 是否成功
     */
    bool importConfig(const std::string& inputPath);

    // 获取各个子系统
    ResourceManager& getResourceManager() { return ResourceManager::getInstance(); }
    ResourceCache& getResourceCache() { return ResourceCache::getInstance(); }
    ResourceLoader& getResourceLoader() { return ResourceLoader::getInstance(); }
    ResourceLogger& getResourceLogger() { return ResourceLogger::getInstance(); }
    ResourceCompressor& getResourceCompressor() { return ResourceCompressor::getInstance(); }
    ResourcePackager& getResourcePackager() { return ResourcePackager::getInstance(); }
    ResourceVersionControl& getResourceVersionControl() { return ResourceVersionControl::getInstance(); }
    ResourcePreloader& getResourcePreloader() { return ResourcePreloader::getInstance(); }

private:
    ResourceSystem() = default;
    ~ResourceSystem() = default;

    // 禁用拷贝和移动
    ResourceSystem(const ResourceSystem&) = delete;
    ResourceSystem& operator=(const ResourceSystem&) = delete;
    ResourceSystem(ResourceSystem&&) = delete;
    ResourceSystem& operator=(ResourceSystem&&) = delete;

    /**
     * @brief 初始化各个子系统
     * @return 是否成功
     */
    bool initializeSubsystems();

    /**
     * @brief 注册资源加载器
     */
    void registerResourceLoaders();

    /**
     * @brief 更新统计信息
     */
    void updateStats();

    /**
     * @brief 性能监控线程
     */
    void performanceMonitoringThread();

    ResourceSystemConfig config_;       ///< 系统配置
    ResourceSystemStats stats_;         ///< 系统统计
    std::thread monitoringThread_;      ///< 监控线程
    std::atomic<bool> running_;         ///< 运行标志
    bool initialized_ = false;          ///< 是否已初始化
};

// 便捷的全局函数
namespace ResourceSystemUtils {

/**
 * @brief 快速加载资源
 * @param path 资源路径
 * @param type 资源类型
 * @return 资源指针
 */
std::shared_ptr<IResource> load(const std::string& path, ResourceType type);

/**
 * @brief 快速获取资源
 * @param resourceId 资源ID
 * @return 资源指针
 */
std::shared_ptr<IResource> get(const std::string& resourceId);

/**
 * @brief 快速卸载资源
 * @param resourceId 资源ID
 */
void unload(const std::string& resourceId);

/**
 * @brief 获取系统实例
 * @return 资源系统实例
 */
ResourceSystem& getSystem();

} // namespace ResourceSystemUtils

} // namespace Fantasy 