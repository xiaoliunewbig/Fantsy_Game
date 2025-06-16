/**
 * @file AsyncQueue.h
 * @brief 异步队列实现
 * @details 提供线程安全的异步队列，用于日志的异步处理
 * @author [pengchengkang] 
 * @date 2025.06.16
 */

#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <functional>

namespace Fantasy {

/**
 * @brief 异步队列
 * @tparam T 队列元素类型
 */
template<typename T>
class AsyncQueue {
public:
    using ProcessFunc = std::function<void(const T&)>;

    /**
     * @brief 构造函数
     * @param processFunc 处理函数
     * @param maxSize 队列最大大小，0表示无限制
     */
    explicit AsyncQueue(ProcessFunc processFunc, size_t maxSize = 0)
        : processFunc_(std::move(processFunc))
        , maxSize_(maxSize)
        , running_(true) {
        worker_ = std::thread(&AsyncQueue::processLoop, this);
    }

    /**
     * @brief 析构函数
     */
    ~AsyncQueue() {
        stop();
    }

    /**
     * @brief 添加元素到队列
     * @param item 要添加的元素
     * @return 是否添加成功
     */
    bool push(T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (maxSize_ > 0 && queue_.size() >= maxSize_) {
            return false;
        }
        queue_.push(std::move(item));
        lock.unlock();
        condition_.notify_one();
        return true;
    }

    /**
     * @brief 停止队列处理
     */
    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            running_ = false;
        }
        condition_.notify_all();
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    /**
     * @brief 清空队列
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::queue<T> empty;
        std::swap(queue_, empty);
    }

    /**
     * @brief 获取队列大小
     * @return 当前队列中的元素数量
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    /**
     * @brief 检查队列是否为空
     * @return true表示队列为空
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

private:
    /**
     * @brief 处理循环
     */
    void processLoop() {
        while (true) {
            T item;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                condition_.wait(lock, [this] {
                    return !running_ || !queue_.empty();
                });

                if (!running_ && queue_.empty()) {
                    break;
                }

                item = std::move(queue_.front());
                queue_.pop();
            }

            try {
                processFunc_(item);
            } catch (const std::exception&) {
                // 处理异常，但不中断处理循环
            }
        }
    }

private:
    std::queue<T> queue_;              ///< 队列
    mutable std::mutex mutex_;         ///< 互斥锁
    std::condition_variable condition_; ///< 条件变量
    ProcessFunc processFunc_;          ///< 处理函数
    size_t maxSize_;                   ///< 最大队列大小
    std::atomic<bool> running_;        ///< 运行标志
    std::thread worker_;               ///< 工作线程
};

} // namespace Fantasy 