/**
 * @file ConfigResource.h
 * @brief 配置资源类
 * @author [pengchengkang]
 * @date 2025.06.16
 */

#pragma once

#include "ResourceType.h"
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <filesystem>

namespace Fantasy {

/**
 * @brief 配置资源类
 * @details 用于加载和管理JSON格式的配置文件
 */
class ConfigResource : public IResource {
public:
    /**
     * @brief 构造函数
     * @param path 配置文件路径
     */
    explicit ConfigResource(const std::filesystem::path& path);

    /**
     * @brief 获取资源ID
     * @return 资源ID
     */
    const std::string& getId() const override;

    /**
     * @brief 获取资源类型
     * @return 资源类型
     */
    ResourceType getType() const override;

    /**
     * @brief 获取资源状态
     * @return 资源状态
     */
    ResourceState getState() const override;

    /**
     * @brief 获取资源路径
     * @return 资源文件路径
     */
    const std::filesystem::path& getPath() const override;

    /**
     * @brief 获取最后修改时间
     * @return 最后修改时间
     */
    std::filesystem::file_time_type getLastModifiedTime() const override;

    /**
     * @brief 获取资源大小
     * @return 资源大小（字节）
     */
    std::uintmax_t getSize() const override;

    /**
     * @brief 加载资源
     * @return 是否加载成功
     */
    bool load() override;

    /**
     * @brief 卸载资源
     */
    void unload() override;

    /**
     * @brief 重新加载资源
     * @return 是否重新加载成功
     */
    bool reload() override;

    /**
     * @brief 检查资源是否需要重新加载
     * @return 是否需要重新加载
     */
    bool needsReload() const override;

    /**
     * @brief 获取配置数据
     * @return JSON配置数据
     */
    const nlohmann::json& getData() const;

    /**
     * @brief 获取配置值
     * @tparam T 值类型
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值
     */
    template<typename T>
    T getValue(const std::string& key, const T& defaultValue = T()) const {
        try {
            if (!data_.contains(key)) {
                return defaultValue;
            }
            return data_[key].get<T>();
        } catch (const std::exception&) {
            return defaultValue;
        }
    }

    /**
     * @brief 设置配置值
     * @tparam T 值类型
     * @param key 配置键
     * @param value 配置值
     */
    template<typename T>
    void setValue(const std::string& key, const T& value) {
        data_[key] = value;
    }

    /**
     * @brief 保存配置到文件
     * @return 是否保存成功
     */
    bool save();

private:
    std::string id_;                              ///< 资源ID
    std::filesystem::path path_;                  ///< 资源路径
    ResourceState state_;                         ///< 资源状态
    std::filesystem::file_time_type lastModified_; ///< 最后修改时间
    std::uintmax_t size_;                         ///< 资源大小
    nlohmann::json data_;                         ///< 配置数据
};

/**
 * @brief 配置资源加载器
 */
class ConfigResourceLoader : public IResourceLoader {
public:
    /**
     * @brief 检查是否支持加载指定类型的资源
     * @param type 资源类型
     * @return 是否支持
     */
    bool supportsType(ResourceType type) const override;

    /**
     * @brief 加载资源
     * @param path 资源路径
     * @param type 资源类型
     * @return 加载的资源对象
     */
    std::shared_ptr<IResource> load(const std::filesystem::path& path,
                                  ResourceType type) override;

    /**
     * @brief 异步加载资源
     * @param path 资源路径
     * @param type 资源类型
     * @param callback 加载完成回调
     */
    void loadAsync(const std::filesystem::path& path,
                  ResourceType type,
                  std::function<void(std::shared_ptr<IResource>)> callback) override;
};

} // namespace Fantasy
