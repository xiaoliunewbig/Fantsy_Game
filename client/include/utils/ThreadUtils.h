/**
 * @file ThreadUtils.h
 * @brief 线程工具类 - 提供跨平台线程相关工具函数
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 线程创建与管理
 * - 线程优先级设置
 * - 线程同步工具
 * - 线程池封装（TODO）
 * - 线程局部变量支持（TODO）
 */
#ifndef THREAD_UTILS_H
#define THREAD_UTILS_H

#include <thread>
#include <string>
#include <functional>

class ThreadUtils {
public:
    /**
     * @brief 设置当前线程名称
     * @param name 线程名称
     */
    static void SetCurrentThreadName(const std::string& name);

    /**
     * @brief 获取当前线程ID
     * @return 线程ID
     */
    static std::thread::id GetCurrentThreadId();

    /**
     * @brief 睡眠当前线程指定毫秒
     * @param milliseconds 睡眠时间（毫秒）
     */
    static void SleepForMs(int milliseconds);

    /**
     * @brief 设置线程优先级（0~100）
     * @param thread 目标线程
     * @param priority 优先级数值
     * @todo 实现不同平台下的优先级映射
     */
    static void SetThreadPriority(std::thread& thread, int priority);

    /**
     * @brief 创建并启动一个后台线程
     * @param func 线程执行函数
     * @return 新建线程对象
     * @todo 实现线程池调度支持
     */
    static std::thread CreateBackgroundThread(const std::function<void()>& func);
};

#endif // THREAD_UTILS_H