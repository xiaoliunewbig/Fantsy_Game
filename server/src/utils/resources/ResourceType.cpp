/**
 * @file ResourceType.cpp
 * @brief 资源类型辅助函数实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "include/utils/resources/ResourceType.h"
#include <algorithm>
#include <cstring>

namespace Fantasy {

std::string getResourceTypeString(ResourceType type) {
    switch (type) {
        case ResourceType::CONFIG: return "CONFIG";
        case ResourceType::MAP: return "MAP";
        case ResourceType::SCRIPT: return "SCRIPT";
        case ResourceType::TEXTURE: return "TEXTURE";
        case ResourceType::MODEL: return "MODEL";
        case ResourceType::SOUND: return "SOUND";
        case ResourceType::MUSIC: return "MUSIC";
        case ResourceType::FONT: return "FONT";
        case ResourceType::SHADER: return "SHADER";
        case ResourceType::ANIMATION: return "ANIMATION";
        case ResourceType::UNKNOWN: return "UNKNOWN";
        default: return "UNKNOWN";
    }
}

ResourceType getResourceTypeFromString(const std::string& typeStr) {
    std::string upperType = typeStr;
    std::transform(upperType.begin(), upperType.end(), upperType.begin(), ::toupper);
    
    if (upperType == "CONFIG") return ResourceType::CONFIG;
    if (upperType == "MAP") return ResourceType::MAP;
    if (upperType == "SCRIPT") return ResourceType::SCRIPT;
    if (upperType == "TEXTURE") return ResourceType::TEXTURE;
    if (upperType == "MODEL") return ResourceType::MODEL;
    if (upperType == "SOUND") return ResourceType::SOUND;
    if (upperType == "MUSIC") return ResourceType::MUSIC;
    if (upperType == "FONT") return ResourceType::FONT;
    if (upperType == "SHADER") return ResourceType::SHADER;
    if (upperType == "ANIMATION") return ResourceType::ANIMATION;
    
    return ResourceType::UNKNOWN;
}

ResourceType getResourceTypeFromExtension(const std::string& extension) {
    std::string lowerExt = extension;
    std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
    
    // 配置文件
    if (lowerExt == ".json" || lowerExt == ".xml" || lowerExt == ".yaml" || 
        lowerExt == ".yml" || lowerExt == ".ini" || lowerExt == ".toml" || 
        lowerExt == ".cfg" || lowerExt == ".conf") {
        return ResourceType::CONFIG;
    }
    
    // 地图文件
    if (lowerExt == ".tmx" || lowerExt == ".map" || lowerExt == ".bin") {
        return ResourceType::MAP;
    }
    
    // 脚本文件
    if (lowerExt == ".lua" || lowerExt == ".js" || lowerExt == ".py" || 
        lowerExt == ".cpp" || lowerExt == ".h" || lowerExt == ".cs") {
        return ResourceType::SCRIPT;
    }
    
    // 纹理文件
    if (lowerExt == ".png" || lowerExt == ".jpg" || lowerExt == ".jpeg" || 
        lowerExt == ".bmp" || lowerExt == ".tga" || lowerExt == ".dds" || 
        lowerExt == ".ktx" || lowerExt == ".pvr") {
        return ResourceType::TEXTURE;
    }
    
    // 模型文件
    if (lowerExt == ".obj" || lowerExt == ".fbx" || lowerExt == ".dae" || 
        lowerExt == ".3ds" || lowerExt == ".max" || lowerExt == ".blend" || 
        lowerExt == ".ply" || lowerExt == ".stl") {
        return ResourceType::MODEL;
    }
    
    // 音频文件
    if (lowerExt == ".wav" || lowerExt == ".mp3" || lowerExt == ".ogg" || 
        lowerExt == ".flac" || lowerExt == ".aac" || lowerExt == ".m4a") {
        return ResourceType::SOUND;
    }
    
    // 音乐文件
    if (lowerExt == ".mid" || lowerExt == ".midi" || lowerExt == ".mod" || 
        lowerExt == ".s3m" || lowerExt == ".xm" || lowerExt == ".it") {
        return ResourceType::MUSIC;
    }
    
    // 字体文件
    if (lowerExt == ".ttf" || lowerExt == ".otf" || lowerExt == ".woff" || 
        lowerExt == ".woff2" || lowerExt == ".eot" || lowerExt == ".fnt") {
        return ResourceType::FONT;
    }
    
    // 着色器文件
    if (lowerExt == ".vert" || lowerExt == ".frag" || lowerExt == ".geom" || 
        lowerExt == ".tesc" || lowerExt == ".tese" || lowerExt == ".comp" || 
        lowerExt == ".glsl" || lowerExt == ".hlsl") {
        return ResourceType::SHADER;
    }
    
    // 动画文件
    if (lowerExt == ".anim" || lowerExt == ".bvh" || lowerExt == ".fbx" || 
        lowerExt == ".dae" || lowerExt == ".c3d" || lowerExt == ".htr") {
        return ResourceType::ANIMATION;
    }
    
    return ResourceType::UNKNOWN;
}

ResourceType getResourceTypeFromPath(const std::filesystem::path& path) {
    return getResourceTypeFromExtension(path.extension().string());
}

bool isCompressibleType(ResourceType type) {
    switch (type) {
        case ResourceType::CONFIG:
        case ResourceType::MAP:
        case ResourceType::SCRIPT:
        case ResourceType::TEXTURE:
        case ResourceType::MODEL:
        case ResourceType::SOUND:
        case ResourceType::MUSIC:
        case ResourceType::FONT:
        case ResourceType::SHADER:
        case ResourceType::ANIMATION:
            return true;
        default:
            return false;
    }
}

bool isCacheableType(ResourceType type) {
    switch (type) {
        case ResourceType::CONFIG:
        case ResourceType::MAP:
        case ResourceType::SCRIPT:
        case ResourceType::TEXTURE:
        case ResourceType::MODEL:
        case ResourceType::FONT:
        case ResourceType::SHADER:
        case ResourceType::ANIMATION:
            return true;
        case ResourceType::SOUND:
        case ResourceType::MUSIC:
            return false; // 音频通常不缓存，直接流式播放
        default:
            return false;
    }
}

bool isPreloadableType(ResourceType type) {
    switch (type) {
        case ResourceType::CONFIG:
        case ResourceType::MAP:
        case ResourceType::SCRIPT:
        case ResourceType::TEXTURE:
        case ResourceType::MODEL:
        case ResourceType::FONT:
        case ResourceType::SHADER:
        case ResourceType::ANIMATION:
            return true;
        case ResourceType::SOUND:
        case ResourceType::MUSIC:
            return false; // 音频通常不预加载
        default:
            return false;
    }
}

size_t getDefaultCacheSize(ResourceType type) {
    switch (type) {
        case ResourceType::CONFIG:
            return 1 * 1024 * 1024; // 1MB
        case ResourceType::MAP:
            return 10 * 1024 * 1024; // 10MB
        case ResourceType::SCRIPT:
            return 5 * 1024 * 1024; // 5MB
        case ResourceType::TEXTURE:
            return 50 * 1024 * 1024; // 50MB
        case ResourceType::MODEL:
            return 20 * 1024 * 1024; // 20MB
        case ResourceType::SOUND:
            return 0; // 不缓存
        case ResourceType::MUSIC:
            return 0; // 不缓存
        case ResourceType::FONT:
            return 10 * 1024 * 1024; // 10MB
        case ResourceType::SHADER:
            return 2 * 1024 * 1024; // 2MB
        case ResourceType::ANIMATION:
            return 5 * 1024 * 1024; // 5MB
        default:
            return 1 * 1024 * 1024; // 1MB
    }
}

int getDefaultCompressionLevel(ResourceType type) {
    switch (type) {
        case ResourceType::CONFIG:
        case ResourceType::MAP:
        case ResourceType::SCRIPT:
            return 9; // 最高压缩比
        case ResourceType::TEXTURE:
            return 6; // 中等压缩比
        case ResourceType::MODEL:
            return 7; // 较高压缩比
        case ResourceType::SOUND:
        case ResourceType::MUSIC:
            return 5; // 较低压缩比，保持音质
        case ResourceType::FONT:
            return 8; // 高压缩比
        case ResourceType::SHADER:
            return 9; // 最高压缩比
        case ResourceType::ANIMATION:
            return 7; // 较高压缩比
        default:
            return 6; // 默认中等压缩比
    }
}

std::string getMimeType(ResourceType type) {
    switch (type) {
        case ResourceType::CONFIG:
            return "application/json";
        case ResourceType::MAP:
            return "application/octet-stream";
        case ResourceType::SCRIPT:
            return "text/plain";
        case ResourceType::TEXTURE:
            return "image/png";
        case ResourceType::MODEL:
            return "application/octet-stream";
        case ResourceType::SOUND:
            return "audio/wav";
        case ResourceType::MUSIC:
            return "audio/midi";
        case ResourceType::FONT:
            return "font/ttf";
        case ResourceType::SHADER:
            return "text/plain";
        case ResourceType::ANIMATION:
            return "application/octet-stream";
        default:
            return "application/octet-stream";
    }
}

bool isValidResourceType(ResourceType type) {
    return type >= ResourceType::CONFIG && type <= ResourceType::UNKNOWN;
}

std::vector<ResourceType> getAllResourceTypes() {
    return {
        ResourceType::CONFIG,
        ResourceType::MAP,
        ResourceType::SCRIPT,
        ResourceType::TEXTURE,
        ResourceType::MODEL,
        ResourceType::SOUND,
        ResourceType::MUSIC,
        ResourceType::FONT,
        ResourceType::SHADER,
        ResourceType::ANIMATION
    };
}

} // namespace Fantasy
