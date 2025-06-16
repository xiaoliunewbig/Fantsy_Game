/**
 * @file LogLevel.cpp
 * @brief 日志等级转换实现
 * @details 实现日志等级与字符串之间的转换功能
 * @author [pengchengkang] 
 * @date 2025.06.16
 */

#include "LogLevel.h"
#include <algorithm>
#include <cstring>

namespace Fantasy {

const char* toString(LogLevel level) {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default:              return "UNKNOWN";
    }
}

LogLevel fromString(const std::string& levelStr) {
    std::string upperStr = levelStr;
    std::transform(upperStr.begin(), upperStr.end(), upperStr.begin(), ::toupper);
    
    if (upperStr == "TRACE") return LogLevel::TRACE;
    if (upperStr == "DEBUG") return LogLevel::DEBUG;
    if (upperStr == "INFO")  return LogLevel::INFO;
    if (upperStr == "WARN")  return LogLevel::WARN;
    if (upperStr == "ERROR") return LogLevel::ERROR;
    if (upperStr == "FATAL") return LogLevel::FATAL;
    
    // 默认返回INFO等级
    return LogLevel::INFO;
}

bool isValidLevel(LogLevel level) {
    return level >= LogLevel::TRACE && level <= LogLevel::FATAL;
}

} // namespace Fantasy