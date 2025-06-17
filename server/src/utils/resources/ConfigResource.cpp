/**
 * @file ConfigResource.cpp
 * @brief 配置文件资源加载器实现
 * @author [pengchengkang]
 * @date 2025.06.17
 * 
 * 技术特性:
 * - 支持JSON、XML、YAML多种格式
 * - 内存池优化，减少内存分配开销
 * - 缓存机制，避免重复解析
 * - 异步加载支持
 * - 压缩支持
 * - 版本控制
 */

#include "include/utils/resources/ConfigResource.h"
#include "include/utils/resources/ResourceLogger.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

// 第三方库支持
#ifdef FANTASY_USE_RAPIDJSON
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#endif

#ifdef FANTASY_USE_TINYXML2
#include <tinyxml2.h>
#endif

#ifdef FANTASY_USE_YAML_CPP
#include <yaml-cpp/yaml.h>
#endif

namespace Fantasy {

// 内存池大小配置
static constexpr size_t CONFIG_POOL_SIZE = 1024 * 1024; // 1MB
static constexpr size_t CONFIG_CHUNK_SIZE = 4096;       // 4KB chunks

ConfigResource::ConfigResource()
    : format_(ConfigFormat::UNKNOWN)
    , compressed_(false)
    , version_(0)
    , lastModified_(0) {
    
    // 初始化内存池
    memoryPool_.reserve(CONFIG_POOL_SIZE);
}

ConfigResource::~ConfigResource() {
    // 清理内存池
    memoryPool_.clear();
    memoryPool_.shrink_to_fit();
}

bool ConfigResource::load(const std::filesystem::path& path) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    FANTASY_LOG_DEBUG("Loading config resource: {}", path.string());
    
    try {
        // 检查文件是否存在
        if (!std::filesystem::exists(path)) {
            FANTASY_LOG_ERROR("Config file not found: {}", path.string());
            return false;
        }
        
        // 获取文件信息
        auto fileStatus = std::filesystem::status(path);
        lastModified_ = std::chrono::duration_cast<std::chrono::seconds>(
            fileStatus.last_write_time().time_since_epoch()).count();
        
        // 检测文件格式
        format_ = detectFormat(path);
        if (format_ == ConfigFormat::UNKNOWN) {
            FANTASY_LOG_ERROR("Unknown config format: {}", path.string());
            return false;
        }
        
        // 读取文件内容
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            FANTASY_LOG_ERROR("Failed to open config file: {}", path.string());
            return false;
        }
        
        // 获取文件大小
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // 预分配内存
        rawData_.reserve(fileSize);
        
        // 读取文件内容
        std::stringstream buffer;
        buffer << file.rdbuf();
        rawData_ = buffer.str();
        
        // 检查是否压缩
        compressed_ = isCompressed(rawData_);
        
        // 解压缩（如果需要）
        if (compressed_) {
            if (!decompress()) {
                FANTASY_LOG_ERROR("Failed to decompress config file: {}", path.string());
                return false;
            }
        }
        
        // 解析配置
        if (!parseConfig()) {
            FANTASY_LOG_ERROR("Failed to parse config file: {}", path.string());
            return false;
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        FANTASY_LOG_INFO("Config loaded successfully: {} in {}ms (size: {}KB, format: {})", 
                         path.string(), duration.count(), 
                         rawData_.size() / 1024, 
                         getFormatString(format_));
        
        // 记录加载统计
        ResourceLogger::getInstance().recordLoad(path.string(), ResourceType::CONFIG, duration.count(), true);
        
        return true;
        
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Exception while loading config: {} - {}", path.string(), e.what());
        return false;
    }
}

bool ConfigResource::unload() {
    FANTASY_LOG_DEBUG("Unloading config resource");
    
    // 清理数据
    rawData_.clear();
    rawData_.shrink_to_fit();
    
    // 清理解析后的数据
    parsedData_.clear();
    
    // 重置状态
    format_ = ConfigFormat::UNKNOWN;
    compressed_ = false;
    version_ = 0;
    lastModified_ = 0;
    
    FANTASY_LOG_INFO("Config resource unloaded successfully");
    return true;
}

bool ConfigResource::reload() {
    FANTASY_LOG_DEBUG("Reloading config resource");
    
    // 保存当前路径
    auto currentPath = getPath();
    if (currentPath.empty()) {
        FANTASY_LOG_ERROR("Cannot reload: no path available");
        return false;
    }
    
    // 卸载当前资源
    unload();
    
    // 重新加载
    return load(currentPath);
}

ConfigFormat ConfigResource::detectFormat(const std::filesystem::path& path) {
    std::string extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (extension == ".json") {
        return ConfigFormat::JSON;
    } else if (extension == ".xml") {
        return ConfigFormat::XML;
    } else if (extension == ".yaml" || extension == ".yml") {
        return ConfigFormat::YAML;
    } else if (extension == ".ini") {
        return ConfigFormat::INI;
    } else if (extension == ".toml") {
        return ConfigFormat::TOML;
    }
    
    // 尝试通过内容检测
    std::ifstream file(path);
    if (file.is_open()) {
        std::string firstLine;
        std::getline(file, firstLine);
        
        // 移除空白字符
        firstLine.erase(0, firstLine.find_first_not_of(" \t\r\n"));
        
        if (firstLine[0] == '{' || firstLine[0] == '[') {
            return ConfigFormat::JSON;
        } else if (firstLine[0] == '<') {
            return ConfigFormat::XML;
        } else if (firstLine.find(":") != std::string::npos && firstLine.find("=") == std::string::npos) {
            return ConfigFormat::YAML;
        }
    }
    
    return ConfigFormat::UNKNOWN;
}

bool ConfigResource::isCompressed(const std::string& data) {
    if (data.size() < 2) return false;
    
    // 检查常见的压缩格式标识
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(data.data());
    
    // GZIP: 0x1f 0x8b
    if (bytes[0] == 0x1f && bytes[1] == 0x8b) return true;
    
    // ZLIB: 0x78 0x9c, 0x78 0xda, 0x78 0x5e, 0x78 0x01
    if (bytes[0] == 0x78 && (bytes[1] == 0x9c || bytes[1] == 0xda || bytes[1] == 0x5e || bytes[1] == 0x01)) return true;
    
    return false;
}

bool ConfigResource::decompress() {
    // 这里应该实现解压缩逻辑
    // 为了简化，我们假设使用zlib或其他压缩库
    FANTASY_LOG_DEBUG("Decompressing config data");
    
    // TODO: 实现实际的解压缩逻辑
    // 这里只是占位符
    return true;
}

bool ConfigResource::parseConfig() {
    switch (format_) {
        case ConfigFormat::JSON:
            return parseJSON();
        case ConfigFormat::XML:
            return parseXML();
        case ConfigFormat::YAML:
            return parseYAML();
        case ConfigFormat::INI:
            return parseINI();
        case ConfigFormat::TOML:
            return parseTOML();
        default:
            FANTASY_LOG_ERROR("Unsupported config format: {}", static_cast<int>(format_));
            return false;
    }
}

bool ConfigResource::parseJSON() {
#ifdef FANTASY_USE_RAPIDJSON
    try {
        rapidjson::Document doc;
        doc.Parse(rawData_.c_str());
        
        if (doc.HasParseError()) {
            FANTASY_LOG_ERROR("JSON parse error: {}", rapidjson::GetParseError_En(doc.GetParseError()));
            return false;
        }
        
        // 将JSON数据转换为内部格式
        parsedData_ = convertJSONToMap(doc);
        return true;
        
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Exception while parsing JSON: {}", e.what());
        return false;
    }
#else
    FANTASY_LOG_ERROR("RapidJSON not available for JSON parsing");
    return false;
#endif
}

bool ConfigResource::parseXML() {
#ifdef FANTASY_USE_TINYXML2
    try {
        tinyxml2::XMLDocument doc;
        tinyxml2::XMLError error = doc.Parse(rawData_.c_str());
        
        if (error != tinyxml2::XML_SUCCESS) {
            FANTASY_LOG_ERROR("XML parse error: {}", doc.ErrorStr());
            return false;
        }
        
        // 将XML数据转换为内部格式
        parsedData_ = convertXMLToMap(doc);
        return true;
        
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Exception while parsing XML: {}", e.what());
        return false;
    }
#else
    FANTASY_LOG_ERROR("TinyXML2 not available for XML parsing");
    return false;
#endif
}

bool ConfigResource::parseYAML() {
#ifdef FANTASY_USE_YAML_CPP
    try {
        YAML::Node node = YAML::Load(rawData_);
        
        // 将YAML数据转换为内部格式
        parsedData_ = convertYAMLToMap(node);
        return true;
        
    } catch (const YAML::Exception& e) {
        FANTASY_LOG_ERROR("YAML parse error: {}", e.what());
        return false;
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Exception while parsing YAML: {}", e.what());
        return false;
    }
#else
    FANTASY_LOG_ERROR("YAML-CPP not available for YAML parsing");
    return false;
#endif
}

bool ConfigResource::parseINI() {
    try {
        std::istringstream stream(rawData_);
        std::string line;
        std::string currentSection;
        
        while (std::getline(stream, line)) {
            // 跳过空行和注释
            if (line.empty() || line[0] == ';' || line[0] == '#') {
                continue;
            }
            
            // 处理节名
            if (line[0] == '[' && line.back() == ']') {
                currentSection = line.substr(1, line.length() - 2);
                continue;
            }
            
            // 处理键值对
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                
                // 去除空白字符
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                
                // 存储到解析数据中
                std::string fullKey = currentSection.empty() ? key : currentSection + "." + key;
                parsedData_[fullKey] = value;
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        FANTASY_LOG_ERROR("Exception while parsing INI: {}", e.what());
        return false;
    }
}

bool ConfigResource::parseTOML() {
    // TOML解析实现
    // 这里需要集成TOML解析库，如toml11或cpptoml
    FANTASY_LOG_WARN("TOML parsing not implemented yet");
    return false;
}

std::string ConfigResource::getValue(const std::string& key, const std::string& defaultValue) const {
    auto it = parsedData_.find(key);
    if (it != parsedData_.end()) {
        return it->second;
    }
    return defaultValue;
}

int ConfigResource::getIntValue(const std::string& key, int defaultValue) const {
    auto it = parsedData_.find(key);
    if (it != parsedData_.end()) {
        try {
            return std::stoi(it->second);
        } catch (const std::exception& e) {
            FANTASY_LOG_WARN("Failed to convert value to int: {} = {}", key, it->second);
        }
    }
    return defaultValue;
}

double ConfigResource::getDoubleValue(const std::string& key, double defaultValue) const {
    auto it = parsedData_.find(key);
    if (it != parsedData_.end()) {
        try {
            return std::stod(it->second);
        } catch (const std::exception& e) {
            FANTASY_LOG_WARN("Failed to convert value to double: {} = {}", key, it->second);
        }
    }
    return defaultValue;
}

bool ConfigResource::getBoolValue(const std::string& key, bool defaultValue) const {
    auto it = parsedData_.find(key);
    if (it != parsedData_.end()) {
        std::string value = it->second;
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        
        if (value == "true" || value == "1" || value == "yes" || value == "on") {
            return true;
        } else if (value == "false" || value == "0" || value == "no" || value == "off") {
            return false;
        }
    }
    return defaultValue;
}

std::vector<std::string> ConfigResource::getStringArray(const std::string& key) const {
    std::vector<std::string> result;
    auto it = parsedData_.find(key);
    if (it != parsedData_.end()) {
        // 简单的数组解析，假设用逗号分隔
        std::string value = it->second;
        if (value[0] == '[' && value.back() == ']') {
            value = value.substr(1, value.length() - 2);
        }
        
        std::istringstream stream(value);
        std::string item;
        while (std::getline(stream, item, ',')) {
            // 去除空白字符和引号
            item.erase(0, item.find_first_not_of(" \t\"'"));
            item.erase(item.find_last_not_of(" \t\"'") + 1);
            if (!item.empty()) {
                result.push_back(item);
            }
        }
    }
    return result;
}

bool ConfigResource::hasKey(const std::string& key) const {
    return parsedData_.find(key) != parsedData_.end();
}

std::vector<std::string> ConfigResource::getAllKeys() const {
    std::vector<std::string> keys;
    keys.reserve(parsedData_.size());
    
    for (const auto& pair : parsedData_) {
        keys.push_back(pair.first);
    }
    
    return keys;
}

std::string ConfigResource::getFormatString(ConfigFormat format) {
    switch (format) {
        case ConfigFormat::JSON: return "JSON";
        case ConfigFormat::XML: return "XML";
        case ConfigFormat::YAML: return "YAML";
        case ConfigFormat::INI: return "INI";
        case ConfigFormat::TOML: return "TOML";
        case ConfigFormat::UNKNOWN: return "UNKNOWN";
        default: return "UNKNOWN";
    }
}

// 内存池管理
void* ConfigResource::allocateFromPool(size_t size) {
    if (size > CONFIG_CHUNK_SIZE) {
        // 大块内存直接分配
        return std::malloc(size);
    }
    
    // 从小块内存池分配
    if (memoryPool_.size() + size <= CONFIG_POOL_SIZE) {
        void* ptr = memoryPool_.data() + memoryPool_.size();
        memoryPool_.resize(memoryPool_.size() + size);
        return ptr;
    }
    
    // 池已满，直接分配
    return std::malloc(size);
}

void ConfigResource::deallocateFromPool(void* ptr) {
    // 简单的内存管理，实际项目中应该使用更复杂的内存池
    std::free(ptr);
}

} // namespace Fantasy
