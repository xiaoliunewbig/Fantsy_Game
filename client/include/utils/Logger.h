/**
 * @file Logger.h
 * @brief 日志工具
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QDebug>

namespace Fantasy {

/**
 * @brief 日志级别枚举
 */
enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

/**
 * @brief 日志工具类
 * 
 * 提供统一的日志记录功能
 */
class Logger
{
public:
    // 初始化日志系统
    static void initialize();
    
    // 设置日志级别
    static void setLogLevel(LogLevel level);
    
    // 日志记录方法
    static void debug(const QString& message);
    static void info(const QString& message);
    static void warn(const QString& message);
    static void error(const QString& message);
    static void fatal(const QString& message);
    
    // 格式化日志消息
    static QString formatMessage(LogLevel level, const QString& message);

private:
    static LogLevel s_currentLevel;
    static bool s_initialized;
};

// 日志宏定义
#define CLIENT_LOG_DEBUG(msg, ...) Fantasy::Logger::debug(QString(msg).arg(__VA_ARGS__))
#define CLIENT_LOG_INFO(msg, ...) Fantasy::Logger::info(QString(msg).arg(__VA_ARGS__))
#define CLIENT_LOG_WARN(msg, ...) Fantasy::Logger::warn(QString(msg).arg(__VA_ARGS__))
#define CLIENT_LOG_ERROR(msg, ...) Fantasy::Logger::error(QString(msg).arg(__VA_ARGS__))
#define CLIENT_LOG_FATAL(msg, ...) Fantasy::Logger::fatal(QString(msg).arg(__VA_ARGS__))

} // namespace Fantasy

#endif // LOGGER_H
