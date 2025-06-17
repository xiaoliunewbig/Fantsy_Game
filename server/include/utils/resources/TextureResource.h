/**
 * @file TextureResource.h
 * @brief 纹理资源类
 * @author [pengchengkang]
 * @date 2025.06.16
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
 * @brief 纹理格式
 */
enum class TextureFormat {
    RGB,        ///< RGB格式
    RGBA,       ///< RGBA格式
    DXT1,       ///< DXT1压缩格式
    DXT5,       ///< DXT5压缩格式
    ETC1,       ///< ETC1压缩格式
    ETC2,       ///< ETC2压缩格式
    ASTC,       ///< ASTC压缩格式
    UNKNOWN     ///< 未知格式
};

/**
 * @brief 纹理过滤模式
 */
enum class TextureFilter {
    NEAREST,    ///< 最近点采样
    LINEAR,     ///< 线性过滤
    BILINEAR,   ///< 双线性过滤
    TRILINEAR,  ///< 三线性过滤
    ANISOTROPIC ///< 各向异性过滤
};

/**
 * @brief 纹理包装模式
 */
enum class TextureWrap {
    REPEAT,     ///< 重复
    MIRROR,     ///< 镜像重复
    CLAMP,      ///< 钳制
    BORDER      ///< 边界颜色
};

/**
 * @brief 纹理资源类
 * @details 用于加载和管理纹理资源
 */
class TextureResource : public IResource {
public:
    /**
     * @brief 纹理数据
     */
    struct TextureData {
        std::vector<uint8_t> pixels;     ///< 像素数据
        uint32_t width;                  ///< 宽度
        uint32_t height;                 ///< 高度
        uint32_t channels;               ///< 通道数
        TextureFormat format;            ///< 纹理格式
        bool hasMipmaps;                 ///< 是否有mipmap
        std::vector<std::vector<uint8_t>> mipmaps; ///< mipmap数据
    };

    /**
     * @brief 构造函数
     * @param path 纹理文件路径
     */
    explicit TextureResource(const std::filesystem::path& path);

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
     * @brief 获取纹理数据
     * @return 纹理数据
     */
    const TextureData& getData() const;

    /**
     * @brief 获取纹理宽度
     * @return 宽度（像素）
     */
    uint32_t getWidth() const;

    /**
     * @brief 获取纹理高度
     * @return 高度（像素）
     */
    uint32_t getHeight() const;

    /**
     * @brief 获取纹理格式
     * @return 纹理格式
     */
    TextureFormat getFormat() const;

    /**
     * @brief 获取通道数
     * @return 通道数
     */
    uint32_t getChannels() const;

    /**
     * @brief 是否有mipmap
     * @return 是否有mipmap
     */
    bool hasMipmaps() const;

    /**
     * @brief 获取mipmap数据
     * @param level mipmap级别
     * @return mipmap数据
     */
    const std::vector<uint8_t>& getMipmap(uint32_t level) const;

    /**
     * @brief 获取mipmap数量
     * @return mipmap数量
     */
    uint32_t getMipmapCount() const;

    /**
     * @brief 设置过滤模式
     * @param filter 过滤模式
     */
    void setFilter(TextureFilter filter);

    /**
     * @brief 设置包装模式
     * @param wrap 包装模式
     */
    void setWrap(TextureWrap wrap);

    /**
     * @brief 获取过滤模式
     * @return 过滤模式
     */
    TextureFilter getFilter() const;

    /**
     * @brief 获取包装模式
     * @return 包装模式
     */
    TextureWrap getWrap() const;

    /**
     * @brief 生成mipmap
     * @return 是否成功生成
     */
    bool generateMipmaps();

    /**
     * @brief 压缩纹理
     * @param format 目标格式
     * @return 是否成功压缩
     */
    bool compress(TextureFormat format);

private:
    std::string id_;                              ///< 资源ID
    std::filesystem::path path_;                  ///< 资源路径
    ResourceState state_;                         ///< 资源状态
    std::filesystem::file_time_type lastModified_; ///< 最后修改时间
    std::uintmax_t size_;                         ///< 资源大小

    TextureData data_;                            ///< 纹理数据
    TextureFilter filter_;                        ///< 过滤模式
    TextureWrap wrap_;                            ///< 包装模式
};

/**
 * @brief 纹理资源加载器
 */
class TextureResourceLoader : public IResourceLoader {
public:
    /**
     * @brief 构造函数
     * @param supportedFormats 支持的纹理格式列表
     */
    explicit TextureResourceLoader(const std::vector<TextureFormat>& supportedFormats = {});

    bool supportsType(ResourceType type) const override;
    std::shared_ptr<IResource> load(const std::filesystem::path& path,
                                  ResourceType type) override;
    void loadAsync(const std::filesystem::path& path,
                  ResourceType type,
                  std::function<void(std::shared_ptr<IResource>)> callback) override;

private:
    std::vector<TextureFormat> supportedFormats_; ///< 支持的纹理格式
};

} // namespace Fantasy 