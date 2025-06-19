/**
 * @file Logger.h
 * @brief 日志记录工具接口定义 - 支持多级别日志输出
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 支持 DEBUG/INFO/WARN/ERROR 级别
 * - 支持日志文件输出（TODO）
 * - 支持控制台输出
 * - 支持日志级别过滤
 * - 支持日志时间戳（TODO）
 */
#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <sstream>
#include <iostream>
#include <mutex>
#include <ctime>

namespace Fantasy {

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

class Logger {
public:
    // 初始化日志系统
    static void initialize();

    // 设置日志级别
    static void setLogLevel(LogLevel level);

    // 日志记录方法
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);
    static void fatal(const std::string& message);

    // 格式化日志消息（包含时间戳）
    static std::string formatMessage(LogLevel level, const std::string& message);

    // 格式化字符串工具（类似 QString.arg(...)）
    template<typename... Args>
    static std::string formatString(const char* fmt, Args... args) {
        int size = snprintf(nullptr, 0, fmt, args...); // 获取所需长度
        std::string buffer(size + 1, '\0');
        snprintf(buffer.data(), size + 1, fmt, args...);
        return buffer;
    }

private:
    static LogLevel s_currentLevel;
    static bool s_initialized;
    static std::mutex s_mutex;

    // 内部日志输出函数
    static void log(LogLevel level, const std::string& message);
};

} // namespace Fantasy

#endif // LOGGER_H