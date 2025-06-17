/**
 * @file ResourceVersionControl.h
 * @brief 资源版本控制系统
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#pragma once

#include "ResourceType.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include <filesystem>
#include <chrono>

namespace Fantasy {

/**
 * @brief 版本号格式
 */
struct Version {
    uint32_t major;     ///< 主版本号
    uint32_t minor;     ///< 次版本号
    uint32_t patch;     ///< 补丁版本号
    std::string build;  ///< 构建标识

    Version(uint32_t m = 0, uint32_t n = 0, uint32_t p = 0, const std::string& b = "")
        : major(m), minor(n), patch(p), build(b) {}

    /**
     * @brief 从字符串解析版本号
     * @param versionStr 版本字符串
     * @return 版本号
     */
    static Version fromString(const std::string& versionStr);

    /**
     * @brief 转换为字符串
     * @return 版本字符串
     */
    std::string toString() const;

    /**
     * @brief 比较版本号
     * @param other 其他版本号
     * @return 比较结果
     */
    int compare(const Version& other) const;

    bool operator<(const Version& other) const { return compare(other) < 0; }
    bool operator<=(const Version& other) const { return compare(other) <= 0; }
    bool operator==(const Version& other) const { return compare(other) == 0; }
    bool operator!=(const Version& other) const { return compare(other) != 0; }
    bool operator>=(const Version& other) const { return compare(other) >= 0; }
    bool operator>(const Version& other) const { return compare(other) > 0; }
};

/**
 * @brief 资源版本信息
 */
struct ResourceVersion {
    std::string resourceId;             ///< 资源ID
    std::string path;                   ///< 资源路径
    ResourceType type;                  ///< 资源类型
    Version version;                    ///< 版本号
    std::string checksum;               ///< 校验和
    uint64_t size;                      ///< 文件大小
    std::filesystem::file_time_type lastModified; ///< 最后修改时间
    std::string author;                 ///< 作者
    std::string description;            ///< 描述
    std::vector<std::string> tags;      ///< 标签
    std::vector<std::string> dependencies; ///< 依赖项
    bool isRequired;                    ///< 是否必需
    bool isOptional;                    ///< 是否可选
};

/**
 * @brief 版本差异信息
 */
struct VersionDiff {
    std::string resourceId;             ///< 资源ID
    Version oldVersion;                 ///< 旧版本
    Version newVersion;                 ///< 新版本
    std::string oldChecksum;            ///< 旧校验和
    std::string newChecksum;            ///< 新校验和
    uint64_t sizeDiff;                  ///< 大小差异
    std::vector<std::string> changes;   ///< 变更列表
    std::chrono::system_clock::time_point timestamp; ///< 时间戳
};

/**
 * @brief 版本控制选项
 */
struct VersionControlOptions {
    bool enableAutoVersioning = true;   ///< 启用自动版本控制
    bool enableChecksum = true;         ///< 启用校验和
    bool enableBackup = true;           ///< 启用备份
    bool enableRollback = true;         ///< 启用回滚
    uint32_t maxVersions = 10;          ///< 最大版本数
    std::string backupDir;              ///< 备份目录
    std::string versionFile;            ///< 版本文件
    bool enableCompression = true;      ///< 启用压缩
    bool enableEncryption = false;      ///< 启用加密
    std::string encryptionKey;          ///< 加密密钥
};

/**
 * @brief 版本控制统计信息
 */
struct VersionControlStats {
    uint64_t totalResources;            ///< 总资源数
    uint64_t totalVersions;             ///< 总版本数
    uint64_t totalSize;                 ///< 总大小
    uint64_t backupSize;                ///< 备份大小
    std::chrono::system_clock::time_point lastUpdate; ///< 最后更新时间
    std::unordered_map<ResourceType, uint64_t> typeStats; ///< 各类型统计
    std::vector<VersionDiff> recentChanges; ///< 最近变更
};

/**
 * @brief 资源版本控制器
 */
class ResourceVersionControl {
public:
    /**
     * @brief 获取单例实例
     * @return ResourceVersionControl实例引用
     */
    static ResourceVersionControl& getInstance();

    /**
     * @brief 初始化版本控制器
     * @param options 版本控制选项
     * @return 是否初始化成功
     */
    bool initialize(const VersionControlOptions& options = {});

    /**
     * @brief 添加资源版本
     * @param resourcePath 资源路径
     * @param version 版本号
     * @param author 作者
     * @param description 描述
     * @return 是否成功
     */
    bool addVersion(const std::string& resourcePath,
                   const Version& version,
                   const std::string& author = "",
                   const std::string& description = "");

    /**
     * @brief 获取资源版本
     * @param resourceId 资源ID
     * @param version 版本号
     * @return 版本信息
     */
    const ResourceVersion* getVersion(const std::string& resourceId,
                                    const Version& version) const;

    /**
     * @brief 获取最新版本
     * @param resourceId 资源ID
     * @return 最新版本信息
     */
    const ResourceVersion* getLatestVersion(const std::string& resourceId) const;

    /**
     * @brief 获取所有版本
     * @param resourceId 资源ID
     * @return 版本列表
     */
    std::vector<ResourceVersion> getAllVersions(const std::string& resourceId) const;

    /**
     * @brief 更新资源版本
     * @param resourcePath 资源路径
     * @param newVersion 新版本号
     * @param description 描述
     * @return 是否成功
     */
    bool updateVersion(const std::string& resourcePath,
                      const Version& newVersion,
                      const std::string& description = "");

    /**
     * @brief 回滚到指定版本
     * @param resourceId 资源ID
     * @param version 目标版本
     * @return 是否成功
     */
    bool rollbackToVersion(const std::string& resourceId,
                          const Version& version);

    /**
     * @brief 删除版本
     * @param resourceId 资源ID
     * @param version 版本号
     * @return 是否成功
     */
    bool deleteVersion(const std::string& resourceId,
                      const Version& version);

    /**
     * @brief 清理旧版本
     * @param maxVersions 保留的最大版本数
     * @return 清理的版本数
     */
    uint64_t cleanupOldVersions(uint32_t maxVersions = 5);

    /**
     * @brief 比较版本
     * @param resourceId 资源ID
     * @param version1 版本1
     * @param version2 版本2
     * @return 版本差异
     */
    VersionDiff compareVersions(const std::string& resourceId,
                               const Version& version1,
                               const Version& version2);

    /**
     * @brief 获取版本差异
     * @param resourceId 资源ID
     * @param fromVersion 起始版本
     * @param toVersion 目标版本
     * @return 版本差异列表
     */
    std::vector<VersionDiff> getVersionDiffs(const std::string& resourceId,
                                            const Version& fromVersion,
                                            const Version& toVersion);

    /**
     * @brief 导出版本信息
     * @param outputPath 输出路径
     * @return 是否成功
     */
    bool exportVersionInfo(const std::string& outputPath);

    /**
     * @brief 导入版本信息
     * @param inputPath 输入路径
     * @return 是否成功
     */
    bool importVersionInfo(const std::string& inputPath);

    /**
     * @brief 获取统计信息
     * @return 统计信息
     */
    VersionControlStats getStats() const;

    /**
     * @brief 验证版本完整性
     * @param resourceId 资源ID
     * @param version 版本号
     * @return 是否完整
     */
    bool verifyVersion(const std::string& resourceId,
                      const Version& version);

    /**
     * @brief 备份版本
     * @param resourceId 资源ID
     * @param version 版本号
     * @return 是否成功
     */
    bool backupVersion(const std::string& resourceId,
                      const Version& version);

    /**
     * @brief 恢复版本
     * @param resourceId 资源ID
     * @param version 版本号
     * @return 是否成功
     */
    bool restoreVersion(const std::string& resourceId,
                       const Version& version);

    /**
     * @brief 获取变更历史
     * @param resourceId 资源ID
     * @param limit 限制数量
     * @return 变更历史
     */
    std::vector<VersionDiff> getChangeHistory(const std::string& resourceId,
                                             uint32_t limit = 10);

private:
    ResourceVersionControl() = default;
    ~ResourceVersionControl() = default;

    // 禁用拷贝和移动
    ResourceVersionControl(const ResourceVersionControl&) = delete;
    ResourceVersionControl& operator=(const ResourceVersionControl&) = delete;
    ResourceVersionControl(ResourceVersionControl&&) = delete;
    ResourceVersionControl& operator=(ResourceVersionControl&&) = delete;

    /**
     * @brief 计算文件校验和
     * @param filePath 文件路径
     * @return 校验和
     */
    std::string calculateFileChecksum(const std::string& filePath);

    /**
     * @brief 保存版本信息
     * @return 是否成功
     */
    bool saveVersionInfo();

    /**
     * @brief 加载版本信息
     * @return 是否成功
     */
    bool loadVersionInfo();

    /**
     * @brief 创建备份
     * @param resourcePath 资源路径
     * @param version 版本号
     * @return 备份路径
     */
    std::string createBackup(const std::string& resourcePath,
                            const Version& version);

    /**
     * @brief 恢复备份
     * @param backupPath 备份路径
     * @param targetPath 目标路径
     * @return 是否成功
     */
    bool restoreBackup(const std::string& backupPath,
                      const std::string& targetPath);

    VersionControlOptions options_;     ///< 版本控制选项
    std::unordered_map<std::string, std::vector<ResourceVersion>> versions_; ///< 版本信息
    std::vector<VersionDiff> changeHistory_; ///< 变更历史
    VersionControlStats stats_;         ///< 统计信息
    bool initialized_ = false;          ///< 是否已初始化
};

} // namespace Fantasy 