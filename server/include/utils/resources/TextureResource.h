/**
 * @file TextureResource.h
 * @brief 纹理资源类
 * @author [pengchengkang]
 * @date 2025.06.18
 */

#pragma once

#include "ResourceType.h"
#include <string>
#include <memory>
#include <filesystem>
#include <functional>

namespace Fantasy {

/**
 * @brief 纹理资源类
 * @details 用于加载和管理纹理资源
 */
class TextureResource : public IResource {
public:
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
     * @return 纹理文件内容
     */
    const std::string& getTextureData() const;

    // TODO: 实现纹理属性获取功能
    // uint32_t getWidth() const;
    // uint32_t getHeight() const;
    // TextureFormat getFormat() const;

private:
    std::string resourceId_;                      ///< 资源ID
    std::filesystem::path path_;                  ///< 资源路径
    ResourceState state_;                         ///< 资源状态
    std::filesystem::file_time_type lastModifiedTime_; ///< 最后修改时间
    std::uintmax_t size_;                         ///< 资源大小
    std::string textureData_;                     ///< 纹理数据
};

/**
 * @brief 纹理资源加载器
 */
class TextureResourceLoader : public IResourceLoader {
public:
    /**
     * @brief 构造函数
     */
    TextureResourceLoader() = default;

    bool supportsType(ResourceType type) const override;
    std::shared_ptr<IResource> load(const std::filesystem::path& path,
                                  ResourceType type) override;
    void loadAsync(const std::filesystem::path& path,
                  ResourceType type,
                  std::function<void(std::shared_ptr<IResource>)> callback) override;
};

} // namespace Fantasy 