/**
 * @file ResourceManager.h
 * @brief 资源管理器
 * @author [pengchengkang]
 * @date 2025.06.16
 */

#pragma once

#include "ResourceType.h"
#include "ResourceCache.h"
#include "ResourceLoader.h"
#include <unordered_map>
#include <mutex>
#include <memory>
#include <filesystem>
#include <functional>

namespace Fantasy {

/**
 * @brief 资源管理器
 * @details 统一管理所有游戏资源，提供资源的加载、卸载、重载等功能
 */
class ResourceManager {
public:
    /**
     * @brief 获取单例实例
     * @return ResourceManager实例引用
     */
    static ResourceManager& getInstance();

    /**
     * @brief 初始化资源管理器
     * @param resourceRoot 资源根目录
     * @param threadCount 加载线程数
     * @param cacheSize 缓存大小限制
     * @return 是否初始化成功
     */
    bool initialize(const std::filesystem::path& resourceRoot,
                   size_t threadCount = 4,
                   std::uintmax_t cacheSize = 1024 * 1024 * 1024); // 默认1GB缓存

    /**
     * @brief 加载资源
     * @param path 资源路径（相对于资源根目录）
     * @param type 资源类型
     * @return 加载的资源
     */
    std::shared_ptr<IResource> load(const std::filesystem::path& path, ResourceType type);

    /**
     * @brief 异步加载资源
     * @param path 资源路径（相对于资源根目录）
     * @param type 资源类型
     * @param callback 加载完成回调
     */
    void loadAsync(const std::filesystem::path& path,
                  ResourceType type,
                  std::function<void(std::shared_ptr<IResource>)> callback);

    /**
     * @brief 卸载资源
     * @param path 资源路径
     * @return 是否成功卸载
     */
    bool unload(const std::filesystem::path& path);

    /**
     * @brief 重新加载资源
     * @param path 资源路径
     * @return 是否成功重新加载
     */
    bool reload(const std::filesystem::path& path);

    /**
     * @brief 重新加载所有资源
     */
    void reloadAll();

    /**
     * @brief 获取资源
     * @param path 资源路径
     * @return 资源对象，如果未加载则返回nullptr
     */
    std::shared_ptr<IResource> get(const std::filesystem::path& path);

    /**
     * @brief 检查资源是否存在
     * @param path 资源路径
     * @return 是否存在
     */
    bool exists(const std::filesystem::path& path) const;

    /**
     * @brief 获取资源根目录
     * @return 资源根目录路径
     */
    const std::filesystem::path& getResourceRoot() const;

    /**
     * @brief 设置资源根目录
     * @param root 新的资源根目录
     * @return 是否设置成功
     */
    bool setResourceRoot(const std::filesystem::path& root);

    /**
     * @brief 获取资源加载器
     * @return 资源加载器引用
     */
    ResourceLoader& getLoader();

    /**
     * @brief 获取资源缓存
     * @return 资源缓存引用
     */
    ResourceCache& getCache();

    /**
     * @brief 清理未使用的资源
     */
    void cleanup();

    /**
     * @brief 获取资源统计信息
     * @return 统计信息字符串
     */
    std::string getStats() const;

private:
    ResourceManager();
    ~ResourceManager();

    // 禁用拷贝和移动
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    /**
     * @brief 获取资源的完整路径
     * @param path 相对路径
     * @return 完整路径
     */
    std::filesystem::path getFullPath(const std::filesystem::path& path) const;

    /**
     * @brief 检查资源是否需要重新加载
     * @param resource 资源对象
     * @return 是否需要重新加载
     */
    bool needsReload(const std::shared_ptr<IResource>& resource) const;

private:
    std::filesystem::path resourceRoot_;                          ///< 资源根目录
    std::unordered_map<std::string, std::weak_ptr<IResource>> resources_; ///< 资源映射
    mutable std::mutex mutex_;                                    ///< 互斥锁
    bool initialized_;                                            ///< 初始化标志
};

} // namespace Fantasy
