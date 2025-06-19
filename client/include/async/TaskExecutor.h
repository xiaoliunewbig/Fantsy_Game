/**
 * @file TaskExecutor.h
 * @brief 异步任务执行器 - 管理后台任务队列并调度执行
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 支持任务添加与取消
 * - 支持线程池调度
 * - 支持任务优先级
 * - 支持任务依赖关系（TODO）
 * - 支持任务结果回调（TODO）
 */
#ifndef TASK_EXECUTOR_H
#define TASK_EXECUTOR_H

#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class TaskExecutor {
public:
    using Task = std::function<void()>;

    /**
     * @brief 获取单例实例
     * @return 单例引用
     */
    static TaskExecutor& GetInstance();

    /**
     * @brief 初始化任务执行器
     * @param threadCount 工作线程数
     */
    void Init(int threadCount);

    /**
     * @brief 添加一个任务到队列
     * @param task 任务函数
     */
    void AddTask(Task&& task);

    /**
     * @brief 添加一个高优先级任务
     * @param task 任务函数
     * @todo 实现优先级调度
     */
    void AddHighPriorityTask(Task&& task);

    /**
     * @brief 等待所有任务完成
     */
    void WaitForAllTasks();

    /**
     * @brief 停止任务执行器并退出线程
     */
    void Stop();

    /**
     * @brief 获取当前任务队列长度
     * @return 队列长度
     */
    size_t GetTaskQueueSize() const;

private:
    TaskExecutor() = default;
    ~TaskExecutor();

    struct Worker {
        std::thread thread;
        std::atomic<bool> running{true};
    };

    void WorkerLoop();

    std::vector<Worker> workers_;
    std::queue<Task> tasks_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    bool stopped_ = false;
};

#endif // TASK_EXECUTOR_H