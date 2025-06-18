/**
 * @file ConsoleSink.h
 * @brief 控制台日志输出器
 * @details 将日志消息输出到控制台，支持彩色输出和格式化
 * @author [pengchengkang] 
 * @date 2025.06.16
 */

#pragma once

#include "LogSink.h"
#include <iostream>

namespace Fantasy {

/**
 * @brief 控制台日志输出器
 * @details 将日志消息输出到标准输出或标准错误流
 */
class ConsoleSink : public LogSink {
public:
    /**
     * @brief 构造函数
     * @param useStderr 是否使用标准错误流，默认使用标准输出流
     * @param enableColors 是否启用彩色输出，默认启用
     */
    explicit ConsoleSink(bool useStderr = false, bool enableColors = true);
    
    /**
     * @brief 析构函数
     */
    ~ConsoleSink() override = default;
    
    /**
     * @brief 输出日志消息到控制台
     * @param level 日志等级
     * @param message 日志消息内容
     */
    void log(LogLevel level, const std::string& message) override;
    
    /**
     * @brief 刷新输出缓冲区
     */
    void flush() override;
    
    /**
     * @brief 检查控制台输出器是否可用
     * @return 总是返回true，因为控制台总是可用的
     */
    bool isAvailable() const override { return true; }
    
    /**
     * @brief 设置是否启用彩色输出
     * @param enable 是否启用
     */
    void setColorEnabled(bool enable) { colorEnabled_ = enable; }
    
    /**
     * @brief 获取是否启用彩色输出
     * @return true表示启用，false表示禁用
     */
    bool isColorEnabled() const { return colorEnabled_; }

private:
    /**
     * @brief 获取日志等级对应的颜色代码
     * @param level 日志等级
     * @return ANSI颜色代码
     */
    std::string getColorCode(LogLevel level) const;
    
    /**
     * @brief 重置颜色代码
     * @return ANSI重置颜色代码
     */
    std::string getResetCode() const;

private:
    bool useStderr_;     ///< 是否使用标准错误流
    bool colorEnabled_;  ///< 是否启用彩色输出
    std::ostream* stream_; ///< 输出流指针
};

} // namespace Fantasy