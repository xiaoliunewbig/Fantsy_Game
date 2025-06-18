/**
 * @file FileSink.h
 * @brief 文件日志输出器
 * @details 将日志消息输出到文件，支持文件轮转、自动创建目录和日志分类
 * @author [pengchengkang] 
 * @date 2025.06.16
 */

#pragma once

#include "LogSink.h"
#include "LogType.h"
#include <fstream>
#include <string>
#include <chrono>
#include <mutex>
#include <thread>
#include <atomic>

namespace Fantasy {

/**
 * @brief 文件轮转策略
 */
struct RotationPolicy {
    size_t maxFileSize = 10 * 1024 * 1024;  ///< 单个文件最大大小（字节）
    size_t maxFiles = 5;                    ///< 最大文件数量
    std::chrono::hours maxAge{12};          ///< 文件最大保留时间（12小时）
    bool rotateOnStart = false;             ///< 启动时是否轮转
    std::chrono::minutes cleanupInterval{60}; ///< 清理检查间隔（1小时）
};

/**
 * @brief 文件日志输出器
 * @details 将日志消息输出到指定文件，支持文件轮转、缓冲输出和日志分类
 */
class FileSink : public LogSink {
public:
    /**
     * @brief 构造函数
     * @param baseDir 日志基础目录
     * @param type 日志类型
     * @param policy 轮转策略
     * @param append 是否追加模式，默认true
     * @param autoFlush 是否自动刷新，默认true
     */
    explicit FileSink(const std::string& baseDir,
                     LogType type,
                     const RotationPolicy& policy = RotationPolicy(),
                     bool append = true,
                     bool autoFlush = true);
    
    /**
     * @brief 析构函数
     */
    ~FileSink() override;
    
    /**
     * @brief 输出日志消息到文件
     * @param level 日志等级
     * @param message 日志消息内容
     */
    void log(LogLevel level, const std::string& message) override;
    
    /**
     * @brief 刷新文件缓冲区
     */
    void flush() override;
    
    /**
     * @brief 检查文件输出器是否可用
     * @return true表示文件打开成功，false表示文件打开失败
     */
    bool isAvailable() const override;
    
    /**
     * @brief 重新打开日志文件
     * @return true表示成功，false表示失败
     */
    bool reopen();
    
    /**
     * @brief 设置基础目录
     * @param baseDir 新的基础目录
     * @return true表示成功，false表示失败
     */
    bool setBaseDir(const std::string& baseDir);
    
    /**
     * @brief 获取当前基础目录
     * @return 基础目录
     */
    const std::string& getBaseDir() const { return baseDir_; }
    
    /**
     * @brief 设置日志类型
     * @param type 新的日志类型
     * @return true表示成功，false表示失败
     */
    bool setLogType(LogType type);
    
    /**
     * @brief 获取当前日志类型
     * @return 日志类型
     */
    LogType getLogType() const { return type_; }
    
    /**
     * @brief 设置是否自动刷新
     * @param autoFlush 是否自动刷新
     */
    void setAutoFlush(bool autoFlush) { autoFlush_ = autoFlush; }
    
    /**
     * @brief 获取是否自动刷新
     * @return true表示自动刷新，false表示手动刷新
     */
    bool isAutoFlush() const { return autoFlush_; }
    
    /**
     * @brief 设置轮转策略
     * @param policy 新的轮转策略
     */
    void setRotationPolicy(const RotationPolicy& policy);
    
    /**
     * @brief 获取当前轮转策略
     * @return 轮转策略
     */
    const RotationPolicy& getRotationPolicy() const { return policy_; }
    
    /**
     * @brief 强制轮转日志文件
     * @return true表示成功，false表示失败
     */
    bool rotate();
    
    /**
     * @brief 启动自动清理线程
     */
    void startCleanupThread();
    
    /**
     * @brief 停止自动清理线程
     */
    void stopCleanupThread();

private:
    /**
     * @brief 创建日志文件目录
     * @return true表示成功，false表示失败
     */
    bool createDirectory();
    
    /**
     * @brief 打开日志文件
     * @return true表示成功，false表示失败
     */
    bool openFile();
    
    /**
     * @brief 检查是否需要轮转
     * @return true表示需要轮转
     */
    bool shouldRotate() const;
    
    /**
     * @brief 清理旧文件
     */
    void cleanupOldFiles();
    
    /**
     * @brief 获取轮转后的文件名
     * @param index 文件索引
     * @return 轮转后的文件名
     */
    std::string getRotatedFileName(size_t index) const;
    
    /**
     * @brief 生成当前日志文件名
     * @return 日志文件名
     */
    std::string generateCurrentFileName() const;
    
    /**
     * @brief 清理线程函数
     */
    void cleanupThreadFunc();

private:
    std::string baseDir_;                  ///< 日志基础目录
    LogType type_;                         ///< 日志类型
    std::ofstream fileStream_;             ///< 文件流
    bool append_;                          ///< 是否追加模式
    bool autoFlush_;                       ///< 是否自动刷新
    bool isOpen_;                          ///< 文件是否打开
    RotationPolicy policy_;                ///< 轮转策略
    std::chrono::system_clock::time_point lastRotationTime_; ///< 上次轮转时间
    size_t currentFileSize_;               ///< 当前文件大小
    std::string currentFileName_;          ///< 当前文件名
    std::mutex mutex_;                     ///< 互斥锁
    std::thread cleanupThread_;            ///< 清理线程
    std::atomic<bool> cleanupRunning_;     ///< 清理线程运行标志
};

} // namespace Fantasy