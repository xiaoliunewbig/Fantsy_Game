/**
 * @file ResourceLoader.h
 * @brief 资源加载器
 * @author [pengchengkang]
 * @date 2025.06.16
 */

#pragma once

#include "ResourceType.h"
#include <unordered_map>
#include <mutex>
#include <memory>
#include <thread>
#include <queue>
#include <condition_variable>
#include <atomic>

namespace Fantasy {

/**
 * @brief 资源加载器
 * @details 管理资源加载，支持同步和异步加载
 */
class ResourceLoader {
public:
    /**
     * @brief 获取单例实例
     * @return ResourceLoader实例引用
     */
    static ResourceLoader& getInstance();

    /**
     * @brief 注册资源加载器
     * @param type 资源类型
     * @param loader 资源加载器
     */
    void registerLoader(ResourceType type, std::shared_ptr<IResourceLoader> loader);

    /**
     * @brief 加载资源
     * @param path 资源路径
     * @param type 资源类型
     * @return 加载的资源
     */
    std::shared_ptr<IResource> load(const std::filesystem::path& path, ResourceType type);

    /**
     * @brief 异步加载资源
     * @param path 资源路径
     * @param type 资源类型
     * @param callback 加载完成回调
     */
    void loadAsync(const std::filesystem::path& path,
                  ResourceType type,
                  std::function<void(std::shared_ptr<IResource>)> callback);

    /**
     * @brief 取消异步加载
     * @param path 资源路径
     * @return 是否成功取消
     */
    bool cancelAsyncLoad(const std::filesystem::path& path);

    /**
     * @brief 设置工作线程数
     * @param threadCount 线程数
     */
    void setThreadCount(size_t threadCount);

    /**
     * @brief 获取当前加载队列大小
     * @return 队列中的任务数
     */
    size_t getQueueSize();

    /**
     * @brief 等待所有异步加载完成
     */
    void waitForAll();

    /**
     * @brief 停止所有异步加载
     */
    void stopAll();

private:
    ResourceLoader();
    ~ResourceLoader();

    // 禁用拷贝和移动
    ResourceLoader(const ResourceLoader&) = delete;
    ResourceLoader& operator=(const ResourceLoader&) = delete;
    ResourceLoader(ResourceLoader&&) = delete;
    ResourceLoader& operator=(ResourceLoader&&) = delete;

    struct LoadTask {
        std::filesystem::path path;
        ResourceType type;
        std::function<void(std::shared_ptr<IResource>)> callback;
        bool cancelled;
    };

    /**
     * @brief 工作线程函数
     */
    void workerThread();

    /**
     * @brief 处理加载任务
     * @param task 加载任务
     */
    void processLoadTask(const LoadTask& task);

private:
    std::unordered_map<ResourceType, std::shared_ptr<IResourceLoader>> loaders_;  ///< 资源加载器
    std::queue<LoadTask> loadQueue_;                                             ///< 加载队列
    std::vector<std::thread> workerThreads_;                                     ///< 工作线程
    std::mutex mutex_;                                                           ///< 互斥锁
    std::condition_variable condition_;                                          ///< 条件变量
    std::atomic<bool> running_;                                                  ///< 运行标志
    std::atomic<size_t> activeThreads_;                                         ///< 活动线程数
};

} // namespace Fantasy
