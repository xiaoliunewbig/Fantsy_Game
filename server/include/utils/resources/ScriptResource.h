/**
 * @file ScriptResource.h
 * @brief 脚本资源类
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
 * @brief 脚本资源类
 * @details 用于加载和管理脚本资源（支持多种脚本语言）
 */
class ScriptResource : public IResource {
public:
    /**
     * @brief 构造函数
     * @param path 脚本文件路径
     */
    explicit ScriptResource(const std::filesystem::path& path);

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
     * @brief 获取脚本内容
     * @return 脚本文件内容
     */
    const std::string& getScriptContent() const;

    // TODO: 实现脚本执行功能
    // bool execute(const std::string& functionName, const std::vector<std::string>& args);

private:
    std::string resourceId_;                      ///< 资源ID
    std::filesystem::path path_;                  ///< 资源路径
    ResourceState state_;                         ///< 资源状态
    std::filesystem::file_time_type lastModifiedTime_; ///< 最后修改时间
    std::uintmax_t size_;                         ///< 资源大小
    std::string scriptContent_;                   ///< 脚本内容
};

/**
 * @brief 脚本资源加载器
 */
class ScriptResourceLoader : public IResourceLoader {
public:
    /**
     * @brief 构造函数
     */
    ScriptResourceLoader() = default;

    bool supportsType(ResourceType type) const override;
    std::shared_ptr<IResource> load(const std::filesystem::path& path,
                                  ResourceType type) override;
    void loadAsync(const std::filesystem::path& path,
                  ResourceType type,
                  std::function<void(std::shared_ptr<IResource>)> callback) override;
};

} // namespace Fantasy
