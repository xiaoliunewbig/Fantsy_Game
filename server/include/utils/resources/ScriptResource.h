/**
 * @file ScriptResource.h
 * @brief 脚本资源类
 * @author [pengchengkang]
 * @date 2025.06.16
 */

#pragma once

#include "ResourceType.h"
#include <string>
#include <memory>
#include <filesystem>
#include <functional>
#include <unordered_map>
#include <pybind11/pybind11.h>

namespace Fantasy {

namespace py = pybind11;

/**
 * @brief 脚本资源类
 * @details 用于加载和管理Python脚本资源
 */
class ScriptResource : public IResource {
public:
    /**
     * @brief 脚本类型
     */
    enum class ScriptType {
        QUEST,      ///< 任务脚本
        DIALOGUE,   ///< 对话脚本
        EVENT,      ///< 事件脚本
        AI,         ///< AI脚本
        SYSTEM,     ///< 系统脚本
        CUSTOM      ///< 自定义脚本
    };

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
     * @brief 获取脚本类型
     * @return 脚本类型
     */
    ScriptType getScriptType() const;

    /**
     * @brief 获取脚本模块
     * @return Python模块对象
     */
    py::module_ getModule() const;

    /**
     * @brief 调用脚本函数
     * @tparam R 返回值类型
     * @tparam Args 参数类型
     * @param functionName 函数名
     * @param args 函数参数
     * @return 函数返回值
     */
    template<typename R = void, typename... Args>
    R callFunction(const std::string& functionName, Args&&... args) const {
        try {
            if (!module_) {
                throw std::runtime_error("Script module not loaded");
            }
            return module_.attr(functionName.c_str())(std::forward<Args>(args)...).cast<R>();
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to call script function: " + std::string(e.what()));
        }
    }

    /**
     * @brief 获取脚本变量
     * @tparam T 变量类型
     * @param name 变量名
     * @return 变量值
     */
    template<typename T>
    T getVariable(const std::string& name) const {
        try {
            if (!module_) {
                throw std::runtime_error("Script module not loaded");
            }
            return module_.attr(name.c_str()).cast<T>();
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to get script variable: " + std::string(e.what()));
        }
    }

    /**
     * @brief 设置脚本变量
     * @tparam T 变量类型
     * @param name 变量名
     * @param value 变量值
     */
    template<typename T>
    void setVariable(const std::string& name, const T& value) {
        try {
            if (!module_) {
                throw std::runtime_error("Script module not loaded");
            }
            module_.attr(name.c_str()) = value;
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to set script variable: " + std::string(e.what()));
        }
    }

    /**
     * @brief 检查脚本是否包含指定函数
     * @param functionName 函数名
     * @return 是否包含
     */
    bool hasFunction(const std::string& functionName) const;

    /**
     * @brief 检查脚本是否包含指定变量
     * @param name 变量名
     * @return 是否包含
     */
    bool hasVariable(const std::string& name) const;

    /**
     * @brief 获取脚本错误信息
     * @return 错误信息
     */
    const std::string& getError() const;

private:
    std::string id_;                              ///< 资源ID
    std::filesystem::path path_;                  ///< 资源路径
    ResourceState state_;                         ///< 资源状态
    std::filesystem::file_time_type lastModified_; ///< 最后修改时间
    std::uintmax_t size_;                         ///< 资源大小

    ScriptType scriptType_;                       ///< 脚本类型
    py::module_ module_;                          ///< Python模块
    std::string error_;                           ///< 错误信息
};

/**
 * @brief 脚本资源加载器
 */
class ScriptResourceLoader : public IResourceLoader {
public:
    /**
     * @brief 构造函数
     * @param pythonPath Python模块搜索路径
     */
    explicit ScriptResourceLoader(const std::filesystem::path& pythonPath = "");

    bool supportsType(ResourceType type) const override;
    std::shared_ptr<IResource> load(const std::filesystem::path& path,
                                  ResourceType type) override;
    void loadAsync(const std::filesystem::path& path,
                  ResourceType type,
                  std::function<void(std::shared_ptr<IResource>)> callback) override;

private:
    std::filesystem::path pythonPath_;            ///< Python模块搜索路径
};

} // namespace Fantasy
