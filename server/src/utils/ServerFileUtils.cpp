/**
 * @file ServerFileUtils.cpp
 * @brief 服务器端文件操作工具类实现
 * @details 实现服务器端所需的文件操作功能，使用标准库实现
 * @author [pengchengkang] 
 * @date 2025.06.16
 */

#include "utils/ServerFileUtils.h"
#include "utils/Logger.h"
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>

namespace Fantasy {

// FileWatcher 实现
FileWatcher::FileWatcher(const std::filesystem::path& path,
                        FileChangeCallback callback,
                        std::chrono::milliseconds interval)
    : path_(path)
    , callback_(std::move(callback))
    , interval_(interval)
    , running_(false) {
    
    // 初始化文件修改时间记录
    if (std::filesystem::exists(path_)) {
        if (std::filesystem::is_directory(path_)) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(path_)) {
                if (entry.is_regular_file()) {
                    lastModified_[entry.path()] = entry.last_modified_time();
                }
            }
        } else {
            lastModified_[path_] = std::filesystem::last_modified_time(path_);
        }
    }
}

FileWatcher::~FileWatcher() {
    stop();
}

void FileWatcher::start() {
    if (running_) return;
    
    running_ = true;
    watcherThread_ = std::thread(&FileWatcher::watchLoop, this);
}

void FileWatcher::stop() {
    if (!running_) return;
    
    running_ = false;
    if (watcherThread_.joinable()) {
        watcherThread_.join();
    }
}

void FileWatcher::watchLoop() {
    while (running_) {
        checkChanges();
        std::this_thread::sleep_for(interval_);
    }
}

void FileWatcher::checkChanges() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!std::filesystem::exists(path_)) {
        // 路径不存在，通知删除
        for (const auto& [oldPath, _] : lastModified_) {
            callback_(oldPath, std::filesystem::path());
        }
        lastModified_.clear();
        return;
    }
    
    if (std::filesystem::is_directory(path_)) {
        // 检查目录中的所有文件
        std::set<std::filesystem::path> currentFiles;
        
        for (const auto& entry : std::filesystem::recursive_directory_iterator(path_)) {
            if (!entry.is_regular_file()) continue;
            
            const auto& currentPath = entry.path();
            currentFiles.insert(currentPath);
            
            auto it = lastModified_.find(currentPath);
            if (it == lastModified_.end()) {
                // 新文件
                lastModified_[currentPath] = entry.last_modified_time();
                callback_(std::filesystem::path(), currentPath);
            } else if (it->second != entry.last_modified_time()) {
                // 文件已修改
                it->second = entry.last_modified_time();
                callback_(currentPath, currentPath);
            }
        }
        
        // 检查删除的文件
        for (auto it = lastModified_.begin(); it != lastModified_.end();) {
            if (currentFiles.find(it->first) == currentFiles.end()) {
                callback_(it->first, std::filesystem::path());
                it = lastModified_.erase(it);
            } else {
                ++it;
            }
        }
    } else {
        // 检查单个文件
        auto currentTime = std::filesystem::last_modified_time(path_);
        auto it = lastModified_.find(path_);
        
        if (it == lastModified_.end()) {
            // 新文件
            lastModified_[path_] = currentTime;
            callback_(std::filesystem::path(), path_);
        } else if (it->second != currentTime) {
            // 文件已修改
            it->second = currentTime;
            callback_(path_, path_);
        }
    }
}

// ServerFileUtils 实现
ServerFileUtils& ServerFileUtils::getInstance() {
    static ServerFileUtils instance;
    return instance;
}

ServerFileUtils::ServerFileUtils()
    : nextWatchId_(0) {
    
    // 创建临时目录
    tempDir_ = std::filesystem::temp_directory_path() / "FantasyLegend";
    createDirectory(tempDir_);
}

ServerFileUtils::~ServerFileUtils() {
    stopAllWatching();
    cleanupTempFiles();
}

bool ServerFileUtils::exists(const std::filesystem::path& path) {
    return std::filesystem::exists(path);
}

bool ServerFileUtils::isDirectory(const std::filesystem::path& path) {
    return std::filesystem::is_directory(path);
}

bool ServerFileUtils::createDirectory(const std::filesystem::path& path, bool recursive) {
    try {
        if (recursive) {
            return std::filesystem::create_directories(path);
        } else {
            return std::filesystem::create_directory(path);
        }
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Failed to create directory: {}", e.what());
        return false;
    }
}

bool ServerFileUtils::remove(const std::filesystem::path& path, bool recursive) {
    try {
        if (recursive) {
            return std::filesystem::remove_all(path) > 0;
        } else {
            return std::filesystem::remove(path);
        }
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Failed to remove path: {}", e.what());
        return false;
    }
}

bool ServerFileUtils::copy(const std::filesystem::path& source,
                         const std::filesystem::path& destination,
                         bool recursive) {
    try {
        if (recursive) {
            std::filesystem::copy(source, destination, 
                                std::filesystem::copy_options::recursive);
        } else {
            std::filesystem::copy(source, destination);
        }
        return true;
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Failed to copy: {}", e.what());
        return false;
    }
}

bool ServerFileUtils::move(const std::filesystem::path& source,
                         const std::filesystem::path& destination) {
    try {
        std::filesystem::rename(source, destination);
        return true;
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Failed to move: {}", e.what());
        return false;
    }
}

std::uintmax_t ServerFileUtils::getFileSize(const std::filesystem::path& path) {
    try {
        return std::filesystem::file_size(path);
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Failed to get file size: {}", e.what());
        return 0;
    }
}

std::filesystem::file_time_type ServerFileUtils::getLastModifiedTime(const std::filesystem::path& path) {
    try {
        return std::filesystem::last_modified_time(path);
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Failed to get last modified time: {}", e.what());
        return std::filesystem::file_time_type::min();
    }
}

std::vector<std::filesystem::path> ServerFileUtils::getFiles(const std::filesystem::path& path,
                                                           bool recursive) {
    std::vector<std::filesystem::path> files;
    try {
        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    files.push_back(entry.path());
                }
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    files.push_back(entry.path());
                }
            }
        }
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Failed to get files: {}", e.what());
    }
    return files;
}

std::vector<std::filesystem::path> ServerFileUtils::getDirectories(const std::filesystem::path& path,
                                                                 bool recursive) {
    std::vector<std::filesystem::path> dirs;
    try {
        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
                if (entry.is_directory()) {
                    dirs.push_back(entry.path());
                }
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.is_directory()) {
                    dirs.push_back(entry.path());
                }
            }
        }
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Failed to get directories: {}", e.what());
    }
    return dirs;
}

std::filesystem::path ServerFileUtils::createTempFile(const std::string& prefix,
                                                    const std::string& suffix) {
    try {
        // 生成随机文件名
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        const char* hex = "0123456789abcdef";
        
        std::string randomStr;
        randomStr.reserve(32);
        for (int i = 0; i < 32; ++i) {
            randomStr += hex[dis(gen)];
        }
        
        std::stringstream ss;
        ss << prefix << "_" << randomStr;
        if (!suffix.empty() && suffix[0] != '.') {
            ss << ".";
        }
        ss << suffix;
        
        std::filesystem::path tempPath = tempDir_ / ss.str();
        std::ofstream file(tempPath);
        if (!file) {
            throw std::runtime_error("Failed to create temporary file");
        }
        
        return tempPath;
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Failed to create temporary file: {}", e.what());
        return std::filesystem::path();
    }
}

std::filesystem::path ServerFileUtils::createTempDirectory(const std::string& prefix) {
    try {
        // 生成随机目录名
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        const char* hex = "0123456789abcdef";
        
        std::string randomStr;
        randomStr.reserve(32);
        for (int i = 0; i < 32; ++i) {
            randomStr += hex[dis(gen)];
        }
        
        std::filesystem::path tempPath = tempDir_ / (prefix + "_" + randomStr);
        if (!std::filesystem::create_directory(tempPath)) {
            throw std::runtime_error("Failed to create temporary directory");
        }
        
        return tempPath;
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Failed to create temporary directory: {}", e.what());
        return std::filesystem::path();
    }
}

void ServerFileUtils::cleanupTempFiles(std::chrono::hours olderThan) {
    try {
        auto now = std::chrono::system_clock::now();
        
        for (const auto& entry : std::filesystem::directory_iterator(tempDir_)) {
            try {
                auto fileTime = std::chrono::clock_cast<std::chrono::system_clock>(
                    std::filesystem::last_modified_time(entry.path()));
                auto age = std::chrono::duration_cast<std::chrono::hours>(now - fileTime);
                
                if (age >= olderThan) {
                    if (entry.is_directory()) {
                        std::filesystem::remove_all(entry.path());
                    } else {
                        std::filesystem::remove(entry.path());
                    }
                }
            } catch (const std::exception& e) {
                FANTASY_LOG_ERROR("Failed to cleanup file {}: {}", 
                                entry.path().string(), e.what());
            }
        }
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Failed to cleanup temporary files: {}", e.what());
    }
}

size_t ServerFileUtils::startWatching(const std::filesystem::path& path,
                                    FileChangeCallback callback,
                                    std::chrono::milliseconds interval) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto watcher = std::make_unique<FileWatcher>(path, std::move(callback), interval);
    size_t watchId = nextWatchId_++;
    
    watcher->start();
    watchers_[watchId] = std::move(watcher);
    
    return watchId;
}

void ServerFileUtils::stopWatching(size_t watchId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = watchers_.find(watchId);
    if (it != watchers_.end()) {
        it->second->stop();
        watchers_.erase(it);
    }
}

void ServerFileUtils::stopAllWatching() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& [_, watcher] : watchers_) {
        watcher->stop();
    }
    watchers_.clear();
}

} // namespace Fantasy 