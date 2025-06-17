/**
 * @file ResourcePreloader.h
 * @brief 资源预加载系统
 * @author [pengchengkang]
 * @date 2025.06.16
 */

#pragma once

#include "ResourceType.h"
#include "ResourceManager.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <queue>
#include <thread>
#include <atomic>
#include <functional>
#include <chrono>

namespace Fantasy {

/**
 * @brief 预加载策略
 */
enum class PreloadStrategy {
    NONE,           ///< 不预加载
    IMMEDIATE,      ///< 立即预加载
    BACKGROUND,     ///< 后台预加载
    ON_DEMAND,      ///< 按需预加载
    SMART,          ///< 智能预加载
    PRIORITY        ///< 优先级预加载
};

/**
 * @brief 预加载优先级
 */
enum class PreloadPriority {
    LOW = 1,        ///< 低优先级
    NORMAL = 5,     ///< 普通优先级
    HIGH = 10,      ///< 高优先级
    CRITICAL = 20   ///< 关键优先级
};

/**
 * @brief 预加载任务
 */
struct PreloadTask {
    std::string resourceId;             ///< 资源ID
    std::string path;                   ///< 资源路径
    ResourceType type;                  ///< 资源类型
    PreloadPriority priority;           ///< 优先级
    PreloadStrategy strategy;           ///< 预加载策略
    std::function<void(std::shared_ptr<IResource>)> callback; ///< 回调函数
    std::chrono::steady_clock::time_point timestamp; ///< 创建时间
    uint32_t retryCount;                ///< 重试次数
    bool isCompleted;                   ///< 是否完成
    bool isFailed;                      ///< 是否失败
};

/**
 * @brief 预加载统计信息
 */
struct PreloadStats {
    uint64_t totalTasks;                ///< 总任务数
    uint64_t completedTasks;            ///< 完成任务数
    uint64_t failedTasks;               ///< 失败任务数
    uint64_t pendingTasks;              ///< 待处理任务数
    uint64_t totalLoadTime;             ///< 总加载时间
    uint64_t averageLoadTime;           ///< 平均加载时间
    uint64_t cacheHits;                 ///< 缓存命中数
    uint64_t cacheMisses;               ///< 缓存未命中数
    float hitRate;                      ///< 命中率
    std::unordered_map<ResourceType, uint64_t> typeStats; ///< 各类型统计
    std::chrono::steady_clock::time_point lastUpdate; ///< 最后更新时间
};

/**
 * @brief 预加载配置
 */
struct PreloadConfig {
    uint32_t maxConcurrentTasks = 4;    ///< 最大并发任务数
    uint32_t maxRetryCount = 3;         ///< 最大重试次数
    uint32_t retryDelay = 1000;         ///< 重试延迟（毫秒）
    uint32_t taskTimeout = 30000;       ///< 任务超时时间（毫秒）
    uint32_t maxQueueSize = 1000;       ///< 最大队列大小
    bool enablePriorityQueue = true;    ///< 启用优先级队列
    bool enableSmartPreloading = true;  ///< 启用智能预加载
    bool enableLoadPrediction = true;   ///< 启用加载预测
    uint32_t predictionWindow = 5000;   ///< 预测窗口（毫秒）
    float memoryThreshold = 0.8f;       ///< 内存阈值
    uint32_t cleanupInterval = 60000;   ///< 清理间隔（毫秒）
};

/**
 * @brief 资源预加载器
 */
class ResourcePreloader {
public:
    /**
     * @brief 获取单例实例
     * @return ResourcePreloader实例引用
     */
    static ResourcePreloader& getInstance();

    /**
     * @brief 初始化预加载器
     * @param config 预加载配置
     * @return 是否初始化成功
     */
    bool initialize(const PreloadConfig& config = {});

    /**
     * @brief 添加预加载任务
     * @param resourcePath 资源路径
     * @param type 资源类型
     * @param priority 优先级
     * @param strategy 预加载策略
     * @param callback 回调函数
     * @return 任务ID
     */
    std::string addPreloadTask(const std::string& resourcePath,
                              ResourceType type,
                              PreloadPriority priority = PreloadPriority::NORMAL,
                              PreloadStrategy strategy = PreloadStrategy::BACKGROUND,
                              std::function<void(std::shared_ptr<IResource>)> callback = nullptr);

    /**
     * @brief 批量添加预加载任务
     * @param tasks 任务列表
     * @return 任务ID列表
     */
    std::vector<std::string> addBatchPreloadTasks(
        const std::vector<std::pair<std::string, ResourceType>>& tasks,
        PreloadPriority priority = PreloadPriority::NORMAL,
        PreloadStrategy strategy = PreloadStrategy::BACKGROUND);

    /**
     * @brief 取消预加载任务
     * @param taskId 任务ID
     * @return 是否成功
     */
    bool cancelPreloadTask(const std::string& taskId);

    /**
     * @brief 取消所有任务
     */
    void cancelAllTasks();

    /**
     * @brief 获取任务状态
     * @param taskId 任务ID
     * @return 任务状态
     */
    const PreloadTask* getTaskStatus(const std::string& taskId) const;

    /**
     * @brief 等待任务完成
     * @param taskId 任务ID
     * @param timeout 超时时间（毫秒）
     * @return 是否完成
     */
    bool waitForTask(const std::string& taskId, uint32_t timeout = 30000);

    /**
     * @brief 等待所有任务完成
     * @param timeout 超时时间（毫秒）
     * @return 是否全部完成
     */
    bool waitForAllTasks(uint32_t timeout = 60000);

    /**
     * @brief 设置预加载策略
     * @param resourceType 资源类型
     * @param strategy 预加载策略
     */
    void setPreloadStrategy(ResourceType resourceType, PreloadStrategy strategy);

    /**
     * @brief 获取预加载策略
     * @param resourceType 资源类型
     * @return 预加载策略
     */
    PreloadStrategy getPreloadStrategy(ResourceType resourceType) const;

    /**
     * @brief 启用智能预加载
     * @param enabled 是否启用
     */
    void enableSmartPreloading(bool enabled);

    /**
     * @brief 设置内存阈值
     * @param threshold 内存阈值
     */
    void setMemoryThreshold(float threshold);

    /**
     * @brief 获取统计信息
     * @return 统计信息
     */
    PreloadStats getStats() const;

    /**
     * @brief 重置统计信息
     */
    void resetStats();

    /**
     * @brief 清理完成的任务
     */
    void cleanupCompletedTasks();

    /**
     * @brief 优化预加载队列
     */
    void optimizeQueue();

    /**
     * @brief 预测资源需求
     * @param currentResources 当前资源列表
     * @return 预测的资源列表
     */
    std::vector<std::string> predictResourceNeeds(
        const std::vector<std::string>& currentResources);

    /**
     * @brief 获取队列大小
     * @return 队列大小
     */
    uint32_t getQueueSize() const;

    /**
     * @brief 获取活跃任务数
     * @return 活跃任务数
     */
    uint32_t getActiveTaskCount() const;

    /**
     * @brief 暂停预加载
     */
    void pause();

    /**
     * @brief 恢复预加载
     */
    void resume();

    /**
     * @brief 是否已暂停
     * @return 是否已暂停
     */
    bool isPaused() const;

    /**
     * @brief 关闭预加载器
     */
    void shutdown();

private:
    ResourcePreloader() = default;
    ~ResourcePreloader() = default;

    // 禁用拷贝和移动
    ResourcePreloader(const ResourcePreloader&) = delete;
    ResourcePreloader& operator=(const ResourcePreloader&) = delete;
    ResourcePreloader(ResourcePreloader&&) = delete;
    ResourcePreloader& operator=(ResourcePreloader&&) = delete;

    /**
     * @brief 工作线程函数
     */
    void workerThread();

    /**
     * @brief 处理预加载任务
     * @param task 预加载任务
     */
    void processTask(PreloadTask& task);

    /**
     * @brief 智能预加载分析
     */
    void smartPreloadAnalysis();

    /**
     * @brief 更新统计信息
     */
    void updateStats();

    /**
     * @brief 检查内存使用情况
     * @return 内存使用率
     */
    float checkMemoryUsage();

    /**
     * @brief 生成任务ID
     * @return 任务ID
     */
    std::string generateTaskId();

    PreloadConfig config_;              ///< 预加载配置
    std::unordered_map<std::string, PreloadTask> tasks_; ///< 任务映射
    std::priority_queue<PreloadTask> taskQueue_; ///< 任务队列
    std::unordered_map<ResourceType, PreloadStrategy> strategies_; ///< 预加载策略
    std::vector<std::thread> workerThreads_; ///< 工作线程
    std::atomic<bool> running_;         ///< 运行标志
    std::atomic<bool> paused_;          ///< 暂停标志
    std::atomic<uint32_t> activeTaskCount_; ///< 活跃任务数
    mutable PreloadStats stats_;        ///< 统计信息
    std::mutex mutex_;                  ///< 互斥锁
    std::condition_variable condition_; ///< 条件变量
    bool initialized_ = false;          ///< 是否已初始化
    std::atomic<uint64_t> taskIdCounter_; ///< 任务ID计数器
};

} // namespace Fantasy 