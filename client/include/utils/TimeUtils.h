/**
 * @file TimeUtils.h
 * @brief 时间工具类 - 提供常用时间操作和格式化方法
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 获取当前时间戳
 * - 时间格式化输出
 * - 持续时间计算
 * - 支持高精度计时器
 * - 支持本地/UTC时间转换（TODO）
 */
#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <string>
#include <chrono>

class TimeUtils {
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    /**
     * @brief 获取当前时间戳（毫秒）
     * @return 当前时间戳（ms）
     */
    static long long GetCurrentTimestampMs();

    /**
     * @brief 获取当前时间戳（秒）
     * @return 当前时间戳（s）
     */
    static long long GetCurrentTimestampSec();

    /**
     * @brief 将时间点格式化为可读字符串
     * @param time 时间点
     * @param format 格式字符串（默认: "YYYY-MM-DD HH:MM:SS"）
     * @return 格式化后的时间字符串
     * @todo 实现本地/UTC时间切换
     */
    static std::string FormatTime(const TimePoint& time, const std::string& format = "%Y-%m-%d %H:%M:%S");

    /**
     * @brief 开始计时
     */
    static void StartTimer();

    /**
     * @brief 获取已流逝时间（毫秒）
     * @return 已经过的时间（ms）
     */
    static long long GetElapsedTimeMs();

    /**
     * @brief 获取已流逝时间（秒）
     * @return 已经过的时间（s）
     */
    static double GetElapsedTimeSec();

    /**
     * @brief 检查是否已过指定时间
     * @param durationMs 指定持续时间（ms）
     * @return 是否已过期
     */
    static bool HasElapsed(long long durationMs);

private:
    static TimePoint startTime_;
};

#endif // TIME_UTILS_H