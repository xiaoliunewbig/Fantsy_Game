/**
 * @file ResourcePackager.h
 * @brief 资源打包系统
 * @author [pengchengkang]
 * @date 2025.06.16
 */

#pragma once

#include "ResourceType.h"
#include "ResourceCompressor.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include <filesystem>

namespace Fantasy {

/**
 * @brief 打包格式
 */
enum class PackageFormat {
    FANTASY_PACK,   ///< Fantasy自定义格式
    ZIP,            ///< ZIP格式
    TAR_GZ,         ///< TAR+GZIP格式
    SEVEN_ZIP,      ///< 7-Zip格式
    UNKNOWN         ///< 未知格式
};

/**
 * @brief 打包选项
 */
struct PackageOptions {
    PackageFormat format = PackageFormat::FANTASY_PACK;
    CompressionOptions compression;
    bool enableEncryption = false;       ///< 启用加密
    std::string encryptionKey;           ///< 加密密钥
    bool enableChecksum = true;          ///< 启用校验和
    bool enableIndex = true;             ///< 启用索引
    uint32_t blockSize = 64 * 1024;      ///< 块大小
    bool enableMultithreading = true;    ///< 启用多线程
    uint32_t threadCount = 0;            ///< 线程数量
};

/**
 * @brief 打包条目信息
 */
struct PackageEntry {
    std::string name;                    ///< 条目名称
    std::string path;                    ///< 文件路径
    ResourceType type;                   ///< 资源类型
    uint64_t offset;                     ///< 数据偏移
    uint64_t size;                       ///< 原始大小
    uint64_t compressedSize;             ///< 压缩后大小
    CompressionAlgorithm algorithm;      ///< 压缩算法
    std::string checksum;                ///< 校验和
    std::filesystem::file_time_type lastModified; ///< 最后修改时间
    bool isCompressed;                   ///< 是否压缩
    bool isEncrypted;                    ///< 是否加密
};

/**
 * @brief 打包文件头
 */
struct PackageHeader {
    char magic[8];                       ///< 魔数 "FANTASY"
    uint32_t version;                    ///< 版本号
    PackageFormat format;                ///< 打包格式
    uint64_t entryCount;                 ///< 条目数量
    uint64_t indexOffset;                ///< 索引偏移
    uint64_t indexSize;                  ///< 索引大小
    std::string checksum;                ///< 文件校验和
    uint64_t creationTime;               ///< 创建时间
    uint64_t totalSize;                  ///< 总大小
    uint64_t compressedSize;             ///< 压缩后总大小
};

/**
 * @brief 打包统计信息
 */
struct PackageStats {
    uint64_t totalFiles;                 ///< 总文件数
    uint64_t totalSize;                  ///< 总大小
    uint64_t compressedSize;             ///< 压缩后大小
    float compressionRatio;              ///< 压缩比
    uint64_t creationTime;               ///< 创建时间
    uint64_t readTime;                   ///< 读取时间
    uint64_t writeTime;                  ///< 写入时间
    std::unordered_map<ResourceType, uint64_t> typeStats; ///< 各类型统计
};

/**
 * @brief 资源打包器
 */
class ResourcePackager {
public:
    /**
     * @brief 获取单例实例
     * @return ResourcePackager实例引用
     */
    static ResourcePackager& getInstance();

    /**
     * @brief 初始化打包器
     * @return 是否初始化成功
     */
    bool initialize();

    /**
     * @brief 创建资源包
     * @param packagePath 包文件路径
     * @param resourcePaths 资源文件路径列表
     * @param options 打包选项
     * @return 是否成功
     */
    bool createPackage(const std::string& packagePath,
                      const std::vector<std::string>& resourcePaths,
                      const PackageOptions& options = {});

    /**
     * @brief 打开资源包
     * @param packagePath 包文件路径
     * @return 是否成功
     */
    bool openPackage(const std::string& packagePath);

    /**
     * @brief 关闭资源包
     */
    void closePackage();

    /**
     * @brief 提取资源
     * @param entryName 条目名称
     * @param outputPath 输出路径
     * @return 是否成功
     */
    bool extractResource(const std::string& entryName,
                        const std::string& outputPath);

    /**
     * @brief 提取所有资源
     * @param outputDir 输出目录
     * @return 是否成功
     */
    bool extractAll(const std::string& outputDir);

    /**
     * @brief 获取资源数据
     * @param entryName 条目名称
     * @return 资源数据
     */
    std::vector<uint8_t> getResourceData(const std::string& entryName);

    /**
     * @brief 列出所有条目
     * @return 条目列表
     */
    std::vector<PackageEntry> listEntries() const;

    /**
     * @brief 查找条目
     * @param name 条目名称
     * @return 条目信息
     */
    const PackageEntry* findEntry(const std::string& name) const;

    /**
     * @brief 检查条目是否存在
     * @param name 条目名称
     * @return 是否存在
     */
    bool hasEntry(const std::string& name) const;

    /**
     * @brief 获取包统计信息
     * @return 统计信息
     */
    PackageStats getStats() const;

    /**
     * @brief 验证包完整性
     * @return 是否完整
     */
    bool verifyPackage();

    /**
     * @brief 更新包索引
     * @return 是否成功
     */
    bool updateIndex();

    /**
     * @brief 优化包文件
     * @return 是否成功
     */
    bool optimizePackage();

    /**
     * @brief 获取支持的格式
     * @return 支持的格式列表
     */
    std::vector<PackageFormat> getSupportedFormats() const;

    /**
     * @brief 转换包格式
     * @param outputPath 输出路径
     * @param targetFormat 目标格式
     * @return 是否成功
     */
    bool convertFormat(const std::string& outputPath,
                      PackageFormat targetFormat);

private:
    ResourcePackager() = default;
    ~ResourcePackager() = default;

    // 禁用拷贝和移动
    ResourcePackager(const ResourcePackager&) = delete;
    ResourcePackager& operator=(const ResourcePackager&) = delete;
    ResourcePackager(ResourcePackager&&) = delete;
    ResourcePackager& operator=(ResourcePackager&&) = delete;

    /**
     * @brief 写入包文件头
     * @param file 文件流
     * @param header 文件头
     * @return 是否成功
     */
    bool writeHeader(std::ofstream& file, const PackageHeader& header);

    /**
     * @brief 读取包文件头
     * @param file 文件流
     * @param header 文件头
     * @return 是否成功
     */
    bool readHeader(std::ifstream& file, PackageHeader& header);

    /**
     * @brief 写入条目索引
     * @param file 文件流
     * @param entries 条目列表
     * @return 是否成功
     */
    bool writeIndex(std::ofstream& file, const std::vector<PackageEntry>& entries);

    /**
     * @brief 读取条目索引
     * @param file 文件流
     * @param entryCount 条目数量
     * @return 条目列表
     */
    std::vector<PackageEntry> readIndex(std::ifstream& file, uint64_t entryCount);

    /**
     * @brief 计算文件校验和
     * @param data 数据
     * @return 校验和
     */
    std::string calculateChecksum(const std::vector<uint8_t>& data);

    /**
     * @brief 加密数据
     * @param data 数据
     * @param key 密钥
     * @return 加密后数据
     */
    std::vector<uint8_t> encryptData(const std::vector<uint8_t>& data,
                                    const std::string& key);

    /**
     * @brief 解密数据
     * @param data 数据
     * @param key 密钥
     * @return 解密后数据
     */
    std::vector<uint8_t> decryptData(const std::vector<uint8_t>& data,
                                    const std::string& key);

    std::string currentPackagePath_;     ///< 当前包文件路径
    PackageHeader header_;               ///< 包文件头
    std::vector<PackageEntry> entries_;  ///< 条目列表
    std::unordered_map<std::string, size_t> entryMap_; ///< 条目映射
    PackageStats stats_;                 ///< 统计信息
    bool isOpen_ = false;                ///< 是否已打开
    bool initialized_ = false;           ///< 是否已初始化
};

} // namespace Fantasy 