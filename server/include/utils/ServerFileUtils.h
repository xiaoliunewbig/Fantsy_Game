/**
 * @file ServerFileUtils.h
 * @brief 服务器端文件操作工具类
 * @details 提供服务器端所需的文件操作功能，使用标准库实现
 * @author [pengchengkang] 
 * @date 2025.06.16
 */

#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <filesystem>
#include <optional>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <map>
#include <set>
#include <fstream>

namespace Fantasy {

// 前向声明
class FileWatcher;

/**
 * @brief 文件监控回调函数类型
 */
using FileChangeCallback = std::function<void(const std::filesystem::path&, const std::string&)>;

/**
 * @brief 文件监控器类
 */
class FileWatcher {
public:
    /**
     * @brief 构造函数
     * @param path 要监控的路径
     * @param callback 文件变化回调函数
     * @param interval 检查间隔（毫秒）
     */
    FileWatcher(const std::filesystem::path& path, 
                FileChangeCallback callback,
                std::chrono::milliseconds interval = std::chrono::seconds(1));
    
    /**
     * @brief 析构函数
     */
    ~FileWatcher();
    
    /**
     * @brief 开始监控
     */
    void start();
    
    /**
     * @brief 停止监控
     */
    void stop();
    
    /**
     * @brief 检查是否正在监控
     * @return true表示正在监控
     */
    bool isWatching() const { return running_; }

private:
    void monitorLoop();
    void checkChanges();

private:
    std::filesystem::path path_;           ///< 监控路径
    FileChangeCallback callback_;          ///< 回调函数
    std::chrono::milliseconds interval_;   ///< 检查间隔
    std::thread thread_;                   ///< 监控线程
    std::atomic<bool> running_;            ///< 运行标志
    std::mutex mutex_;                     ///< 互斥锁
    std::map<std::filesystem::path, std::filesystem::file_time_type> lastModified_; ///< 上次修改时间
};

/**
 * @brief 服务器端文件工具类
 */
class ServerFileUtils {
public:
    /**
     * @brief 获取单例实例
     * @return ServerFileUtils实例引用
     */
    static ServerFileUtils& getInstance();
    
    /**
     * @brief 检查文件是否存在
     * @param path 文件路径
     * @return true表示存在
     */
    static bool exists(const std::filesystem::path& path);
    
    /**
     * @brief 检查是否为目录
     * @param path 路径
     * @return true表示是目录
     */
    static bool isDirectory(const std::filesystem::path& path);
    
    /**
     * @brief 检查是否为文件
     * @param path 路径
     * @return true表示是文件
     */
    static bool isFile(const std::filesystem::path& path);
    
    /**
     * @brief 创建目录
     * @param path 目录路径
     * @param recursive 是否递归创建
     * @return true表示成功
     */
    static bool createDirectory(const std::filesystem::path& path, bool recursive = true);
    
    /**
     * @brief 删除文件
     * @param path 文件路径
     * @return true表示成功
     */
    static bool deleteFile(const std::filesystem::path& path);
    
    /**
     * @brief 删除目录
     * @param path 目录路径
     * @return true表示成功
     */
    static bool deleteDirectory(const std::filesystem::path& path);
    
    /**
     * @brief 复制文件
     * @param source 源路径
     * @param destination 目标路径
     * @return true表示成功
     */
    static bool copyFile(const std::filesystem::path& source, const std::filesystem::path& destination);
    
    /**
     * @brief 移动文件
     * @param source 源路径
     * @param destination 目标路径
     * @return true表示成功
     */
    static bool moveFile(const std::filesystem::path& source, const std::filesystem::path& destination);
    
    /**
     * @brief 删除文件或目录
     * @param path 路径
     * @param recursive 是否递归删除目录
     * @return true表示成功
     */
    static bool remove(const std::filesystem::path& path, bool recursive = false);
    
    /**
     * @brief 复制文件或目录
     * @param source 源路径
     * @param destination 目标路径
     * @param recursive 是否递归复制目录
     * @return true表示成功
     */
    static bool copy(const std::filesystem::path& source, 
                    const std::filesystem::path& destination,
                    bool recursive = false);
    
    /**
     * @brief 移动文件或目录
     * @param source 源路径
     * @param destination 目标路径
     * @return true表示成功
     */
    static bool move(const std::filesystem::path& source, const std::filesystem::path& destination);
    
    /**
     * @brief 获取文件大小
     * @param path 文件路径
     * @return 文件大小（字节）
     */
    static std::uintmax_t getFileSize(const std::filesystem::path& path);
    
    /**
     * @brief 获取文件最后修改时间
     * @param path 文件路径
     * @return 最后修改时间
     */
    static std::filesystem::file_time_type getLastModifiedTime(const std::filesystem::path& path);
    
    /**
     * @brief 列出目录中的文件
     * @param directory 目录路径
     * @param extension 文件扩展名过滤
     * @return 文件路径列表
     */
    static std::vector<std::filesystem::path> listFiles(const std::filesystem::path& directory, const std::string& extension = "");
    
    /**
     * @brief 列出目录中的子目录
     * @param directory 目录路径
     * @return 目录路径列表
     */
    static std::vector<std::filesystem::path> listDirectories(const std::filesystem::path& directory);
    
    /**
     * @brief 获取目录中的所有文件
     * @param path 目录路径
     * @param recursive 是否递归获取
     * @return 文件路径列表
     */
    static std::vector<std::filesystem::path> getFiles(const std::filesystem::path& path, 
                                                     bool recursive = false);
    
    /**
     * @brief 获取目录中的所有子目录
     * @param path 目录路径
     * @param recursive 是否递归获取
     * @return 目录路径列表
     */
    static std::vector<std::filesystem::path> getDirectories(const std::filesystem::path& path,
                                                           bool recursive = false);
    
    /**
     * @brief 创建临时文件
     * @param prefix 文件名前缀
     * @param extension 文件扩展名
     * @return 临时文件路径
     */
    static std::filesystem::path createTempFile(const std::string& prefix = "temp",
                                              const std::string& extension = "");
    
    /**
     * @brief 创建临时目录
     * @param prefix 目录名前缀
     * @return 临时目录路径
     */
    static std::filesystem::path createTempDirectory(const std::string& prefix = "temp");
    
    /**
     * @brief 清理临时文件
     * @param maxAge 清理指定时间之前的文件
     */
    static void cleanupTempFiles(std::chrono::hours maxAge = std::chrono::hours(24));
    
    /**
     * @brief 开始监控文件变化
     * @param path 要监控的路径
     * @param callback 文件变化回调函数
     * @param interval 检查间隔
     * @return 监控器ID
     */
    size_t startWatching(const std::filesystem::path& path,
                        FileChangeCallback callback,
                        std::chrono::milliseconds interval = std::chrono::seconds(1));
    
    /**
     * @brief 停止监控
     * @param watchId 监控器ID
     */
    void stopWatching(size_t watchId);
    
    /**
     * @brief 停止所有监控
     */
    void stopAllWatching();

private:
    ServerFileUtils();
    ~ServerFileUtils();
    
    // 禁用拷贝和移动
    ServerFileUtils(const ServerFileUtils&) = delete;
    ServerFileUtils& operator=(const ServerFileUtils&) = delete;
    ServerFileUtils(ServerFileUtils&&) = delete;
    ServerFileUtils& operator=(ServerFileUtils&&) = delete;

private:
    std::mutex mutex_;                     ///< 互斥锁
    std::map<size_t, std::unique_ptr<FileWatcher>> watchers_; ///< 文件监控器
    size_t nextWatchId_;                   ///< 下一个监控器ID
    std::filesystem::path tempDir_;        ///< 临时目录
};

} // namespace Fantasy 