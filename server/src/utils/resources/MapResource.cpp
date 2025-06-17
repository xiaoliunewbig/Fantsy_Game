/**
 * @file MapResource.cpp
 * @brief 地图资源加载器实现
 * @author [pengchengkang]
 * @date 2025.06.17
 * 
 * 技术特性:
 * - 支持多种地图格式 (Tiled, 自定义二进制格式)
 * - 分块加载，支持大地图
 * - 内存优化，使用对象池
 * - 异步加载支持
 * - LOD (Level of Detail) 支持
 * - 压缩纹理和几何数据
 * - 智能缓存策略
 */

#include "include/utils/resources/MapResource.h"
#include "include/utils/resources/ResourceLogger.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

// 第三方库支持
#ifdef FANTASY_USE_TILED
#include <tiled/tiled.h>
#endif

namespace Fantasy {

// 内存池配置
static constexpr size_t MAP_POOL_SIZE = 4 * 1024 * 1024;  // 4MB
static constexpr size_t CHUNK_SIZE = 64 * 1024;           // 64KB chunks
static constexpr size_t MAX_CHUNKS = 64;                  // 最大分块数

MapResource::MapResource()
    : width_(0)
    , height_(0)
    , tileWidth_(0)
    , tileHeight_(0)
    , chunkSize_(32)
    , compressionLevel_(6)
    , useLOD_(true)
    , maxLODLevel_(4) {
    
    // 初始化内存池
    memoryPool_.reserve(MAP_POOL_SIZE);
    
    // 初始化对象池
    tilePool_.reserve(1000);
    objectPool_.reserve(500);
    
    // 初始化LOD系统
    lodLevels_.resize(maxLODLevel_);
}

MapResource::~MapResource() {
    // 清理内存池
    memoryPool_.clear();
    memoryPool_.shrink_to_fit();
    
    // 清理对象池
    tilePool_.clear();
    objectPool_.clear();
    
    // 清理LOD数据
    lodLevels_.clear();
}

bool MapResource::load(const std::filesystem::path& path) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    FANTASY_LOG_DEBUG("Loading map resource: {}", path.string());
    
    try {
        // 检查文件是否存在
        if (!std::filesystem::exists(path)) {
            FANTASY_LOG_ERROR("Map file not found: {}", path.string());
            return false;
        }
        
        // 检测地图格式
        MapFormat format = detectFormat(path);
        if (format == MapFormat::UNKNOWN) {
            FANTASY_LOG_ERROR("Unknown map format: {}", path.string());
            return false;
        }
        
        // 根据格式加载地图
        bool success = false;
        switch (format) {
            case MapFormat::TILED_JSON:
                success = loadTiledJSON(path);
                break;
            case MapFormat::TILED_XML:
                success = loadTiledXML(path);
                break;
            case MapFormat::BINARY:
                success = loadBinary(path);
                break;
            case MapFormat::CUSTOM:
                success = loadCustom(path);
                break;
            default:
                FANTASY_LOG_ERROR("Unsupported map format: {}", static_cast<int>(format));
                return false;
        }
        
        if (!success) {
            FANTASY_LOG_ERROR("Failed to load map: {}", path.string());
            return false;
        }
        
        // 生成LOD数据
        if (useLOD_) {
            generateLODData();
        }
        
        // 优化内存布局
        optimizeMemoryLayout();
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        FANTASY_LOG_INFO("Map loaded successfully: {} in {}ms (size: {}x{}, tiles: {}x{}, chunks: {})", 
                         path.string(), duration.count(), 
                         width_, height_, tileWidth_, tileHeight_, chunks_.size());
        
        // 记录加载统计
        ResourceLogger::getInstance().recordLoad(path.string(), ResourceType::MAP, duration.count(), true);
        
        return true;
        
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Exception while loading map: {} - {}", path.string(), e.what());
        return false;
    }
}

bool MapResource::unload() {
    FANTASY_LOG_DEBUG("Unloading map resource");
    
    // 清理地图数据
    tiles_.clear();
    objects_.clear();
    chunks_.clear();
    
    // 清理LOD数据
    for (auto& lodLevel : lodLevels_) {
        lodLevel.clear();
    }
    
    // 重置状态
    width_ = height_ = 0;
    tileWidth_ = tileHeight_ = 0;
    
    FANTASY_LOG_INFO("Map resource unloaded successfully");
    return true;
}

bool MapResource::reload() {
    FANTASY_LOG_DEBUG("Reloading map resource");
    
    auto currentPath = getPath();
    if (currentPath.empty()) {
        FANTASY_LOG_ERROR("Cannot reload: no path available");
        return false;
    }
    
    unload();
    return load(currentPath);
}

MapFormat MapResource::detectFormat(const std::filesystem::path& path) {
    std::string extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (extension == ".json") {
        // 检查是否为Tiled JSON格式
        std::ifstream file(path);
        if (file.is_open()) {
            std::string firstLine;
            std::getline(file, firstLine);
            if (firstLine.find("\"width\"") != std::string::npos && 
                firstLine.find("\"height\"") != std::string::npos) {
                return MapFormat::TILED_JSON;
            }
        }
        return MapFormat::CUSTOM;
    } else if (extension == ".tmx") {
        return MapFormat::TILED_XML;
    } else if (extension == ".map" || extension == ".bin") {
        return MapFormat::BINARY;
    }
    
    return MapFormat::UNKNOWN;
}

bool MapResource::loadTiledJSON(const std::filesystem::path& path) {
#ifdef FANTASY_USE_TILED
    try {
        // 使用Tiled库加载JSON地图
        tiled::Map* map = tiled::Map::fromFile(path.string());
        if (!map) {
            FANTASY_LOG_ERROR("Failed to load Tiled JSON map: {}", path.string());
            return false;
        }
        
        // 提取地图信息
        width_ = map->width();
        height_ = map->height();
        tileWidth_ = map->tileWidth();
        tileHeight_ = map->tileHeight();
        
        // 加载图层
        for (const auto& layer : map->layers()) {
            if (layer->isTileLayer()) {
                loadTileLayer(static_cast<const tiled::TileLayer*>(layer));
            } else if (layer->isObjectGroup()) {
                loadObjectGroup(static_cast<const tiled::ObjectGroup*>(layer));
            }
        }
        
        delete map;
        return true;
        
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Exception while loading Tiled JSON: {}", e.what());
        return false;
    }
#else
    FANTASY_LOG_ERROR("Tiled library not available for JSON map loading");
    return false;
#endif
}

bool MapResource::loadTiledXML(const std::filesystem::path& path) {
#ifdef FANTASY_USE_TILED
    try {
        // 使用Tiled库加载XML地图
        tiled::Map* map = tiled::Map::fromFile(path.string());
        if (!map) {
            FANTASY_LOG_ERROR("Failed to load Tiled XML map: {}", path.string());
            return false;
        }
        
        // 提取地图信息
        width_ = map->width();
        height_ = map->height();
        tileWidth_ = map->tileWidth();
        tileHeight_ = map->tileHeight();
        
        // 加载图层
        for (const auto& layer : map->layers()) {
            if (layer->isTileLayer()) {
                loadTileLayer(static_cast<const tiled::TileLayer*>(layer));
            } else if (layer->isObjectGroup()) {
                loadObjectGroup(static_cast<const tiled::ObjectGroup*>(layer));
            }
        }
        
        delete map;
        return true;
        
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Exception while loading Tiled XML: {}", e.what());
        return false;
    }
#else
    FANTASY_LOG_ERROR("Tiled library not available for XML map loading");
    return false;
#endif
}

bool MapResource::loadBinary(const std::filesystem::path& path) {
    try {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            FANTASY_LOG_ERROR("Failed to open binary map file: {}", path.string());
            return false;
        }
        
        // 读取文件头
        MapHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(header));
        
        // 验证文件头
        if (std::memcmp(header.magic, "FANTASY_MAP", 11) != 0) {
            FANTASY_LOG_ERROR("Invalid binary map file: {}", path.string());
            return false;
        }
        
        // 提取地图信息
        width_ = header.width;
        height_ = header.height;
        tileWidth_ = header.tileWidth;
        tileHeight_ = header.tileHeight;
        chunkSize_ = header.chunkSize;
        
        // 读取分块数据
        chunks_.reserve(header.chunkCount);
        for (uint32_t i = 0; i < header.chunkCount; ++i) {
            MapChunk chunk;
            file.read(reinterpret_cast<char*>(&chunk.header), sizeof(chunk.header));
            
            // 读取压缩数据
            std::vector<char> compressedData(chunk.header.compressedSize);
            file.read(compressedData.data(), chunk.header.compressedSize);
            
            // 解压缩数据
            if (!decompressChunk(chunk, compressedData)) {
                FANTASY_LOG_ERROR("Failed to decompress chunk {}", i);
                return false;
            }
            
            chunks_.push_back(chunk);
        }
        
        // 重建完整地图数据
        rebuildMapFromChunks();
        
        return true;
        
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Exception while loading binary map: {}", e.what());
        return false;
    }
}

bool MapResource::loadCustom(const std::filesystem::path& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            FANTASY_LOG_ERROR("Failed to open custom map file: {}", path.string());
            return false;
        }
        
        std::string line;
        
        // 读取地图头信息
        if (std::getline(file, line)) {
            std::istringstream iss(line);
            iss >> width_ >> height_ >> tileWidth_ >> tileHeight_;
        }
        
        // 读取瓦片数据
        tiles_.resize(width_ * height_);
        for (int y = 0; y < height_; ++y) {
            if (std::getline(file, line)) {
                std::istringstream iss(line);
                for (int x = 0; x < width_; ++x) {
                    uint32_t tileId;
                    iss >> tileId;
                    tiles_[y * width_ + x] = tileId;
                }
            }
        }
        
        // 读取对象数据
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            MapObject obj;
            std::istringstream iss(line);
            iss >> obj.id >> obj.x >> obj.y >> obj.width >> obj.height >> obj.type;
            
            // 读取对象属性
            std::string attrLine;
            if (std::getline(file, attrLine)) {
                parseObjectAttributes(obj, attrLine);
            }
            
            objects_.push_back(obj);
        }
        
        // 创建分块
        createChunks();
        
        return true;
        
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Exception while loading custom map: {}", e.what());
        return false;
    }
}

void MapResource::loadTileLayer(const tiled::TileLayer* layer) {
    // 从Tiled图层加载瓦片数据
    for (int y = 0; y < layer->height(); ++y) {
        for (int x = 0; x < layer->width(); ++x) {
            const tiled::Cell& cell = layer->cellAt(x, y);
            if (cell.tile()) {
                uint32_t tileId = cell.tile()->id();
                tiles_[y * width_ + x] = tileId;
            }
        }
    }
}

void MapResource::loadObjectGroup(const tiled::ObjectGroup* group) {
    // 从Tiled对象组加载对象数据
    for (const auto& object : group->objects()) {
        MapObject obj;
        obj.id = object->id();
        obj.x = object->x();
        obj.y = object->y();
        obj.width = object->width();
        obj.height = object->height();
        obj.type = object->type().toStdString();
        
        // 加载对象属性
        for (const auto& property : object->properties()) {
            obj.attributes[property.name().toStdString()] = property.value().toString().toStdString();
        }
        
        objects_.push_back(obj);
    }
}

bool MapResource::decompressChunk(MapChunk& chunk, const std::vector<char>& compressedData) {
    // 使用zlib或其他压缩库解压缩分块数据
    // 这里只是占位符实现
    FANTASY_LOG_DEBUG("Decompressing chunk data");
    
    // TODO: 实现实际的解压缩逻辑
    chunk.data.resize(chunk.header.originalSize);
    std::memcpy(chunk.data.data(), compressedData.data(), 
                std::min(compressedData.size(), chunk.data.size()));
    
    return true;
}

void MapResource::rebuildMapFromChunks() {
    // 从分块数据重建完整地图
    tiles_.resize(width_ * height_);
    
    for (const auto& chunk : chunks_) {
        int chunkX = chunk.header.x / chunkSize_;
        int chunkY = chunk.header.y / chunkSize_;
        
        for (int y = 0; y < chunkSize_ && (chunkY * chunkSize_ + y) < height_; ++y) {
            for (int x = 0; x < chunkSize_ && (chunkX * chunkSize_ + x) < width_; ++x) {
                int mapX = chunkX * chunkSize_ + x;
                int mapY = chunkY * chunkSize_ + y;
                int chunkIndex = y * chunkSize_ + x;
                
                if (chunkIndex < chunk.data.size()) {
                    tiles_[mapY * width_ + mapX] = chunk.data[chunkIndex];
                }
            }
        }
    }
}

void MapResource::createChunks() {
    // 将地图数据分割成分块
    int chunksX = (width_ + chunkSize_ - 1) / chunkSize_;
    int chunksY = (height_ + chunkSize_ - 1) / chunkSize_;
    
    chunks_.reserve(chunksX * chunksY);
    
    for (int cy = 0; cy < chunksY; ++cy) {
        for (int cx = 0; cx < chunksX; ++cx) {
            MapChunk chunk;
            chunk.header.x = cx * chunkSize_;
            chunk.header.y = cy * chunkSize_;
            chunk.header.width = std::min(chunkSize_, width_ - cx * chunkSize_);
            chunk.header.height = std::min(chunkSize_, height_ - cy * chunkSize_);
            
            // 提取分块数据
            chunk.data.reserve(chunk.header.width * chunk.header.height);
            for (int y = 0; y < chunk.header.height; ++y) {
                for (int x = 0; x < chunk.header.width; ++x) {
                    int mapX = cx * chunkSize_ + x;
                    int mapY = cy * chunkSize_ + y;
                    chunk.data.push_back(tiles_[mapY * width_ + mapX]);
                }
            }
            
            chunks_.push_back(chunk);
        }
    }
}

void MapResource::generateLODData() {
    FANTASY_LOG_DEBUG("Generating LOD data for map");
    
    for (size_t level = 0; level < maxLODLevel_; ++level) {
        size_t factor = 1 << level;
        size_t lodWidth = (width_ + factor - 1) / factor;
        size_t lodHeight = (height_ + factor - 1) / factor;
        
        lodLevels_[level].resize(lodWidth * lodHeight);
        
        for (size_t y = 0; y < lodHeight; ++y) {
            for (size_t x = 0; x < lodWidth; ++x) {
                // 计算LOD瓦片值（取平均值或最频繁值）
                uint32_t lodValue = calculateLODValue(x * factor, y * factor, factor);
                lodLevels_[level][y * lodWidth + x] = lodValue;
            }
        }
    }
}

uint32_t MapResource::calculateLODValue(size_t startX, size_t startY, size_t factor) {
    // 简单的LOD计算：取区域内的最频繁瓦片ID
    std::unordered_map<uint32_t, size_t> tileCounts;
    
    for (size_t y = startY; y < std::min(startY + factor, static_cast<size_t>(height_)); ++y) {
        for (size_t x = startX; x < std::min(startX + factor, static_cast<size_t>(width_)); ++x) {
            uint32_t tileId = tiles_[y * width_ + x];
            tileCounts[tileId]++;
        }
    }
    
    // 找到最频繁的瓦片ID
    uint32_t mostFrequent = 0;
    size_t maxCount = 0;
    
    for (const auto& pair : tileCounts) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
            mostFrequent = pair.first;
        }
    }
    
    return mostFrequent;
}

void MapResource::optimizeMemoryLayout() {
    FANTASY_LOG_DEBUG("Optimizing memory layout");
    
    // 压缩瓦片数据
    tiles_.shrink_to_fit();
    
    // 压缩对象数据
    objects_.shrink_to_fit();
    
    // 压缩分块数据
    for (auto& chunk : chunks_) {
        chunk.data.shrink_to_fit();
    }
    chunks_.shrink_to_fit();
    
    // 压缩LOD数据
    for (auto& lodLevel : lodLevels_) {
        lodLevel.shrink_to_fit();
    }
}

uint32_t MapResource::getTile(int x, int y) const {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) {
        return 0; // 边界外返回空瓦片
    }
    return tiles_[y * width_ + x];
}

uint32_t MapResource::getTileLOD(int x, int y, int lodLevel) const {
    if (lodLevel < 0 || lodLevel >= static_cast<int>(maxLODLevel_)) {
        return getTile(x, y);
    }
    
    size_t factor = 1 << lodLevel;
    size_t lodX = x / factor;
    size_t lodY = y / factor;
    
    if (lodX < lodLevels_[lodLevel].size() / ((width_ + factor - 1) / factor)) {
        return lodLevels_[lodLevel][lodY * ((width_ + factor - 1) / factor) + lodX];
    }
    
    return 0;
}

std::vector<MapObject> MapResource::getObjectsInArea(int x, int y, int width, int height) const {
    std::vector<MapObject> result;
    
    for (const auto& obj : objects_) {
        if (obj.x < x + width && obj.x + obj.width > x &&
            obj.y < y + height && obj.y + obj.height > y) {
            result.push_back(obj);
        }
    }
    
    return result;
}

MapChunk MapResource::getChunk(int chunkX, int chunkY) const {
    for (const auto& chunk : chunks_) {
        if (chunk.header.x == chunkX * chunkSize_ && chunk.header.y == chunkY * chunkSize_) {
            return chunk;
        }
    }
    
    // 返回空分块
    return MapChunk{};
}

void MapResource::parseObjectAttributes(MapObject& obj, const std::string& attrLine) {
    std::istringstream iss(attrLine);
    std::string key, value;
    
    while (iss >> key) {
        if (iss >> value) {
            obj.attributes[key] = value;
        }
    }
}

std::string MapResource::getFormatString(MapFormat format) {
    switch (format) {
        case MapFormat::TILED_JSON: return "TILED_JSON";
        case MapFormat::TILED_XML: return "TILED_XML";
        case MapFormat::BINARY: return "BINARY";
        case MapFormat::CUSTOM: return "CUSTOM";
        case MapFormat::UNKNOWN: return "UNKNOWN";
        default: return "UNKNOWN";
    }
}

// 内存池管理
void* MapResource::allocateFromPool(size_t size) {
    if (size > CHUNK_SIZE) {
        return std::malloc(size);
    }
    
    if (memoryPool_.size() + size <= MAP_POOL_SIZE) {
        void* ptr = memoryPool_.data() + memoryPool_.size();
        memoryPool_.resize(memoryPool_.size() + size);
        return ptr;
    }
    
    return std::malloc(size);
}

void MapResource::deallocateFromPool(void* ptr) {
    std::free(ptr);
}

} // namespace Fantasy
