/**
 * @file ResourceType.h
 * @brief 资源类型定义和基础资源接口
 * @author [pengchengkang]
 * @date 2025.06.16
 */

#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <filesystem>
#include <functional>

namespace Fantasy {

/**
 * @brief 资源类型枚举
 */
enum class ResourceType {
    CONFIG,     ///< 配置文件资源
    MAP,        ///< 地图资源
    SCRIPT,     ///< 脚本资源
    TEXTURE,    ///< 纹理资源
    MODEL,      ///< 模型资源
    AUDIO,      ///< 音频资源
    SOUND,      ///< 音效资源
    MUSIC,      ///< 音乐资源
    FONT,       ///< 字体资源
    SHADER,     ///< 着色器资源
    ANIMATION,  ///< 动画资源
    UNKNOWN     ///< 未知资源类型
};

/**
 * @brief 资源状态枚举
 */
enum class ResourceState {
    UNLOADED,   ///< 未加载
    LOADING,    ///< 加载中
    LOADED,     ///< 已加载
    ERROR       ///< 加载错误
};

/**
 * @brief 基础资源接口
 */
class IResource {
public:
    virtual ~IResource() = default;

    /**
     * @brief 获取资源ID
     * @return 资源ID
     */
    virtual const std::string& getId() const = 0;

    /**
     * @brief 获取资源类型
     * @return 资源类型
     */
    virtual ResourceType getType() const = 0;

    /**
     * @brief 获取资源状态
     * @return 资源状态
     */
    virtual ResourceState getState() const = 0;

    /**
     * @brief 获取资源路径
     * @return 资源文件路径
     */
    virtual const std::filesystem::path& getPath() const = 0;

    /**
     * @brief 获取最后修改时间
     * @return 最后修改时间
     */
    virtual std::filesystem::file_time_type getLastModifiedTime() const = 0;

    /**
     * @brief 获取资源大小
     * @return 资源大小（字节）
     */
    virtual std::uintmax_t getSize() const = 0;

    /**
     * @brief 加载资源
     * @return 是否加载成功
     */
    virtual bool load() = 0;

    /**
     * @brief 卸载资源
     */
    virtual void unload() = 0;

    /**
     * @brief 重新加载资源
     * @return 是否重新加载成功
     */
    virtual bool reload() = 0;

    /**
     * @brief 检查资源是否需要重新加载
     * @return 是否需要重新加载
     */
    virtual bool needsReload() const = 0;
};

/**
 * @brief 资源加载器接口
 */
class IResourceLoader {
public:
    virtual ~IResourceLoader() = default;

    /**
     * @brief 检查是否支持加载指定类型的资源
     * @param type 资源类型
     * @return 是否支持
     */
    virtual bool supportsType(ResourceType type) const = 0;

    /**
     * @brief 加载资源
     * @param path 资源路径
     * @param type 资源类型
     * @return 加载的资源对象
     */
    virtual std::shared_ptr<IResource> load(const std::filesystem::path& path, 
                                          ResourceType type) = 0;

    /**
     * @brief 异步加载资源
     * @param path 资源路径
     * @param type 资源类型
     * @param callback 加载完成回调
     */
    virtual void loadAsync(const std::filesystem::path& path,
                         ResourceType type,
                         std::function<void(std::shared_ptr<IResource>)> callback) = 0;
};

} // namespace Fantasy
