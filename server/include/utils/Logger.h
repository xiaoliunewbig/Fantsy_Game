/**
 * @file Logger.h
 * @brief 日志记录器核心类
 * @details 提供统一的日志记录接口，支持多种输出方式和格式化
 * @author [pengchengkang] 
 * @date 2025.06.16
 */

#pragma once

#include "LogLevel.h"
#include "LogSink.h"
#include "AsyncQueue.h"
#include <vector>
#include <memory>
#include <mutex>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace Fantasy {

// 日志宏定义，方便使用
#define FANTASY_LOG_TRACE(...) Fantasy::Logger::getInstance().trace(__FILE__, __LINE__, __VA_ARGS__)
#define FANTASY_LOG_DEBUG(...) Fantasy::Logger::getInstance().debug(__FILE__, __LINE__, __VA_ARGS__)
#define FANTASY_LOG_INFO(...)  Fantasy::Logger::getInstance().info(__FILE__, __LINE__, __VA_ARGS__)
#define FANTASY_LOG_WARN(...)  Fantasy::Logger::getInstance().warn(__FILE__, __LINE__, __VA_ARGS__)
#define FANTASY_LOG_ERROR(...) Fantasy::Logger::getInstance().error(__FILE__, __LINE__, __VA_ARGS__)
#define FANTASY_LOG_FATAL(...) Fantasy::Logger::getInstance().fatal(__FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief 日志消息结构
 */
struct LogMessage {
    LogLevel level;                    ///< 日志等级
    std::string message;               ///< 日志消息
    std::string timestamp;             ///< 时间戳
    std::string filename;              ///< 文件名
    int line;                          ///< 行号
    std::thread::id threadId;          ///< 线程ID
};

/**
 * @brief 日志记录器
 * @details 单例模式的日志记录器，支持多种输出方式和线程安全
 */
class Logger {
public:
    /**
     * @brief 获取日志记录器单例
     * @return 日志记录器引用
     */
    static Logger& getInstance();
    
    /**
     * @brief 设置日志等级
     * @param level 新的日志等级
     */
    void setLevel(LogLevel level);
    
    /**
     * @brief 获取当前日志等级
     * @return 当前日志等级
     */
    LogLevel getLevel() const { return level_; }
    
    /**
     * @brief 添加日志输出器
     * @param sink 日志输出器智能指针
     */
    void addSink(LogSinkPtr sink);
    
    /**
     * @brief 移除日志输出器
     * @param name 输出器名称
     */
    void removeSink(const std::string& name);
    
    /**
     * @brief 清空所有输出器
     */
    void clearSinks();
    
    /**
     * @brief 记录日志消息
     * @param level 日志等级
     * @param filename 文件名
     * @param line 行号
     * @param message 日志消息
     */
    void log(LogLevel level, const char* filename, int line, const std::string& message);
    
    /**
     * @brief 格式化记录日志
     * @param level 日志等级
     * @param filename 文件名
     * @param line 行号
     * @param format 格式化字符串
     * @param args 格式化参数
     */
    template<typename... Args>
    void log(LogLevel level, const char* filename, int line, const std::string& format, Args&&... args);
    
    // 便捷的日志记录方法
    template<typename... Args>
    void trace(const char* filename, int line, const std::string& format, Args&&... args) {
        log(LogLevel::TRACE, filename, line, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void debug(const char* filename, int line, const std::string& format, Args&&... args) {
        log(LogLevel::DEBUG, filename, line, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void info(const char* filename, int line, const std::string& format, Args&&... args) {
        log(LogLevel::INFO, filename, line, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void warn(const char* filename, int line, const std::string& format, Args&&... args) {
        log(LogLevel::WARN, filename, line, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void error(const char* filename, int line, const std::string& format, Args&&... args) {
        log(LogLevel::ERROR, filename, line, format, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void fatal(const char* filename, int line, const std::string& format, Args&&... args) {
        log(LogLevel::FATAL, filename, line, format, std::forward<Args>(args)...);
    }
    
    /**
     * @brief 刷新所有输出器
     */
    void flush();
    
    /**
     * @brief 设置日志格式
     * @param format 日志格式字符串，支持以下占位符：
     *               %timestamp% - 时间戳
     *               %level% - 日志等级
     *               %message% - 日志消息
     *               %filename% - 文件名
     *               %line% - 行号
     *               %thread% - 线程ID
     */
    void setFormat(const std::string& format);
    
    /**
     * @brief 获取当前日志格式
     * @return 日志格式字符串
     */
    const std::string& getFormat() const { return format_; }
    
    /**
     * @brief 设置是否启用异步日志
     * @param enable 是否启用
     * @param queueSize 队列大小，0表示无限制
     */
    void setAsync(bool enable, size_t queueSize = 10000);
    
    /**
     * @brief 获取是否启用异步日志
     * @return true表示启用
     */
    bool isAsync() const { return asyncEnabled_; }

private:
    /**
     * @brief 私有构造函数，实现单例模式
     */
    Logger();
    
    /**
     * @brief 私有析构函数
     */
    ~Logger();
    
    /**
     * @brief 禁用拷贝构造
     */
    Logger(const Logger&) = delete;
    
    /**
     * @brief 禁用赋值操作
     */
    Logger& operator=(const Logger&) = delete;
    
    /**
     * @brief 格式化日志消息
     * @param msg 日志消息结构
     * @return 格式化后的消息
     */
    std::string formatMessage(const LogMessage& msg);
    
    /**
     * @brief 获取当前时间戳
     * @return 格式化的时间戳字符串
     */
    std::string getCurrentTimestamp();
    
    /**
     * @brief 处理日志消息
     * @param msg 日志消息
     */
    void processLogMessage(const LogMessage& msg);

private:
    LogLevel level_;                                    ///< 当前日志等级
    std::vector<LogSinkPtr> sinks_;                     ///< 日志输出器列表
    mutable std::mutex mutex_;                          ///< 线程安全锁
    std::string format_;                                ///< 日志格式
    std::chrono::steady_clock::time_point startTime_;   ///< 启动时间
    bool asyncEnabled_;                                 ///< 是否启用异步日志
    std::unique_ptr<AsyncQueue<LogMessage>> asyncQueue_; ///< 异步队列
};

} // namespace Fantasy
