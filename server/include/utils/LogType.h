/**
 * @file LogType.h
 * @brief 日志类型定义
 * @details 定义不同类型的日志分类
 * @author [pengchengkang] 
 * @date 2025.06.16
 */

#pragma once

#include <string>

namespace Fantasy {

/**
 * @brief 日志类型枚举
 */
enum class LogType {
    SYSTEM,     ///< 系统日志
    PROGRAM,    ///< 程序日志
    CODE,       ///< 代码段日志
    NETWORK,    ///< 网络日志
    DATABASE,   ///< 数据库日志
    SECURITY,   ///< 安全日志
    PERFORMANCE ///< 性能日志
};

/**
 * @brief 获取日志类型字符串
 * @param type 日志类型
 * @return 日志类型字符串
 */
inline std::string toString(LogType type) {
    switch (type) {
        case LogType::SYSTEM:      return "System";
        case LogType::PROGRAM:     return "Program";
        case LogType::CODE:        return "Code";
        case LogType::NETWORK:     return "Network";
        case LogType::DATABASE:    return "Database";
        case LogType::SECURITY:    return "Security";
        case LogType::PERFORMANCE: return "Performance";
        default:                   return "Unknown";
    }
}

/**
 * @brief 获取日志类型目录名
 * @param type 日志类型
 * @return 日志类型目录名
 */
inline std::string getLogTypeDir(LogType type) {
    switch (type) {
        case LogType::SYSTEM:      return "system";
        case LogType::PROGRAM:     return "program";
        case LogType::CODE:        return "code";
        case LogType::NETWORK:     return "network";
        case LogType::DATABASE:    return "database";
        case LogType::SECURITY:    return "security";
        case LogType::PERFORMANCE: return "performance";
        default:                   return "unknown";
    }
}

} // namespace Fantasy 