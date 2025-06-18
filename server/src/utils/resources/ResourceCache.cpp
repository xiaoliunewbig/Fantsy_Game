/**
 * @file ResourceCache.cpp
 * @brief 资源缓存系统实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "utils/resources/ResourceCache.h"
#include "utils/resources/ResourceLogger.h"
#include <algorithm>
#include <sstream>

namespace Fantasy {

ResourceCache::ResourceCache()
    : maxSize_(1024 * 1024 * 1024) // 默认1GB
    , currentSize_(0)
    , expirationTime_(std::chrono::hours(1)) // 默认1小时
    , hits_(0)
    , misses_(0) {
}

ResourceCache::~ResourceCache() {
    clear();
}

ResourceCache& ResourceCache::getInstance() {
    static ResourceCache instance;
    return instance;
}

void ResourceCache::setMaxSize(std::uintmax_t maxSize) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    maxSize_ = maxSize;
    FANTASY_LOG_INFO("Cache max size set to {}MB", maxSize_ / (1024 * 1024));
    
    // 如果当前大小超过新的最大值，进行清理
    while (currentSize_ > maxSize_ && !cache_.empty()) {
        removeLRU();
    }
}

void ResourceCache::setExpirationTime(std::chrono::milliseconds expirationTime) {
    std::lock_guard<std::mutex> lock(mutex_);
    expirationTime_ = expirationTime;
    FANTASY_LOG_INFO("Cache expiration time set to {}ms", expirationTime_.count());
}

bool ResourceCache::addResource(std::shared_ptr<IResource> resource) {
    if (!resource) {
        FANTASY_LOG_WARN("Attempted to cache null resource");
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 检查资源大小
    std::uintmax_t resourceSize = resource->getSize();
    if (resourceSize > maxSize_) {
        FANTASY_LOG_WARN("Resource {} too large ({}MB) for cache (max: {}MB)",
                         resource->getId(), resourceSize / (1024 * 1024), maxSize_ / (1024 * 1024));
        return false;
    }
    
    // 如果缓存已满，进行清理
    while ((currentSize_ + resourceSize > maxSize_) && !cache_.empty()) {
        removeLRU();
    }
    
    // 创建缓存条目
    CacheEntry entry;
    entry.resource = resource;
    entry.lastAccess = std::chrono::steady_clock::now();
    entry.expirationTime = entry.lastAccess + expirationTime_;
    
    // 添加到缓存
    std::string resourceId = resource->getId();
    cache_[resourceId] = entry;
    accessOrder_.push_back(resourceId);
    currentSize_ += resourceSize;
    
    FANTASY_LOG_DEBUG("Resource cached: {} (size: {}MB, total: {}MB)",
                      resourceId, resourceSize / (1024 * 1024), currentSize_ / (1024 * 1024));
    
    // 记录缓存统计
    ResourceLogger::getInstance().recordCachePut(resourceId, resource->getType(), resourceSize);
    
    return true;
}

std::shared_ptr<IResource> ResourceCache::getResource(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = cache_.find(id);
    if (it == cache_.end()) {
        misses_++;
        FANTASY_LOG_DEBUG("Cache miss: {}", id);
        return nullptr;
    }
    
    CacheEntry& entry = it->second;
    
    // 检查是否过期
    auto now = std::chrono::steady_clock::now();
    if (now > entry.expirationTime) {
        FANTASY_LOG_DEBUG("Cached resource expired: {}", id);
        removeResource(id);
        misses_++;
        return nullptr;
    }
    
    // 更新访问时间
    entry.lastAccess = now;
    updateAccessTime(id);
    
    hits_++;
    FANTASY_LOG_DEBUG("Cache hit: {}", id);
    
    // 记录缓存统计
    ResourceLogger::getInstance().recordCacheHit(id, entry.resource->getType());
    
    return entry.resource;
}

bool ResourceCache::removeResource(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = cache_.find(id);
    if (it == cache_.end()) {
        return false;
    }
    
    currentSize_ -= it->second.resource->getSize();
    cache_.erase(it);
    
    // 从访问顺序列表中移除
    accessOrder_.remove(id);
    
    FANTASY_LOG_DEBUG("Removed cached resource: {}", id);
    return true;
}

void ResourceCache::cleanup() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::steady_clock::now();
    size_t removedCount = 0;
    
    auto it = cache_.begin();
    while (it != cache_.end()) {
        if (now > it->second.expirationTime) {
            currentSize_ -= it->second.resource->getSize();
            accessOrder_.remove(it->first);
            it = cache_.erase(it);
            removedCount++;
        } else {
            ++it;
        }
    }
    
    if (removedCount > 0) {
        FANTASY_LOG_INFO("Cleaned up {} expired resources from cache", removedCount);
    }
}

void ResourceCache::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t entryCount = cache_.size();
    cache_.clear();
    accessOrder_.clear();
    currentSize_ = 0;
    
    FANTASY_LOG_INFO("Cache cleared ({} entries removed)", entryCount);
}

std::uintmax_t ResourceCache::getCurrentSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return currentSize_;
}

double ResourceCache::getHitRate() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    uint64_t totalRequests = hits_ + misses_;
    if (totalRequests == 0) {
        return 0.0;
    }
    
    return static_cast<double>(hits_) / totalRequests;
}

void ResourceCache::updateAccessTime(const std::string& id) {
    // 将访问的资源移到列表末尾（最近使用）
    accessOrder_.remove(id);
    accessOrder_.push_back(id);
}

void ResourceCache::removeLRU() {
    if (accessOrder_.empty()) {
        return;
    }
    
    // 移除最久未使用的资源（列表头部）
    std::string lruId = accessOrder_.front();
    accessOrder_.pop_front();
    
    auto it = cache_.find(lruId);
    if (it != cache_.end()) {
        currentSize_ -= it->second.resource->getSize();
        cache_.erase(it);
        
        FANTASY_LOG_DEBUG("Removed LRU resource: {}", lruId);
    }
}

} // namespace Fantasy
