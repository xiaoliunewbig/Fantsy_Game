/**
 * @file TextureResource.cpp
 * @brief 纹理资源类实现
 * @author [pengchengkang]
 * @date 2025.06.16
 */

#include "utils/resources/TextureResource.h"
#include "include/utils/LogSys/Logger.h"
#include <stb_image.h>
#include <stb_image_write.h>
#include <stb_image_resize.h>
#include <algorithm>
#include <cmath>

namespace Fantasy {

TextureResource::TextureResource(const std::filesystem::path& path)
    : path_(path)
    , state_(ResourceState::UNLOADED)
    , size_(0)
    , filter_(TextureFilter::LINEAR)
    , wrap_(TextureWrap::REPEAT) {
    id_ = path.filename().string();
    try {
        lastModified_ = std::filesystem::last_write_time(path);
        size_ = std::filesystem::file_size(path);
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Failed to get file info for texture {}: {}", path.string(), e.what());
    }
}

const std::string& TextureResource::getId() const {
    return id_;
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
    return lastModified_;
}

std::uintmax_t TextureResource::getSize() const {
    return size_;
}

bool TextureResource::load() {
    if (state_ == ResourceState::LOADED) {
        return true;
    }

    try {
        state_ = ResourceState::LOADING;

        // 使用stb_image加载纹理
        int width, height, channels;
        unsigned char* pixels = stbi_load(path_.string().c_str(), &width, &height, &channels, 0);
        if (!pixels) {
            throw std::runtime_error("Failed to load texture: " + std::string(stbi_failure_reason()));
        }

        // 设置纹理数据
        data_.width = static_cast<uint32_t>(width);
        data_.height = static_cast<uint32_t>(height);
        data_.channels = static_cast<uint32_t>(channels);
        data_.format = channels == 3 ? TextureFormat::RGB : TextureFormat::RGBA;
        data_.hasMipmaps = false;
        data_.pixels.assign(pixels, pixels + width * height * channels);

        // 释放stb_image分配的内存
        stbi_image_free(pixels);

        // 生成mipmap
        if (!generateMipmaps()) {
            FANTASY_LOG_WARN("Failed to generate mipmaps for texture: {}", path_.string());
        }

        state_ = ResourceState::LOADED;
        FANTASY_LOG_INFO("Successfully loaded texture: {}", path_.string());
        return true;
    } catch (const std::exception& e) {
        state_ = ResourceState::ERROR;
        FANTASY_LOG_ERROR("Failed to load texture {}: {}", path_.string(), e.what());
        return false;
    }
}

void TextureResource::unload() {
    if (state_ != ResourceState::LOADED) {
        return;
    }

    data_.pixels.clear();
    data_.mipmaps.clear();
    data_.hasMipmaps = false;
    state_ = ResourceState::UNLOADED;

    FANTASY_LOG_INFO("Successfully unloaded texture: {}", path_.string());
}

bool TextureResource::reload() {
    unload();
    return load();
}

bool TextureResource::needsReload() const {
    try {
        return std::filesystem::last_write_time(path_) > lastModified_;
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Failed to check if texture needs reload: {}", e.what());
        return false;
    }
}

const TextureResource::TextureData& TextureResource::getData() const {
    return data_;
}

uint32_t TextureResource::getWidth() const {
    return data_.width;
}

uint32_t TextureResource::getHeight() const {
    return data_.height;
}

TextureFormat TextureResource::getFormat() const {
    return data_.format;
}

uint32_t TextureResource::getChannels() const {
    return data_.channels;
}

bool TextureResource::hasMipmaps() const {
    return data_.hasMipmaps;
}

const std::vector<uint8_t>& TextureResource::getMipmap(uint32_t level) const {
    if (level >= data_.mipmaps.size()) {
        throw std::out_of_range("Invalid mipmap level");
    }
    return data_.mipmaps[level];
}

uint32_t TextureResource::getMipmapCount() const {
    return static_cast<uint32_t>(data_.mipmaps.size());
}

void TextureResource::setFilter(TextureFilter filter) {
    filter_ = filter;
}

void TextureResource::setWrap(TextureWrap wrap) {
    wrap_ = wrap;
}

TextureFilter TextureResource::getFilter() const {
    return filter_;
}

TextureWrap TextureResource::getWrap() const {
    return wrap_;
}

bool TextureResource::generateMipmaps() {
    if (data_.pixels.empty()) {
        return false;
    }

    try {
        data_.mipmaps.clear();
        uint32_t width = data_.width;
        uint32_t height = data_.height;
        uint32_t channels = data_.channels;

        // 计算mipmap级别
        uint32_t levels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

        // 生成每一级mipmap
        for (uint32_t level = 1; level < levels; ++level) {
            uint32_t newWidth = std::max(1u, width >> level);
            uint32_t newHeight = std::max(1u, height >> level);
            std::vector<uint8_t> mipmap(newWidth * newHeight * channels);

            // 使用stb_image_resize进行缩放
            if (!stbir_resize_uint8(data_.pixels.data(), width, height, 0,
                                  mipmap.data(), newWidth, newHeight, 0,
                                  channels)) {
                throw std::runtime_error("Failed to generate mipmap level " + std::to_string(level));
            }

            data_.mipmaps.push_back(std::move(mipmap));
        }

        data_.hasMipmaps = true;
        return true;
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Failed to generate mipmaps: {}", e.what());
        return false;
    }
}

bool TextureResource::compress(TextureFormat format) {
    // TODO: 实现纹理压缩
    // 这需要根据不同的压缩格式使用相应的压缩库
    // 例如：使用libsquish进行DXT压缩，使用etcpack进行ETC压缩等
    FANTASY_LOG_WARN("Texture compression not implemented yet");
    return false;
}

// TextureResourceLoader实现
TextureResourceLoader::TextureResourceLoader(const std::vector<TextureFormat>& supportedFormats)
    : supportedFormats_(supportedFormats) {
    if (supportedFormats_.empty()) {
        // 默认支持所有格式
        supportedFormats_ = {
            TextureFormat::RGB,
            TextureFormat::RGBA,
            TextureFormat::DXT1,
            TextureFormat::DXT5,
            TextureFormat::ETC1,
            TextureFormat::ETC2,
            TextureFormat::ASTC
        };
    }
}

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

} // namespace Fantasy 