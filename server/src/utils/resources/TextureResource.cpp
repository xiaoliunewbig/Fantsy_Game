/**
 * @file TextureResource.cpp
 * @brief 纹理资源实现
 * @author [pengchengkang]
 * @date 2025.06.18
 */

#include "utils/resources/TextureResource.h"
#include "utils/LogSys/Logger.h"
#include <fstream>
#include <sstream>

using namespace Fantasy;

TextureResource::TextureResource(const std::filesystem::path& path)
    : path_(path), state_(ResourceState::UNLOADED), size_(0) {
    
    // 生成资源ID
    resourceId_ = "texture_" + path_.filename().string();
    
    // 获取文件信息
    if (std::filesystem::exists(path_)) {
        size_ = std::filesystem::file_size(path_);
        lastModifiedTime_ = std::filesystem::last_write_time(path_);
    }
}

const std::string& TextureResource::getId() const {
    return resourceId_;
}

ResourceType TextureResource::getType() const {
    return ResourceType::TEXTURE;
}

ResourceState TextureResource::getState() const {
    return state_;
}

const std::filesystem::path& TextureResource::getPath() const {
    return path_;
}

std::filesystem::file_time_type TextureResource::getLastModifiedTime() const {
    return lastModifiedTime_;
}

std::uintmax_t TextureResource::getSize() const {
    return size_;
}

bool TextureResource::load() {
    if (state_ == ResourceState::LOADED) {
        FANTASY_LOG_DEBUG("纹理资源已加载: {}", path_.string());
        return true;
    }
    
    if (!std::filesystem::exists(path_)) {
        FANTASY_LOG_ERROR("纹理文件不存在: {}", path_.string());
        state_ = ResourceState::ERROR;
        return false;
    }
    
    try {
        FANTASY_LOG_DEBUG("加载纹理资源: {}", path_.string());
        
        // 读取纹理文件内容
        std::ifstream file(path_, std::ios::binary);
        if (!file.is_open()) {
            FANTASY_LOG_ERROR("无法打开纹理文件: {}", path_.string());
            state_ = ResourceState::ERROR;
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        textureData_ = buffer.str();
        
        // TODO: 实现纹理解析和加载
        // 目前只是简单读取文件内容
        FANTASY_LOG_DEBUG("纹理资源加载成功: {} (大小: {} bytes)", 
                         path_.string(), textureData_.size());
        
        state_ = ResourceState::LOADED;
        return true;
        
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("纹理资源加载失败: {} - {}", path_.string(), e.what());
        state_ = ResourceState::ERROR;
        return false;
    }
}

void TextureResource::unload() {
    if (state_ == ResourceState::UNLOADED) {
        return;
    }
    
    FANTASY_LOG_DEBUG("卸载纹理资源: {}", path_.string());
    
    // 清理纹理数据
    textureData_.clear();
    
    // TODO: 清理GPU纹理资源
    // 如果有GPU纹理对象，需要清理
    
    state_ = ResourceState::UNLOADED;
    FANTASY_LOG_DEBUG("纹理资源卸载完成: {}", path_.string());
}

bool TextureResource::reload() {
    FANTASY_LOG_DEBUG("重新加载纹理资源: {}", path_.string());
    
    unload();
    return load();
}

bool TextureResource::needsReload() const {
    if (state_ != ResourceState::LOADED) {
        return false;
    }
    
    if (!std::filesystem::exists(path_)) {
        return false;
    }
    
    auto currentTime = std::filesystem::last_write_time(path_);
    return currentTime > lastModifiedTime_;
}

const std::string& TextureResource::getTextureData() const {
    return textureData_;
}

// TODO: 实现纹理属性获取功能
// uint32_t TextureResource::getWidth() const { return 0; }
// uint32_t TextureResource::getHeight() const { return 0; }
// TextureFormat TextureResource::getFormat() const { return TextureFormat::UNKNOWN; }

// TextureResourceLoader实现
// TextureResourceLoader::TextureResourceLoader() = default;

bool TextureResourceLoader::supportsType(ResourceType type) const {
    return type == ResourceType::TEXTURE;
}

std::shared_ptr<IResource> TextureResourceLoader::load(
    const std::filesystem::path& path,
    ResourceType type) {
    if (!supportsType(type)) {
        return nullptr;
    }

    auto resource = std::make_shared<TextureResource>(path);
    if (!resource->load()) {
        return nullptr;
    }
    return resource;
}

void TextureResourceLoader::loadAsync(
    const std::filesystem::path& path,
    ResourceType type,
    std::function<void(std::shared_ptr<IResource>)> callback) {
    if (!supportsType(type)) {
        callback(nullptr);
        return;
    }

    // 在后台线程中加载资源
    std::thread([path, callback]() {
        auto resource = std::make_shared<TextureResource>(path);
        if (resource->load()) {
            callback(resource);
        } else {
            callback(nullptr);
        }
    }).detach();
} 