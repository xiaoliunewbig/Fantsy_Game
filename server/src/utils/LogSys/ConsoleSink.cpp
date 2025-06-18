/**
 * @file ConsoleSink.cpp
 * @brief 控制台日志输出器实现
 * @details 实现控制台日志输出功能，支持彩色输出
 * @author [pengchengkang] 
 * @date 2025.06.16
 */

#include "utils/LogSys/ConsoleSink.h"
#include <iostream>
#include "utils/LogSys/LogLevel.h"

namespace Fantasy {

// ANSI颜色代码
namespace {
    const char* const RESET = "\033[0m";
    const char* const BLACK = "\033[30m";
    const char* const RED = "\033[31m";
    const char* const GREEN = "\033[32m";
    const char* const YELLOW = "\033[33m";
    const char* const BLUE = "\033[34m";
    const char* const MAGENTA = "\033[35m";
    const char* const CYAN = "\033[36m";
    const char* const WHITE = "\033[37m";
}

ConsoleSink::ConsoleSink(bool useStderr, bool enableColors)
    : useStderr_(useStderr)
    , colorEnabled_(enableColors)
    , stream_(useStderr ? &std::cerr : &std::cout) {
}

void ConsoleSink::log(LogLevel level, const std::string& message) {
    if (!stream_) return;
    
    if (colorEnabled_) {
        *stream_ << getColorCode(level) << message << getResetCode() << std::flush;
    } else {
        *stream_ << message << std::flush;
    }
}

void ConsoleSink::flush() {
    if (stream_) {
        stream_->flush();
    }
}

std::string ConsoleSink::getColorCode(LogLevel level) const {
    if (!colorEnabled_) return "";
    
    switch (level) {
        case LogLevel::TRACE: return CYAN;
        case LogLevel::DEBUG: return BLUE;
        case LogLevel::INFO:  return GREEN;
        case LogLevel::WARN:  return YELLOW;
        case LogLevel::ERROR: return RED;
        case LogLevel::FATAL: return MAGENTA;
        default:             return WHITE;
    }
}

std::string ConsoleSink::getResetCode() const {
    return colorEnabled_ ? RESET : "";
}

} // namespace Fantasy