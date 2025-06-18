/**
 * @file ScriptResource.cpp
 * @brief 脚本资源实现
 * @author [pengchengkang]
 * @date 2025.06.18
 */

#include "utils/resources/ScriptResource.h"
#include "utils/LogSys/Logger.h"
#include <fstream>
#include <sstream>

using namespace Fantasy;

ScriptResource::ScriptResource(const std::filesystem::path& path)
    : path_(path), state_(ResourceState::UNLOADED), size_(0) {
    
    // 生成资源ID
    resourceId_ = "script_" + path_.filename().string();
    
    // 获取文件信息
    if (std::filesystem::exists(path_)) {
        size_ = std::filesystem::file_size(path_);
        lastModifiedTime_ = std::filesystem::last_write_time(path_);
    }
}

const std::string& ScriptResource::getId() const {
    return resourceId_;
}

ResourceType ScriptResource::getType() const {
    return ResourceType::SCRIPT;
}

ResourceState ScriptResource::getState() const {
    return state_;
}

const std::filesystem::path& ScriptResource::getPath() const {
    return path_;
}

std::filesystem::file_time_type ScriptResource::getLastModifiedTime() const {
    return lastModifiedTime_;
}

std::uintmax_t ScriptResource::getSize() const {
    return size_;
}

bool ScriptResource::load() {
    if (state_ == ResourceState::LOADED) {
        FANTASY_LOG_DEBUG("脚本资源已加载: {}", path_.string());
        return true;
    }
    
    if (!std::filesystem::exists(path_)) {
        FANTASY_LOG_ERROR("脚本文件不存在: {}", path_.string());
        state_ = ResourceState::ERROR;
        return false;
    }
    
    try {
        FANTASY_LOG_DEBUG("加载脚本资源: {}", path_.string());
        
        // 读取脚本文件内容
        std::ifstream file(path_);
        if (!file.is_open()) {
            FANTASY_LOG_ERROR("无法打开脚本文件: {}", path_.string());
            state_ = ResourceState::ERROR;
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        scriptContent_ = buffer.str();
        
        // TODO: 实现脚本解析和编译
        // 目前只是简单读取文件内容
        FANTASY_LOG_DEBUG("脚本资源加载成功: {} (大小: {} bytes)", 
                         path_.string(), scriptContent_.size());
        
        state_ = ResourceState::LOADED;
        return true;
        
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("脚本资源加载失败: {} - {}", path_.string(), e.what());
        state_ = ResourceState::ERROR;
        return false;
    }
}

void ScriptResource::unload() {
    if (state_ == ResourceState::UNLOADED) {
        return;
    }
    
    FANTASY_LOG_DEBUG("卸载脚本资源: {}", path_.string());
    
    // 清理脚本内容
    scriptContent_.clear();
    
    // TODO: 清理脚本引擎相关资源
    // 如果有脚本引擎实例，需要清理
    
    state_ = ResourceState::UNLOADED;
    FANTASY_LOG_DEBUG("脚本资源卸载完成: {}", path_.string());
}

bool ScriptResource::reload() {
    FANTASY_LOG_DEBUG("重新加载脚本资源: {}", path_.string());
    
    unload();
    return load();
}

bool ScriptResource::needsReload() const {
    if (state_ != ResourceState::LOADED) {
        return false;
    }
    
    if (!std::filesystem::exists(path_)) {
        return false;
    }
    
    auto currentTime = std::filesystem::last_write_time(path_);
    return currentTime > lastModifiedTime_;
}

const std::string& ScriptResource::getScriptContent() const {
    return scriptContent_;
}

// TODO: 实现脚本执行功能
// bool ScriptResource::execute(const std::string& functionName, const std::vector<std::string>& args) {
//     // 实现脚本执行逻辑
//     return false;
// }
