/**
 * @file ServerFileUtils.cpp
 * @brief 服务器文件工具类实现
 * @details 文件操作、监控、临时文件管理的具体实现
 * @author [pengchengkang]
 * @date 2025.06.18
 */

#include "utils/ServerFileUtils.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <set>
#include <sstream>
#include <fstream>

namespace Fantasy {

// FileWatcher 实现
FileWatcher::FileWatcher(const std::filesystem::path& path, FileChangeCallback callback, std::chrono::milliseconds interval)
    : path_(path), callback_(std::move(callback)), interval_(interval), running_(false) {
    
    if (std::filesystem::is_directory(path_)) {
        // 监控目录
        for (const auto& entry : std::filesystem::directory_iterator(path_)) {
            if (entry.is_regular_file()) {
                lastModified_[entry.path()] = entry.last_write_time();
            }
        }
    } else {
        // 监控单个文件
        lastModified_[path_] = std::filesystem::last_write_time(path_);
    }
}

FileWatcher::~FileWatcher() {
    stop();
}

void FileWatcher::start() {
    if (running_) return;
    
    running_ = true;
    thread_ = std::thread(&FileWatcher::monitorLoop, this);
}

void FileWatcher::stop() {
    if (!running_) return;
    
    running_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }
}

void FileWatcher::monitorLoop() {
    while (running_) {
        checkChanges();
        std::this_thread::sleep_for(interval_);
    }
}

void FileWatcher::checkChanges() {
    if (std::filesystem::is_directory(path_)) {
        // 检查目录中的文件变化
        std::set<std::filesystem::path> currentFiles;
        
        // 检查已删除的文件
        for (const auto& [oldPath, _] : lastModified_) {
            if (!std::filesystem::exists(oldPath)) {
                callback_(oldPath, "deleted");
            }
        }
        lastModified_.clear();
        
        // 检查当前文件
        for (const auto& entry : std::filesystem::directory_iterator(path_)) {
            if (!entry.is_regular_file()) continue;
            
            auto currentPath = entry.path();
            currentFiles.insert(currentPath);
            
            auto it = lastModified_.find(currentPath);
            if (it == lastModified_.end()) {
                // 新文件
                lastModified_[currentPath] = entry.last_write_time();
                callback_(currentPath, "created");
            } else if (it->second != entry.last_write_time()) {
                // 文件修改
                callback_(currentPath, "modified");
                it->second = entry.last_write_time();
            }
        }
        
        // 检查删除的文件
        for (auto it = lastModified_.begin(); it != lastModified_.end();) {
            if (currentFiles.find(it->first) == currentFiles.end()) {
                callback_(it->first, "deleted");
                it = lastModified_.erase(it);
            } else {
                ++it;
            }
        }
    } else {
        // 检查单个文件
        if (!std::filesystem::exists(path_)) {
            callback_(path_, "deleted");
            return;
        }
        
        auto currentTime = std::filesystem::last_write_time(path_);
        auto it = lastModified_.find(path_);
        
        if (it == lastModified_.end()) {
            lastModified_[path_] = currentTime;
            callback_(path_, "created");
        } else if (it->second != currentTime) {
            callback_(path_, "modified");
            it->second = currentTime;
        }
    }
}

// ServerFileUtils 实现
ServerFileUtils& ServerFileUtils::getInstance() {
    static ServerFileUtils instance;
    return instance;
}

ServerFileUtils::ServerFileUtils() : nextWatchId_(0) {
    // 创建临时目录
    tempDir_ = std::filesystem::temp_directory_path() / "fantasy_legend";
    std::filesystem::create_directories(tempDir_);
}

ServerFileUtils::~ServerFileUtils() {
    stopAllWatching();
}

bool ServerFileUtils::exists(const std::filesystem::path& path) {
    try {
        return std::filesystem::exists(path);
    } catch (const std::exception& e) {
        std::cerr << "Exists error: " << e.what() << std::endl;
        return false;
    }
}

bool ServerFileUtils::isDirectory(const std::filesystem::path& path) {
    try {
        return std::filesystem::is_directory(path);
    } catch (const std::exception& e) {
        std::cerr << "Is directory error: " << e.what() << std::endl;
        return false;
    }
}

bool ServerFileUtils::isFile(const std::filesystem::path& path) {
    try {
        return std::filesystem::is_regular_file(path);
    } catch (const std::exception& e) {
        std::cerr << "Is file error: " << e.what() << std::endl;
        return false;
    }
}

bool ServerFileUtils::createDirectory(const std::filesystem::path& path, bool recursive) {
    try {
        if (recursive) {
            std::filesystem::create_directories(path);
        } else {
            std::filesystem::create_directory(path);
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Create directory error: " << e.what() << std::endl;
        return false;
    }
}

bool ServerFileUtils::deleteFile(const std::filesystem::path& path) {
    try {
        if (!std::filesystem::exists(path)) {
            return false;
        }
        
        std::filesystem::remove(path);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Delete file error: " << e.what() << std::endl;
        return false;
    }
}

bool ServerFileUtils::deleteDirectory(const std::filesystem::path& path) {
    try {
        if (!std::filesystem::exists(path)) {
            return false;
        }
        
        std::filesystem::remove_all(path);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Delete directory error: " << e.what() << std::endl;
        return false;
    }
}

bool ServerFileUtils::copyFile(const std::filesystem::path& source, const std::filesystem::path& destination) {
    try {
        if (!std::filesystem::exists(source)) {
            return false;
        }
        
        std::filesystem::create_directories(destination.parent_path());
        std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Copy file error: " << e.what() << std::endl;
        return false;
    }
}

bool ServerFileUtils::moveFile(const std::filesystem::path& source, const std::filesystem::path& destination) {
    try {
        if (!std::filesystem::exists(source)) {
            return false;
        }
        
        std::filesystem::create_directories(destination.parent_path());
        std::filesystem::rename(source, destination);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Move file error: " << e.what() << std::endl;
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
        std::cerr << "Remove error: " << e.what() << std::endl;
        return false;
    }
}

bool ServerFileUtils::copy(const std::filesystem::path& source, const std::filesystem::path& destination, bool recursive) {
    try {
        if (recursive) {
            std::filesystem::copy(source, destination, std::filesystem::copy_options::recursive);
        } else {
            std::filesystem::copy(source, destination);
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Copy error: " << e.what() << std::endl;
        return false;
    }
}

bool ServerFileUtils::move(const std::filesystem::path& source, const std::filesystem::path& destination) {
    try {
        std::filesystem::rename(source, destination);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Move error: " << e.what() << std::endl;
        return false;
    }
}

std::uintmax_t ServerFileUtils::getFileSize(const std::filesystem::path& path) {
    try {
        if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
            return 0;
        }
        return std::filesystem::file_size(path);
    } catch (const std::exception& e) {
        std::cerr << "Get file size error: " << e.what() << std::endl;
        return 0;
    }
}

std::filesystem::file_time_type ServerFileUtils::getLastModifiedTime(const std::filesystem::path& path) {
    try {
        return std::filesystem::last_write_time(path);
    } catch (const std::exception& e) {
        std::cerr << "Get last modified time error: " << e.what() << std::endl;
        return std::filesystem::file_time_type::min();
    }
}

std::vector<std::filesystem::path> ServerFileUtils::listFiles(const std::filesystem::path& directory, const std::string& extension) {
    std::vector<std::filesystem::path> files;
    
    try {
        if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
            return files;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                if (extension.empty() || entry.path().extension() == extension) {
                    files.push_back(entry.path());
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "List files error: " << e.what() << std::endl;
    }
    
    return files;
}

std::vector<std::filesystem::path> ServerFileUtils::listDirectories(const std::filesystem::path& directory) {
    std::vector<std::filesystem::path> directories;
    
    try {
        if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
            return directories;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_directory()) {
                directories.push_back(entry.path());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "List directories error: " << e.what() << std::endl;
    }
    
    return directories;
}

std::vector<std::filesystem::path> ServerFileUtils::getFiles(const std::filesystem::path& path, bool recursive) {
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
        std::cerr << "Get files error: " << e.what() << std::endl;
    }
    
    return files;
}

std::vector<std::filesystem::path> ServerFileUtils::getDirectories(const std::filesystem::path& path, bool recursive) {
    std::vector<std::filesystem::path> directories;
    
    try {
        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
                if (entry.is_directory()) {
                    directories.push_back(entry.path());
                }
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.is_directory()) {
                    directories.push_back(entry.path());
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Get directories error: " << e.what() << std::endl;
    }
    
    return directories;
}

std::filesystem::path ServerFileUtils::createTempFile(const std::string& prefix, const std::string& extension) {
    try {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(1000, 9999);
        
        std::stringstream ss;
        ss << prefix << "_" << dis(gen) << extension;
        
        // 使用单例实例的tempDir_
        auto& instance = getInstance();
        std::filesystem::path tempPath = instance.tempDir_ / ss.str();
        std::ofstream file(tempPath);
        file.close();
        
        return tempPath;
    } catch (const std::exception& e) {
        std::cerr << "Create temp file error: " << e.what() << std::endl;
        return std::filesystem::path();
    }
}

std::filesystem::path ServerFileUtils::createTempDirectory(const std::string& prefix) {
    try {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(1000, 9999);
        
        std::string randomStr = std::to_string(dis(gen));
        
        // 使用单例实例的tempDir_
        auto& instance = getInstance();
        std::filesystem::path tempPath = instance.tempDir_ / (prefix + "_" + randomStr);
        
        std::filesystem::create_directories(tempPath);
        return tempPath;
    } catch (const std::exception& e) {
        std::cerr << "Create temp directory error: " << e.what() << std::endl;
        return std::filesystem::path();
    }
}

void ServerFileUtils::cleanupTempFiles(std::chrono::hours maxAge) {
    try {
        // 使用单例实例的tempDir_
        auto& instance = getInstance();
        for (const auto& entry : std::filesystem::directory_iterator(instance.tempDir_)) {
            if (entry.is_regular_file()) {
                auto fileTime = std::chrono::system_clock::from_time_t(
                    std::chrono::duration_cast<std::chrono::seconds>(
                        std::filesystem::last_write_time(entry.path()).time_since_epoch()
                    ).count());
                auto now = std::chrono::system_clock::now();
                
                if (now - fileTime > maxAge) {
                    std::filesystem::remove(entry.path());
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Cleanup temp files error: " << e.what() << std::endl;
    }
}

size_t ServerFileUtils::startWatching(const std::filesystem::path& path, FileChangeCallback callback, std::chrono::milliseconds interval) {
    size_t watchId = nextWatchId_++;
    
    auto watcher = std::make_unique<FileWatcher>(path, std::move(callback), interval);
    watcher->start();
    watchers_[watchId] = std::move(watcher);
    
    return watchId;
}

void ServerFileUtils::stopWatching(size_t watchId) {
    auto it = watchers_.find(watchId);
    if (it != watchers_.end()) {
        it->second->stop();
        watchers_.erase(it);
    }
}

void ServerFileUtils::stopAllWatching() {
    for (auto& [_, watcher] : watchers_) {
        watcher->stop();
    }
    watchers_.clear();
}

} // namespace Fantasy 