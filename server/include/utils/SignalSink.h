/**
 * @file SignalSink.h
 * @brief 信号日志输出器
 * @details 通过回调函数将日志消息传递给外部处理，支持自定义处理逻辑
 * @author [pengchengkang] 
 * @date 2025.06.16
 */

#pragma once

#include "LogSink.h"
#include <functional>
#include <string>

namespace Fantasy {

/**
 * @brief 日志消息回调函数类型
 * @param level 日志等级
 * @param message 日志消息内容
 */
using LogCallback = std::function<void(LogLevel level, const std::string& message)>;

/**
 * @brief 信号日志输出器
 * @details 通过回调函数将日志消息传递给外部处理，适用于需要自定义日志处理逻辑的场景
 */
class SignalSink : public LogSink {
public:
    /**
     * @brief 构造函数
     * @param callback 日志回调函数
     */
    explicit SignalSink(LogCallback callback);
    
    /**
     * @brief 析构函数
     */
    ~SignalSink() override = default;
    
    /**
     * @brief 输出日志消息到回调函数
     * @param level 日志等级
     * @param message 日志消息内容
     */
    void log(LogLevel level, const std::string& message) override;
    
    /**
     * @brief 刷新输出缓冲区（信号输出器无需实现）
     */
    void flush() override {}
    
    /**
     * @brief 检查信号输出器是否可用
     * @return true表示回调函数已设置，false表示回调函数未设置
     */
    bool isAvailable() const override;
    
    /**
     * @brief 设置日志回调函数
     * @param callback 新的回调函数
     */
    void setCallback(LogCallback callback);
    
    /**
     * @brief 获取当前回调函数
     * @return 回调函数，如果未设置则返回空函数
     */
    const LogCallback& getCallback() const { return callback_; }

private:
    LogCallback callback_; ///< 日志回调函数
};

} // namespace Fantasy
