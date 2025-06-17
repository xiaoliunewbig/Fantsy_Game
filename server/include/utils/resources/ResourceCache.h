/**
 * @file ResourceCache.h
 * @brief 资源缓存系统
 * @author [pengchengkang]
 * @date 2025.06.16
 */

#pragma once

#include "ResourceType.h"
#include <unordered_map>
#include <mutex>
#include <memory>
#include <chrono>
#include <list>
#include <atomic>

namespace Fantasy {

/**
 * @brief 资源缓存系统
 * @details 管理已加载资源的缓存，支持LRU缓存策略
 */
class ResourceCache {
public:
    /**
     * @brief 获取单例实例
     * @return ResourceCache实例引用
     */
    static ResourceCache& getInstance();

    /**
     * @brief 设置缓存大小限制
     * @param maxSize 最大缓存大小（字节）
     */
    void setMaxSize(std::uintmax_t maxSize);

    /**
     * @brief 设置缓存过期时间
     * @param expirationTime 过期时间（毫秒）
     */
    void setExpirationTime(std::chrono::milliseconds expirationTime);

    /**
     * @brief 添加资源到缓存
     * @param resource 要缓存的资源
     * @return 是否成功添加到缓存
     */
    bool addResource(std::shared_ptr<IResource> resource);

    /**
     * @brief 从缓存获取资源
     * @param id 资源ID
     * @return 缓存的资源，如果不存在则返回nullptr
     */
    std::shared_ptr<IResource> getResource(const std::string& id);

    /**
     * @brief 从缓存移除资源
     * @param id 资源ID
     * @return 是否成功移除
     */
    bool removeResource(const std::string& id);

    /**
     * @brief 清理过期资源
     */
    void cleanup();

    /**
     * @brief 清空缓存
     */
    void clear();

    /**
     * @brief 获取当前缓存大小
     * @return 当前缓存大小（字节）
     */
    std::uintmax_t getCurrentSize() const;

    /**
     * @brief 获取缓存命中率
     * @return 缓存命中率（0-1）
     */
    double getHitRate() const;

private:
    ResourceCache();
    ~ResourceCache();

    // 禁用拷贝和移动
    ResourceCache(const ResourceCache&) = delete;
    ResourceCache& operator=(const ResourceCache&) = delete;
    ResourceCache(ResourceCache&&) = delete;
    ResourceCache& operator=(ResourceCache&&) = delete;

    /**
     * @brief 更新资源访问时间
     * @param id 资源ID
     */
    void updateAccessTime(const std::string& id);

    /**
     * @brief 移除最久未使用的资源
     */
    void removeLRU();

private:
    struct CacheEntry {
        std::shared_ptr<IResource> resource;
        std::chrono::steady_clock::time_point lastAccess;
        std::chrono::steady_clock::time_point expirationTime;
    };

    std::unordered_map<std::string, CacheEntry> cache_;           ///< 资源缓存
    std::list<std::string> accessOrder_;                          ///< 访问顺序列表
    std::uintmax_t maxSize_;                                      ///< 最大缓存大小
    std::uintmax_t currentSize_;                                  ///< 当前缓存大小
    std::chrono::milliseconds expirationTime_;                    ///< 过期时间
    mutable std::mutex mutex_;                                    ///< 互斥锁
    std::atomic<std::uintmax_t> hits_;                           ///< 缓存命中次数
    std::atomic<std::uintmax_t> misses_;                         ///< 缓存未命中次数
};

} // namespace Fantasy
