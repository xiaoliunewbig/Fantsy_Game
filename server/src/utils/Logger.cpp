/**
 * @file Logger.cpp
 * @brief 日志记录器实现
 * @details 实现日志记录器的核心功能，包括单例模式、日志记录、格式化等
 * @author [pengchengkang] 
 * @date 2025.06.16
 */

#include "utils/Logger.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <filesystem>

namespace Fantasy {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() 
    : level_(LogLevel::INFO)
    , format_("%timestamp% [%level%] [%filename%:%line%] [%thread%] %message%")
    , startTime_(std::chrono::steady_clock::now())
    , asyncEnabled_(false) {
}

Logger::~Logger() {
    if (asyncQueue_) {
        asyncQueue_->stop();
    }
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    level_ = level;
}

void Logger::addSink(LogSinkPtr sink) {
    if (!sink) return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    // 检查是否已存在同名输出器
    auto it = std::find_if(sinks_.begin(), sinks_.end(),
        [&sink](const LogSinkPtr& existing) {
            return existing->getName() == sink->getName();
        });
    
    if (it != sinks_.end()) {
        // 如果已存在，则替换
        *it = sink;
    } else {
        // 如果不存在，则添加
        sinks_.push_back(sink);
    }
}

void Logger::removeSink(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    sinks_.erase(
        std::remove_if(sinks_.begin(), sinks_.end(),
            [&name](const LogSinkPtr& sink) {
                return sink->getName() == name;
            }),
        sinks_.end()
    );
}

void Logger::clearSinks() {
    std::lock_guard<std::mutex> lock(mutex_);
    sinks_.clear();
}

void Logger::log(LogLevel level, const char* filename, int line, const std::string& message) {
    if (level < level_) return;
    
    LogMessage msg;
    msg.level = level;
    msg.message = message;
    msg.timestamp = getCurrentTimestamp();
    msg.filename = std::filesystem::path(filename).filename().string();
    msg.line = line;
    msg.threadId = std::this_thread::get_id();
    
    if (asyncEnabled_ && asyncQueue_) {
        asyncQueue_->push(std::move(msg));
    } else {
        processLogMessage(msg);
    }
}

void Logger::processLogMessage(const LogMessage& msg) {
    std::string formattedMessage = formatMessage(msg);
    
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& sink : sinks_) {
        if (sink && sink->isAvailable()) {
            sink->log(msg.level, formattedMessage);
        }
    }
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& sink : sinks_) {
        if (sink && sink->isAvailable()) {
            sink->flush();
        }
    }
}

void Logger::setFormat(const std::string& format) {
    std::lock_guard<std::mutex> lock(mutex_);
    format_ = format;
}

void Logger::setAsync(bool enable, size_t queueSize) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (enable == asyncEnabled_) return;
    
    if (enable) {
        asyncQueue_ = std::make_unique<AsyncQueue<LogMessage>>(
            [this](const LogMessage& msg) { processLogMessage(msg); },
            queueSize
        );
    } else {
        if (asyncQueue_) {
            asyncQueue_->stop();
            asyncQueue_.reset();
        }
    }
    
    asyncEnabled_ = enable;
}

std::string Logger::formatMessage(const LogMessage& msg) {
    std::string result = format_;
    
    // 替换时间戳
    size_t pos = result.find("%timestamp%");
    if (pos != std::string::npos) {
        result.replace(pos, 11, msg.timestamp);
    }
    
    // 替换日志等级
    pos = result.find("%level%");
    if (pos != std::string::npos) {
        result.replace(pos, 7, toString(msg.level));
    }
    
    // 替换文件名
    pos = result.find("%filename%");
    if (pos != std::string::npos) {
        result.replace(pos, 10, msg.filename);
    }
    
    // 替换行号
    pos = result.find("%line%");
    if (pos != std::string::npos) {
        result.replace(pos, 6, std::to_string(msg.line));
    }
    
    // 替换线程ID
    pos = result.find("%thread%");
    if (pos != std::string::npos) {
        std::stringstream ss;
        ss << msg.threadId;
        result.replace(pos, 8, ss.str());
    }
    
    // 替换消息内容
    pos = result.find("%message%");
    if (pos != std::string::npos) {
        result.replace(pos, 9, msg.message);
    }
    
    return result;
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
       << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

// 模板方法的实现
template<typename... Args>
void Logger::log(LogLevel level, const char* filename, int line, const std::string& format, Args&&... args) {
    if (level < level_) return;
    
    // 简单的格式化实现
    std::string message = format;
    // TODO: 实现更复杂的格式化逻辑
    
    log(level, filename, line, message);
}

} // namespace Fantasy
