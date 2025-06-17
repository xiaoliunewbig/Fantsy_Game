/**
 * @file ResourceCompressor.h
 * @brief 资源压缩系统
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace Fantasy {

/**
 * @brief 压缩算法类型
 */
enum class CompressionAlgorithm {
    NONE,       ///< 无压缩
    LZ4,        ///< LZ4压缩
    ZSTD,       ///< Zstandard压缩
    LZMA,       ///< LZMA压缩
    GZIP,       ///< Gzip压缩
    BROTLI,     ///< Brotli压缩
    ZLIB,       ///< Zlib压缩
    UNKNOWN     ///< 未知算法
};

/**
 * @brief 压缩级别
 */
enum class CompressionLevel {
    FASTEST = 1,    ///< 最快压缩
    FAST = 3,       ///< 快速压缩
    DEFAULT = 6,    ///< 默认压缩
    GOOD = 9,       ///< 良好压缩
    BEST = 12       ///< 最佳压缩
};

/**
 * @brief 压缩统计信息
 */
struct CompressionStats {
    uint64_t originalSize;      ///< 原始大小
    uint64_t compressedSize;    ///< 压缩后大小
    float compressionRatio;     ///< 压缩比
    float compressionSpeed;     ///< 压缩速度 (MB/s)
    float decompressionSpeed;   ///< 解压速度 (MB/s)
    uint64_t compressionTime;   ///< 压缩时间 (ms)
    uint64_t decompressionTime; ///< 解压时间 (ms)
};

/**
 * @brief 压缩选项
 */
struct CompressionOptions {
    CompressionAlgorithm algorithm = CompressionAlgorithm::ZSTD;
    CompressionLevel level = CompressionLevel::DEFAULT;
    bool enableChecksum = true;         ///< 启用校验和
    bool enableDictionary = false;      ///< 启用字典压缩
    uint32_t dictionarySize = 0;        ///< 字典大小
    uint32_t blockSize = 64 * 1024;     ///< 块大小
    bool enableMultithreading = true;   ///< 启用多线程
    uint32_t threadCount = 0;           ///< 线程数量（0表示自动）
};

/**
 * @brief 资源压缩器接口
 */
class IResourceCompressor {
public:
    virtual ~IResourceCompressor() = default;

    /**
     * @brief 压缩数据
     * @param input 输入数据
     * @param output 输出数据
     * @param options 压缩选项
     * @return 是否成功
     */
    virtual bool compress(const std::vector<uint8_t>& input,
                         std::vector<uint8_t>& output,
                         const CompressionOptions& options) = 0;

    /**
     * @brief 解压数据
     * @param input 输入数据
     * @param output 输出数据
     * @return 是否成功
     */
    virtual bool decompress(const std::vector<uint8_t>& input,
                           std::vector<uint8_t>& output) = 0;

    /**
     * @brief 获取压缩统计信息
     * @return 统计信息
     */
    virtual CompressionStats getStats() const = 0;

    /**
     * @brief 重置统计信息
     */
    virtual void resetStats() = 0;

    /**
     * @brief 获取支持的算法
     * @return 支持的算法列表
     */
    virtual std::vector<CompressionAlgorithm> getSupportedAlgorithms() const = 0;
};

/**
 * @brief LZ4压缩器
 */
class LZ4Compressor : public IResourceCompressor {
public:
    bool compress(const std::vector<uint8_t>& input,
                 std::vector<uint8_t>& output,
                 const CompressionOptions& options) override;
    bool decompress(const std::vector<uint8_t>& input,
                   std::vector<uint8_t>& output) override;
    CompressionStats getStats() const override;
    void resetStats() override;
    std::vector<CompressionAlgorithm> getSupportedAlgorithms() const override;

private:
    mutable CompressionStats stats_;
};

/**
 * @brief ZStandard压缩器
 */
class ZSTDCompressor : public IResourceCompressor {
public:
    bool compress(const std::vector<uint8_t>& input,
                 std::vector<uint8_t>& output,
                 const CompressionOptions& options) override;
    bool decompress(const std::vector<uint8_t>& input,
                   std::vector<uint8_t>& output) override;
    CompressionStats getStats() const override;
    void resetStats() override;
    std::vector<CompressionAlgorithm> getSupportedAlgorithms() const override;

private:
    mutable CompressionStats stats_;
};

/**
 * @brief LZMA压缩器
 */
class LZMACompressor : public IResourceCompressor {
public:
    bool compress(const std::vector<uint8_t>& input,
                 std::vector<uint8_t>& output,
                 const CompressionOptions& options) override;
    bool decompress(const std::vector<uint8_t>& input,
                   std::vector<uint8_t>& output) override;
    CompressionStats getStats() const override;
    void resetStats() override;
    std::vector<CompressionAlgorithm> getSupportedAlgorithms() const override;

private:
    mutable CompressionStats stats_;
};

/**
 * @brief 资源压缩管理器
 */
class ResourceCompressor {
public:
    /**
     * @brief 获取单例实例
     * @return ResourceCompressor实例引用
     */
    static ResourceCompressor& getInstance();

    /**
     * @brief 初始化压缩器
     * @return 是否初始化成功
     */
    bool initialize();

    /**
     * @brief 压缩数据
     * @param input 输入数据
     * @param output 输出数据
     * @param options 压缩选项
     * @return 是否成功
     */
    bool compress(const std::vector<uint8_t>& input,
                 std::vector<uint8_t>& output,
                 const CompressionOptions& options = {});

    /**
     * @brief 解压数据
     * @param input 输入数据
     * @param output 输出数据
     * @return 是否成功
     */
    bool decompress(const std::vector<uint8_t>& input,
                   std::vector<uint8_t>& output);

    /**
     * @brief 压缩文件
     * @param inputPath 输入文件路径
     * @param outputPath 输出文件路径
     * @param options 压缩选项
     * @return 是否成功
     */
    bool compressFile(const std::string& inputPath,
                     const std::string& outputPath,
                     const CompressionOptions& options = {});

    /**
     * @brief 解压文件
     * @param inputPath 输入文件路径
     * @param outputPath 输出文件路径
     * @return 是否成功
     */
    bool decompressFile(const std::string& inputPath,
                       const std::string& outputPath);

    /**
     * @brief 获取压缩统计信息
     * @return 统计信息
     */
    CompressionStats getStats() const;

    /**
     * @brief 重置统计信息
     */
    void resetStats();

    /**
     * @brief 获取支持的算法
     * @return 支持的算法列表
     */
    std::vector<CompressionAlgorithm> getSupportedAlgorithms() const;

    /**
     * @brief 测试压缩性能
     * @param data 测试数据
     * @param options 压缩选项
     * @return 性能报告
     */
    std::string benchmarkCompression(const std::vector<uint8_t>& data,
                                    const CompressionOptions& options = {});

    /**
     * @brief 选择最佳压缩算法
     * @param data 数据样本
     * @param targetRatio 目标压缩比
     * @param maxTime 最大压缩时间（ms）
     * @return 最佳算法
     */
    CompressionAlgorithm selectBestAlgorithm(const std::vector<uint8_t>& data,
                                            float targetRatio = 0.5f,
                                            uint64_t maxTime = 1000);

private:
    ResourceCompressor() = default;
    ~ResourceCompressor() = default;

    // 禁用拷贝和移动
    ResourceCompressor(const ResourceCompressor&) = delete;
    ResourceCompressor& operator=(const ResourceCompressor&) = delete;
    ResourceCompressor(ResourceCompressor&&) = delete;
    ResourceCompressor& operator=(ResourceCompressor&&) = delete;

    std::unique_ptr<IResourceCompressor> lz4Compressor_;
    std::unique_ptr<IResourceCompressor> zstdCompressor_;
    std::unique_ptr<IResourceCompressor> lzmaCompressor_;
    mutable CompressionStats globalStats_;
    bool initialized_ = false;
};

} // namespace Fantasy 