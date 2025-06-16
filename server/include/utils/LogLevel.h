/**
 * @file LogLevel.h
 * @brief 日志等级定义和转换工具
 * @details 定义日志系统的等级枚举，提供等级到字符串的转换功能
 * @author [pengchengkang] 
 * @date 2025.06.16
 */

#pragma once

#include <string>

namespace Fantasy {

/**
 * @brief 日志等级枚举
 * @details 定义了从TRACE到FATAL的6个日志等级，等级越高表示越重要
 */
enum class LogLevel {
    TRACE = 0,  ///< 跟踪信息，最详细的调试信息
    DEBUG = 1,  ///< 调试信息，用于开发调试
    INFO = 2,   ///< 一般信息，程序正常运行信息
    WARN = 3,   ///< 警告信息，可能的问题但不影响运行
    ERROR = 4,  ///< 错误信息，程序运行错误
    FATAL = 5   ///< 致命错误，程序无法继续运行
};

/**
 * @brief 将日志等级转换为字符串
 * @param level 日志等级
 * @return 对应的字符串表示
 */
const char* toString(LogLevel level);

/**
 * @brief 将字符串转换为日志等级
 * @param levelStr 等级字符串
 * @return 对应的日志等级，如果无效则返回INFO
 */
LogLevel fromString(const std::string& levelStr);

/**
 * @brief 检查日志等级是否有效
 * @param level 日志等级
 * @return true表示有效，false表示无效
 */
bool isValidLevel(LogLevel level);

} // namespace Fantasy