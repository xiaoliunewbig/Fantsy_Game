/**
 * @file FileSink.cpp
 * @brief 文件日志输出器实现
 * @details 实现文件日志输出功能，支持文件创建、自动刷新、文件轮转和日志分类
 * @author [pengchengkang] 
 * @date 2025.06.16
 */

#include "utils/FileSink.h"
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <thread>

namespace Fantasy {

FileSink::FileSink(const std::string& baseDir, LogType type, const RotationPolicy& policy, bool append, bool autoFlush)
    : baseDir_(baseDir)
    , type_(type)
    , append_(append)
    , autoFlush_(autoFlush)
    , isOpen_(false)
    , policy_(policy)
    , lastRotationTime_(std::chrono::system_clock::now())
    , currentFileSize_(0)
    , cleanupRunning_(false) {
    
    // 生成当前文件名
    currentFileName_ = generateCurrentFileName();
    
    if (policy_.rotateOnStart) {
        rotate();
    }
    
    if (openFile()) {
        startCleanupThread();
    }
}

FileSink::~FileSink() {
    stopCleanupThread();
    if (fileStream_.is_open()) {
        fileStream_.close();
    }
}

void FileSink::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!isOpen_ && !openFile()) return;
    
    // 检查是否需要轮转
    if (shouldRotate()) {
        rotate();
    }
    
    fileStream_ << message;
    currentFileSize_ += message.size();
    
    if (autoFlush_) {
        fileStream_.flush();
    }
}

void FileSink::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (isOpen_) {
        fileStream_.flush();
    }
}

bool FileSink::isAvailable() const {
    return isOpen_;
}

bool FileSink::reopen() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (fileStream_.is_open()) {
        fileStream_.close();
    }
    return openFile();
}

bool FileSink::setBaseDir(const std::string& baseDir) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (baseDir == baseDir_) return true;
    
    baseDir_ = baseDir;
    currentFileName_ = generateCurrentFileName();
    return reopen();
}

bool FileSink::setLogType(LogType type) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (type == type_) return true;
    
    type_ = type;
    currentFileName_ = generateCurrentFileName();
    return reopen();
}

void FileSink::setRotationPolicy(const RotationPolicy& policy) {
    std::lock_guard<std::mutex> lock(mutex_);
    policy_ = policy;
    
    // 如果清理间隔改变，重启清理线程
    if (cleanupRunning_) {
        stopCleanupThread();
        startCleanupThread();
    }
}

bool FileSink::createDirectory() {
    try {
        std::filesystem::path path(baseDir_);
        path /= getLogTypeDir(type_);
        std::filesystem::create_directories(path);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool FileSink::openFile() {
    if (!createDirectory()) {
        return false;
    }
    
    try {
        std::filesystem::path path(baseDir_);
        path /= getLogTypeDir(type_);
        path /= currentFileName_;
        
        std::ios_base::openmode mode = std::ios_base::out;
        if (append_) {
            mode |= std::ios_base::app;
        }
        
        fileStream_.open(path.string(), mode);
        isOpen_ = fileStream_.is_open();
        
        if (isOpen_) {
            fileStream_.imbue(std::locale(""));
            fileStream_.setf(std::ios::unitbuf);
            
            if (append_) {
                fileStream_.seekp(0, std::ios::end);
                currentFileSize_ = fileStream_.tellp();
            } else {
                currentFileSize_ = 0;
            }
        }
        
        return isOpen_;
    } catch (const std::exception&) {
        isOpen_ = false;
        return false;
    }
}

bool FileSink::shouldRotate() const {
    if (!isOpen_) return false;
    
    // 检查文件大小
    if (currentFileSize_ >= policy_.maxFileSize) {
        return true;
    }
    
    // 检查文件年龄
    auto now = std::chrono::system_clock::now();
    auto age = std::chrono::duration_cast<std::chrono::hours>(now - lastRotationTime_);
    if (age >= policy_.maxAge) {
        return true;
    }
    
    return false;
}

bool FileSink::rotate() {
    if (!isOpen_) return false;
    
    fileStream_.close();
    isOpen_ = false;
    
    try {
        std::filesystem::path basePath(baseDir_);
        basePath /= getLogTypeDir(type_);
        
        // 重命名现有文件
        for (size_t i = policy_.maxFiles - 1; i > 0; --i) {
            std::string oldName = getRotatedFileName(i - 1);
            std::string newName = getRotatedFileName(i);
            
            std::filesystem::path oldPath = basePath / oldName;
            std::filesystem::path newPath = basePath / newName;
            
            if (std::filesystem::exists(oldPath)) {
                if (std::filesystem::exists(newPath)) {
                    std::filesystem::remove(newPath);
                }
                std::filesystem::rename(oldPath, newPath);
            }
        }
        
        // 重命名当前文件
        std::string firstRotatedName = getRotatedFileName(0);
        std::filesystem::path currentPath = basePath / currentFileName_;
        std::filesystem::path firstRotatedPath = basePath / firstRotatedName;
        
        if (std::filesystem::exists(currentPath)) {
            if (std::filesystem::exists(firstRotatedPath)) {
                std::filesystem::remove(firstRotatedPath);
            }
            std::filesystem::rename(currentPath, firstRotatedPath);
        }
        
        // 生成新的文件名
        currentFileName_ = generateCurrentFileName();
        lastRotationTime_ = std::chrono::system_clock::now();
        currentFileSize_ = 0;
        
        return openFile();
    } catch (const std::exception&) {
        return false;
    }
}

void FileSink::cleanupOldFiles() {
    try {
        std::filesystem::path basePath(baseDir_);
        basePath /= getLogTypeDir(type_);
        
        auto now = std::chrono::system_clock::now();
        
        for (const auto& entry : std::filesystem::directory_iterator(basePath)) {
            if (!entry.is_regular_file()) continue;
            
            auto fileName = entry.path().filename().string();
            if (fileName.find("Fantasy_" + toString(type_)) != 0) continue;
            
            auto fileTime = std::filesystem::last_write_time(entry.path());
            auto fileTimePoint = std::chrono::clock_cast<std::chrono::system_clock>(fileTime);
            auto age = std::chrono::duration_cast<std::chrono::hours>(now - fileTimePoint);
            
            if (age >= policy_.maxAge) {
                std::filesystem::remove(entry.path());
            }
        }
    } catch (const std::exception&) {
        // 忽略清理错误
    }
}

std::string FileSink::getRotatedFileName(size_t index) const {
    std::string baseName = currentFileName_;
    size_t dotPos = baseName.find_last_of('.');
    if (dotPos != std::string::npos) {
        baseName = baseName.substr(0, dotPos);
    }
    return baseName + "." + std::to_string(index + 1) + ".log";
}

std::string FileSink::generateCurrentFileName() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << "Fantasy_" << toString(type_) << "_"
       << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S")
       << ".log";
    return ss.str();
}

void FileSink::startCleanupThread() {
    if (cleanupRunning_) return;
    
    cleanupRunning_ = true;
    cleanupThread_ = std::thread(&FileSink::cleanupThreadFunc, this);
}

void FileSink::stopCleanupThread() {
    if (!cleanupRunning_) return;
    
    cleanupRunning_ = false;
    if (cleanupThread_.joinable()) {
        cleanupThread_.join();
    }
}

void FileSink::cleanupThreadFunc() {
    while (cleanupRunning_) {
        cleanupOldFiles();
        
        // 等待下一次清理
        for (size_t i = 0; i < policy_.cleanupInterval.count() && cleanupRunning_; ++i) {
            std::this_thread::sleep_for(std::chrono::minutes(1));
        }
    }
}

} // namespace Fantasy