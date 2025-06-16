/**
 * @file SignalSink.cpp
 * @brief 信号日志输出器实现
 * @details 实现通过回调函数处理日志消息的功能
 * @author [pengchengkang] 
 * @date 2025.06.16
 */

#include "include/utils/LogLevel.h"
#include "include/utils/SignalSink.h"

namespace Fantasy {

SignalSink::SignalSink(LogCallback callback)
    : callback_(std::move(callback)) {
}

void SignalSink::log(LogLevel level, const std::string& message) {
    if (callback_) {
        callback_(level, message);
    }
}

bool SignalSink::isAvailable() const {
    return static_cast<bool>(callback_);
}

void SignalSink::setCallback(LogCallback callback) {
    callback_ = std::move(callback);
}

} // namespace Fantasy