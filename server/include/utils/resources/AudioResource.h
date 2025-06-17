/**
 * @file AudioResource.h
 * @brief 音频资源类
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#pragma once

#include "ResourceType.h"
#include <string>
#include <memory>
#include <filesystem>
#include <vector>
#include <cstdint>

namespace Fantasy {

/**
 * @brief 音频格式
 */
enum class AudioFormat {
    WAV,        ///< WAV格式
    MP3,        ///< MP3格式
    OGG,        ///< OGG格式
    FLAC,       ///< FLAC格式
    AAC,        ///< AAC格式
    UNKNOWN     ///< 未知格式
};

/**
 * @brief 音频类型
 */
enum class AudioType {
    SOUND,      ///< 音效
    MUSIC,      ///< 音乐
    VOICE,      ///< 语音
    AMBIENT     ///< 环境音
};

/**
 * @brief 音频数据
 */
struct AudioData {
    std::vector<uint8_t> samples;        ///< 音频样本数据
    uint32_t sampleRate;                 ///< 采样率
    uint16_t channels;                   ///< 声道数
    uint16_t bitsPerSample;              ///< 位深度
    AudioFormat format;                  ///< 音频格式
    AudioType type;                      ///< 音频类型
    float duration;                      ///< 持续时间（秒）
    uint32_t totalSamples;               ///< 总样本数
    bool isCompressed;                   ///< 是否压缩
    std::vector<uint8_t> compressedData; ///< 压缩数据
};

/**
 * @brief 音频资源类
 * @details 用于加载和管理音频资源
 */
class AudioResource : public IResource {
public:
    /**
     * @brief 构造函数
     * @param path 音频文件路径
     * @param type 音频类型
     */
    explicit AudioResource(const std::filesystem::path& path, AudioType type = AudioType::SOUND);

    // IResource接口实现
    const std::string& getId() const override;
    ResourceType getType() const override;
    ResourceState getState() const override;
    const std::filesystem::path& getPath() const override;
    std::filesystem::file_time_type getLastModifiedTime() const override;
    std::uintmax_t getSize() const override;
    bool load() override;
    void unload() override;
    bool reload() override;
    bool needsReload() const override;

    /**
     * @brief 获取音频数据
     * @return 音频数据
     */
    const AudioData& getData() const;

    /**
     * @brief 获取音频格式
     * @return 音频格式
     */
    AudioFormat getFormat() const;

    /**
     * @brief 获取音频类型
     * @return 音频类型
     */
    AudioType getAudioType() const;

    /**
     * @brief 获取采样率
     * @return 采样率
     */
    uint32_t getSampleRate() const;

    /**
     * @brief 获取声道数
     * @return 声道数
     */
    uint16_t getChannels() const;

    /**
     * @brief 获取位深度
     * @return 位深度
     */
    uint16_t getBitsPerSample() const;

    /**
     * @brief 获取持续时间
     * @return 持续时间（秒）
     */
    float getDuration() const;

    /**
     * @brief 获取总样本数
     * @return 总样本数
     */
    uint32_t getTotalSamples() const;

    /**
     * @brief 获取原始音频数据
     * @return 原始音频数据
     */
    const std::vector<uint8_t>& getSamples() const;

    /**
     * @brief 获取压缩音频数据
     * @return 压缩音频数据
     */
    const std::vector<uint8_t>& getCompressedData() const;

    /**
     * @brief 是否已压缩
     * @return 是否已压缩
     */
    bool isCompressed() const;

    /**
     * @brief 解压缩音频数据
     * @return 是否成功解压缩
     */
    bool decompress();

    /**
     * @brief 压缩音频数据
     * @param quality 压缩质量（0-1）
     * @return 是否成功压缩
     */
    bool compress(float quality = 0.8f);

    /**
     * @brief 转换音频格式
     * @param targetFormat 目标格式
     * @return 是否成功转换
     */
    bool convertFormat(AudioFormat targetFormat);

    /**
     * @brief 调整音量
     * @param volume 音量倍数
     * @return 是否成功调整
     */
    bool adjustVolume(float volume);

    /**
     * @brief 调整播放速度
     * @param speed 速度倍数
     * @return 是否成功调整
     */
    bool adjustSpeed(float speed);

    /**
     * @brief 获取音频统计信息
     * @return 统计信息字符串
     */
    std::string getStats() const;

private:
    std::string id_;                              ///< 资源ID
    std::filesystem::path path_;                  ///< 资源路径
    ResourceState state_;                         ///< 资源状态
    std::filesystem::file_time_type lastModified_; ///< 最后修改时间
    std::uintmax_t size_;                         ///< 资源大小

    AudioData data_;                              ///< 音频数据
    AudioType audioType_;                         ///< 音频类型

    /**
     * @brief 加载WAV格式音频
     * @return 是否成功加载
     */
    bool loadWAV();

    /**
     * @brief 加载MP3格式音频
     * @return 是否成功加载
     */
    bool loadMP3();

    /**
     * @brief 加载OGG格式音频
     * @return 是否成功加载
     */
    bool loadOGG();

    /**
     * @brief 加载FLAC格式音频
     * @return 是否成功加载
     */
    bool loadFLAC();

    /**
     * @brief 检测音频格式
     * @return 音频格式
     */
    AudioFormat detectFormat() const;

    /**
     * @brief 计算音频统计信息
     */
    void calculateStats();
};

/**
 * @brief 音频资源加载器
 */
class AudioResourceLoader : public IResourceLoader {
public:
    /**
     * @brief 构造函数
     * @param supportedFormats 支持的音频格式列表
     */
    explicit AudioResourceLoader(const std::vector<AudioFormat>& supportedFormats = {});

    bool supportsType(ResourceType type) const override;
    std::shared_ptr<IResource> load(const std::filesystem::path& path,
                                  ResourceType type) override;
    void loadAsync(const std::filesystem::path& path,
                  ResourceType type,
                  std::function<void(std::shared_ptr<IResource>)> callback) override;

private:
    std::vector<AudioFormat> supportedFormats_; ///< 支持的音频格式
};

} // namespace Fantasy 