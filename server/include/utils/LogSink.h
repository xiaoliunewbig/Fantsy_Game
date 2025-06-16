/**
 * @file LogSink.h
 * @brief 日志输出接口定义
 * @details 定义日志输出的抽象接口，所有具体的输出实现都继承此接口
 * @author [pengchengkang] 
 * @date 2025.06.16
 */

#pragma once

#include "LogLevel.h"
#include <string>
#include <memory>

namespace Fantasy {

/**
 * @brief 日志输出接口
 * @details 定义了日志输出的抽象接口，支持同步和异步输出
 */
class LogSink {
public:
    virtual ~LogSink() = default;
    
    /**
     * @brief 输出日志消息
     * @param level 日志等级
     * @param message 日志消息内容
     */
    virtual void log(LogLevel level, const std::string& message) = 0;
    
    /**
     * @brief 刷新输出缓冲区
     * @details 确保所有待输出的日志都被立即输出
     */
    virtual void flush() = 0;
    
    /**
     * @brief 检查输出器是否可用
     * @return true表示可用，false表示不可用
     */
    virtual bool isAvailable() const = 0;
    
    /**
     * @brief 设置输出器名称
     * @param name 输出器名称
     */
    virtual void setName(const std::string& name) { name_ = name; }
    
    /**
     * @brief 获取输出器名称
     * @return 输出器名称
     */
    virtual const std::string& getName() const { return name_; }

protected:
    std::string name_; ///< 输出器名称
};

/**
 * @brief 日志输出器智能指针类型
 */
using LogSinkPtr = std::shared_ptr<LogSink>;

} // namespace Fantasy