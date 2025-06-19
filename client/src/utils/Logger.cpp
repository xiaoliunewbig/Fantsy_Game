/**
 * @file Logger.cpp
 * @brief 日志工具实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "utils/Logger.h"
#include <iostream>
#include <iomanip>
#include <chrono>

namespace Fantasy {
std::mutex Logger::s_mutex;              
LogLevel Logger::s_currentLevel = LogLevel::DEBUG;
bool Logger::s_initialized = false;

void Logger::initialize() {
    if (!s_initialized) {
        s_initialized = true;
        info("Logger initialized.");
    }
}

void Logger::setLogLevel(LogLevel level) {
    s_currentLevel = level;
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warn(const std::string& message) {
    log(LogLevel::WARN, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::fatal(const std::string& message) {
    log(LogLevel::FATAL, message);
}

std::string Logger::formatMessage(LogLevel level, const std::string& message) {
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");

    std::string levelStr;
    switch (level) {
        case LogLevel::DEBUG:   levelStr = "DEBUG"; break;
        case LogLevel::INFO:    levelStr = "INFO"; break;
        case LogLevel::WARN:    levelStr = "WARN"; break;
        case LogLevel::ERROR:   levelStr = "ERROR"; break;
        case LogLevel::FATAL:   levelStr = "FATAL"; break;
    }

    return "[" + ss.str() + "] [" + levelStr + "] " + message;
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(s_mutex);

    if (level < s_currentLevel)
        return;

    std::string formatted = formatMessage(level, message);
    std::cout << formatted << std::endl;

    // TODO: 如果启用文件日志，可以在此处写入文件
}

} // namespace Fantasy