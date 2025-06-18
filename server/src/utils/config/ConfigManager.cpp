/**
 * @file ConfigManager.cpp
 * @brief 配置管理器实现
 * @details 管理游戏配置的加载、保存、验证和通知
 * @author [pengchengkang]
 * @date 2025.06.18
 */

#include "utils/config/ConfigManager.h"
#include "utils/LogSys/Logger.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <set>

namespace Fantasy {

// ConfigManager::Impl 内部实现
class ConfigManager::Impl {
public:
    std::filesystem::path configDir_;
    std::unordered_map<std::string, ConfigValue> configs_;
    std::unordered_map<std::string, std::vector<ConfigChangeListener>> listeners_;
    std::unordered_map<std::string, std::vector<ConfigItem>> definitions_;
    std::set<std::string> modifiedConfigs_;
    std::atomic<bool> hotReload_{false};
    mutable std::mutex mutex_;
    ConfigManagerStats stats_{};
    
    // 游戏特定配置存储
    std::unordered_map<std::string, ConfigValue> gameConfig_;
    std::unordered_map<std::string, ConfigValue> userConfig_;
    std::unordered_map<std::string, ConfigValue> systemConfig_;
    std::unordered_map<std::string, ConfigValue> dynamicConfig_;
    
    // 配置模板
    std::unordered_map<std::string, std::unordered_map<std::string, ConfigValue>> configTemplates_;
};

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager() : pImpl_(std::make_unique<Impl>()) {}
ConfigManager::~ConfigManager() = default;

bool ConfigManager::initialize(const std::filesystem::path& configDir) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    pImpl_->configDir_ = configDir;
    
    // 创建配置目录
    std::filesystem::create_directories(configDir);
    std::filesystem::create_directories(configDir / "characters");
    std::filesystem::create_directories(configDir / "levels");
    std::filesystem::create_directories(configDir / "items");
    std::filesystem::create_directories(configDir / "skills");
    std::filesystem::create_directories(configDir / "quests");
    
    // 设置默认配置
    setupDefaultGameConfig();
    setupDefaultSystemConfig();
    setupConfigTemplates();
    
    CONFIG_LOG_INFO("ConfigManager initialized with dir: {}", configDir.string());
    return true;
}

void ConfigManager::shutdown() {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    pImpl_->configs_.clear();
    pImpl_->listeners_.clear();
    pImpl_->definitions_.clear();
    pImpl_->modifiedConfigs_.clear();
    pImpl_->gameConfig_.clear();
    pImpl_->userConfig_.clear();
    pImpl_->systemConfig_.clear();
    pImpl_->dynamicConfig_.clear();
    pImpl_->configTemplates_.clear();
    CONFIG_LOG_INFO("ConfigManager shutdown");
}

bool ConfigManager::loadConfig(const std::string& configName, ConfigLevel level) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    std::filesystem::path path = getConfigPath(configName, level);
    std::ifstream file(path);
    if (!file.is_open()) {
        CONFIG_LOG_ERROR("Failed to open config file: {}", path.string());
        return false;
    }
    
    // 简单的key=value格式解析
    std::string line;
    while (std::getline(file, line)) {
        auto pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            pImpl_->configs_[key] = value;
        }
    }
    pImpl_->stats_.loadedConfigs++;
    CONFIG_LOG_INFO("Config loaded: {}", path.string());
    return true;
}

bool ConfigManager::saveConfig(const std::string& configName, ConfigLevel level) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    std::filesystem::path path = getConfigPath(configName, level);
    std::ofstream file(path);
    if (!file.is_open()) {
        CONFIG_LOG_ERROR("Failed to save config file: {}", path.string());
        return false;
    }
    
    for (const auto& [key, value] : pImpl_->configs_) {
        file << key << "=" << ConfigUtils::toString(value) << "\n";
    }
    CONFIG_LOG_INFO("Config saved: {}", path.string());
    return true;
}

bool ConfigManager::reloadConfig(const std::string& configName) {
    return loadConfig(configName);
}

bool ConfigManager::reloadAllConfigs() {
    CONFIG_LOG_INFO("Reloading all configs");
    return true;
}

bool ConfigManager::hasKey(const std::string& key) const {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    return pImpl_->configs_.count(key) > 0;
}

std::vector<std::string> ConfigManager::getAllKeys() const {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    std::vector<std::string> keys;
    for (const auto& [key, _] : pImpl_->configs_) {
        keys.push_back(key);
    }
    return keys;
}

bool ConfigManager::removeKey(const std::string& key, ConfigLevel) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    return pImpl_->configs_.erase(key) > 0;
}

void ConfigManager::addChangeListener(const std::string& key, ConfigChangeListener listener) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    pImpl_->listeners_[key].push_back(listener);
}

void ConfigManager::removeChangeListener(const std::string& key, const ConfigChangeListener&) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    pImpl_->listeners_.erase(key);
}

void ConfigManager::removeAllListeners() {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    pImpl_->listeners_.clear();
}

ConfigManagerStats ConfigManager::getStats() const {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    return pImpl_->stats_;
}

void ConfigManager::resetStats() {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    pImpl_->stats_ = ConfigManagerStats{};
}

void ConfigManager::enableHotReload(bool enable) {
    pImpl_->hotReload_ = enable;
}

bool ConfigManager::isHotReloadEnabled() const {
    return pImpl_->hotReload_;
}

std::string ConfigManager::getConfigPath(const std::string& configName, ConfigLevel) const {
    return (pImpl_->configDir_ / (configName + ".cfg")).string();
}

ConfigFormat ConfigManager::detectFormat(const std::filesystem::path& path) const {
    auto ext = path.extension().string();
    if (ext == ".json") return ConfigFormat::JSON;
    if (ext == ".xml") return ConfigFormat::XML;
    if (ext == ".yaml" || ext == ".yml") return ConfigFormat::YAML;
    if (ext == ".ini") return ConfigFormat::INI;
    if (ext == ".toml") return ConfigFormat::TOML;
    return ConfigFormat::UNKNOWN;
}

// === 游戏特定配置功能实现 ===

void ConfigManager::setupDefaultGameConfig() {
    pImpl_->gameConfig_["version"] = "1.0.0";
    pImpl_->gameConfig_["language"] = "zh_CN";
    pImpl_->gameConfig_["fullscreen"] = false;
    pImpl_->gameConfig_["resolution.width"] = 1920;
    pImpl_->gameConfig_["resolution.height"] = 1080;
    pImpl_->gameConfig_["volume.master"] = 100;
    pImpl_->gameConfig_["volume.music"] = 80;
    pImpl_->gameConfig_["volume.sfx"] = 90;
    pImpl_->gameConfig_["volume.voice"] = 85;
    pImpl_->gameConfig_["graphics.quality"] = "high";
    pImpl_->gameConfig_["graphics.shadows"] = true;
    pImpl_->gameConfig_["graphics.antialiasing"] = true;
    pImpl_->gameConfig_["graphics.vsync"] = true;
}

void ConfigManager::setupDefaultSystemConfig() {
    pImpl_->systemConfig_["auto_save"] = true;
    pImpl_->systemConfig_["auto_save_interval"] = 300;
    pImpl_->systemConfig_["max_save_slots"] = 10;
    pImpl_->systemConfig_["log_level"] = "info";
    pImpl_->systemConfig_["debug_mode"] = false;
}

void ConfigManager::setupConfigTemplates() {
    // 角色配置模板
    pImpl_->configTemplates_["character"] = {
        {"name", ""},
        {"level", 1},
        {"health", 100},
        {"mana", 50},
        {"strength", 10},
        {"agility", 10},
        {"intelligence", 10},
        {"experience", 0},
        {"skills", std::vector<std::string>{}},
        {"equipment", std::vector<std::string>{}}
    };
    
    // 关卡配置模板
    pImpl_->configTemplates_["level"] = {
        {"id", 0},
        {"name", ""},
        {"difficulty", 1.0},
        {"enemies", std::vector<std::string>{}},
        {"rewards", std::vector<std::string>{}},
        {"time_limit", 0},
        {"background", ""},
        {"music", ""}
    };
    
    // 物品配置模板
    pImpl_->configTemplates_["item"] = {
        {"id", ""},
        {"name", ""},
        {"type", ""},
        {"rarity", "common"},
        {"level", 1},
        {"value", 0},
        {"weight", 0.0},
        {"description", ""},
        {"effects", std::vector<std::string>{}}
    };
    
    // 技能配置模板
    pImpl_->configTemplates_["skill"] = {
        {"id", ""},
        {"name", ""},
        {"type", ""},
        {"level", 1},
        {"mana_cost", 0},
        {"cooldown", 0.0},
        {"range", 0.0},
        {"damage", 0},
        {"description", ""},
        {"effects", std::vector<std::string>{}}
    };
}

void ConfigManager::setGameConfig(const std::string& key, const ConfigValue& value) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    pImpl_->gameConfig_[key] = value;
}

ConfigValue ConfigManager::getGameConfig(const std::string& key, const ConfigValue& defaultValue) const {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    auto it = pImpl_->gameConfig_.find(key);
    return it != pImpl_->gameConfig_.end() ? it->second : defaultValue;
}

AppGameConfig ConfigManager::getGameConfig() const {
    AppGameConfig config;
    config.version = ConfigUtils::toString(getGameConfig("version", "1.0.0"));
    config.language = ConfigUtils::toString(getGameConfig("language", "zh_CN"));
    config.fullscreen = ConfigUtils::toBool(getGameConfig("fullscreen", false));
    config.resolution.width = ConfigUtils::toInt(getGameConfig("resolution.width", 1920));
    config.resolution.height = ConfigUtils::toInt(getGameConfig("resolution.height", 1080));
    config.volume.master = ConfigUtils::toInt(getGameConfig("volume.master", 100));
    config.volume.music = ConfigUtils::toInt(getGameConfig("volume.music", 80));
    config.volume.sfx = ConfigUtils::toInt(getGameConfig("volume.sfx", 90));
    config.volume.voice = ConfigUtils::toInt(getGameConfig("volume.voice", 85));
    config.graphics.quality = ConfigUtils::toString(getGameConfig("graphics.quality", "high"));
    config.graphics.shadows = ConfigUtils::toBool(getGameConfig("graphics.shadows", true));
    config.graphics.antialiasing = ConfigUtils::toBool(getGameConfig("graphics.antialiasing", true));
    config.graphics.vsync = ConfigUtils::toBool(getGameConfig("graphics.vsync", true));
    return config;
}

void ConfigManager::setGameConfig(const AppGameConfig& config) {
    setGameConfig("version", config.version);
    setGameConfig("language", config.language);
    setGameConfig("fullscreen", config.fullscreen);
    setGameConfig("resolution.width", config.resolution.width);
    setGameConfig("resolution.height", config.resolution.height);
    setGameConfig("volume.master", config.volume.master);
    setGameConfig("volume.music", config.volume.music);
    setGameConfig("volume.sfx", config.volume.sfx);
    setGameConfig("volume.voice", config.volume.voice);
    setGameConfig("graphics.quality", config.graphics.quality);
    setGameConfig("graphics.shadows", config.graphics.shadows);
    setGameConfig("graphics.antialiasing", config.graphics.antialiasing);
    setGameConfig("graphics.vsync", config.graphics.vsync);
}

void ConfigManager::setUserConfig(const std::string& key, const ConfigValue& value) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    pImpl_->userConfig_[key] = value;
}

ConfigValue ConfigManager::getUserConfig(const std::string& key, const ConfigValue& defaultValue) const {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    auto it = pImpl_->userConfig_.find(key);
    return it != pImpl_->userConfig_.end() ? it->second : defaultValue;
}

void ConfigManager::setSystemConfig(const std::string& key, const ConfigValue& value) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    pImpl_->systemConfig_[key] = value;
}

ConfigValue ConfigManager::getSystemConfig(const std::string& key, const ConfigValue& defaultValue) const {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    auto it = pImpl_->systemConfig_.find(key);
    return it != pImpl_->systemConfig_.end() ? it->second : defaultValue;
}

SystemConfig ConfigManager::getSystemConfig() const {
    SystemConfig config;
    config.autoSave = ConfigUtils::toBool(getSystemConfig("auto_save", true));
    config.autoSaveInterval = ConfigUtils::toInt(getSystemConfig("auto_save_interval", 300));
    config.maxSaveSlots = ConfigUtils::toInt(getSystemConfig("max_save_slots", 10));
    config.logLevel = ConfigUtils::toString(getSystemConfig("log_level", "info"));
    config.debugMode = ConfigUtils::toBool(getSystemConfig("debug_mode", false));
    return config;
}

void ConfigManager::setSystemConfig(const SystemConfig& config) {
    setSystemConfig("auto_save", config.autoSave);
    setSystemConfig("auto_save_interval", config.autoSaveInterval);
    setSystemConfig("max_save_slots", config.maxSaveSlots);
    setSystemConfig("log_level", config.logLevel);
    setSystemConfig("debug_mode", config.debugMode);
}

std::filesystem::path ConfigManager::getGameConfigPath(const std::string& configType, const std::string& id) const {
    if (id.empty()) {
        return pImpl_->configDir_ / configType / "default.cfg";
    }
    return pImpl_->configDir_ / configType / (id + ".cfg");
}

// 角色配置
std::unordered_map<std::string, ConfigValue> ConfigManager::loadCharacterConfig(const std::string& characterType, int level) {
    std::string fileName = characterType + "_" + std::to_string(level) + ".cfg";
    std::filesystem::path path = getGameConfigPath("characters", fileName);
    return loadCharacterConfigFromFile(path);
}

std::unordered_map<std::string, ConfigValue> ConfigManager::loadCharacterConfigFromFile(const std::filesystem::path& filePath) {
    std::unordered_map<std::string, ConfigValue> config;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        CONFIG_LOG_WARN("Failed to open character config file: {}", filePath.string());
        return config;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        auto pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            config[key] = value;
        }
    }
    
    CONFIG_LOG_INFO("Character config loaded: {}", filePath.string());
    return config;
}

bool ConfigManager::saveCharacterConfig(const std::string& characterType, int level, const std::unordered_map<std::string, ConfigValue>& config) {
    std::string fileName = characterType + "_" + std::to_string(level) + ".cfg";
    std::filesystem::path path = getGameConfigPath("characters", fileName);
    
    std::ofstream file(path);
    if (!file.is_open()) {
        CONFIG_LOG_ERROR("Failed to save character config file: {}", path.string());
        return false;
    }
    
    for (const auto& [key, value] : config) {
        file << key << "=" << ConfigUtils::toString(value) << "\n";
    }
    
    CONFIG_LOG_INFO("Character config saved: {}", path.string());
    return true;
}

// 关卡配置
std::unordered_map<std::string, ConfigValue> ConfigManager::loadLevelConfig(int levelId) {
    std::string fileName = "level_" + std::to_string(levelId) + ".cfg";
    std::filesystem::path path = getGameConfigPath("levels", fileName);
    return loadLevelConfigFromFile(path);
}

std::unordered_map<std::string, ConfigValue> ConfigManager::loadLevelConfigFromFile(const std::filesystem::path& filePath) {
    std::unordered_map<std::string, ConfigValue> config;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        CONFIG_LOG_WARN("Failed to open level config file: {}", filePath.string());
        return config;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        auto pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            config[key] = value;
        }
    }
    
    CONFIG_LOG_INFO("Level config loaded: {}", filePath.string());
    return config;
}

bool ConfigManager::saveLevelConfig(int levelId, const std::unordered_map<std::string, ConfigValue>& config) {
    std::string fileName = "level_" + std::to_string(levelId) + ".cfg";
    std::filesystem::path path = getGameConfigPath("levels", fileName);
    
    std::ofstream file(path);
    if (!file.is_open()) {
        CONFIG_LOG_ERROR("Failed to save level config file: {}", path.string());
        return false;
    }
    
    for (const auto& [key, value] : config) {
        file << key << "=" << ConfigUtils::toString(value) << "\n";
    }
    
    CONFIG_LOG_INFO("Level config saved: {}", path.string());
    return true;
}

// 物品配置
std::unordered_map<std::string, ConfigValue> ConfigManager::loadItemConfig(const std::string& itemId) {
    std::filesystem::path path = getGameConfigPath("items", itemId);
    return loadItemConfigFromFile(path);
}

std::unordered_map<std::string, ConfigValue> ConfigManager::loadItemConfigFromFile(const std::filesystem::path& filePath) {
    std::unordered_map<std::string, ConfigValue> config;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        CONFIG_LOG_WARN("Failed to open item config file: {}", filePath.string());
        return config;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        auto pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            config[key] = value;
        }
    }
    
    CONFIG_LOG_INFO("Item config loaded: {}", filePath.string());
    return config;
}

bool ConfigManager::saveItemConfig(const std::string& itemId, const std::unordered_map<std::string, ConfigValue>& config) {
    std::filesystem::path path = getGameConfigPath("items", itemId);
    
    std::ofstream file(path);
    if (!file.is_open()) {
        CONFIG_LOG_ERROR("Failed to save item config file: {}", path.string());
        return false;
    }
    
    for (const auto& [key, value] : config) {
        file << key << "=" << ConfigUtils::toString(value) << "\n";
    }
    
    CONFIG_LOG_INFO("Item config saved: {}", path.string());
    return true;
}

// 技能配置
std::unordered_map<std::string, ConfigValue> ConfigManager::loadSkillConfig(const std::string& skillId) {
    std::filesystem::path path = getGameConfigPath("skills", skillId);
    return loadSkillConfigFromFile(path);
}

std::unordered_map<std::string, ConfigValue> ConfigManager::loadSkillConfigFromFile(const std::filesystem::path& filePath) {
    std::unordered_map<std::string, ConfigValue> config;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        CONFIG_LOG_WARN("Failed to open skill config file: {}", filePath.string());
        return config;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        auto pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            config[key] = value;
        }
    }
    
    CONFIG_LOG_INFO("Skill config loaded: {}", filePath.string());
    return config;
}

bool ConfigManager::saveSkillConfig(const std::string& skillId, const std::unordered_map<std::string, ConfigValue>& config) {
    std::filesystem::path path = getGameConfigPath("skills", skillId);
    
    std::ofstream file(path);
    if (!file.is_open()) {
        CONFIG_LOG_ERROR("Failed to save skill config file: {}", path.string());
        return false;
    }
    
    for (const auto& [key, value] : config) {
        file << key << "=" << ConfigUtils::toString(value) << "\n";
    }
    
    CONFIG_LOG_INFO("Skill config saved: {}", path.string());
    return true;
}

// 任务配置
std::unordered_map<std::string, ConfigValue> ConfigManager::loadQuestConfig(const std::string& questId) {
    std::filesystem::path path = getGameConfigPath("quests", questId);
    return loadQuestConfigFromFile(path);
}

std::unordered_map<std::string, ConfigValue> ConfigManager::loadQuestConfigFromFile(const std::filesystem::path& filePath) {
    std::unordered_map<std::string, ConfigValue> config;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        CONFIG_LOG_WARN("Failed to open quest config file: {}", filePath.string());
        return config;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        auto pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            config[key] = value;
        }
    }
    
    CONFIG_LOG_INFO("Quest config loaded: {}", filePath.string());
    return config;
}

bool ConfigManager::saveQuestConfig(const std::string& questId, const std::unordered_map<std::string, ConfigValue>& config) {
    std::filesystem::path path = getGameConfigPath("quests", questId);
    
    std::ofstream file(path);
    if (!file.is_open()) {
        CONFIG_LOG_ERROR("Failed to save quest config file: {}", path.string());
        return false;
    }
    
    for (const auto& [key, value] : config) {
        file << key << "=" << ConfigUtils::toString(value) << "\n";
    }
    
    CONFIG_LOG_INFO("Quest config saved: {}", path.string());
    return true;
}

// 动态配置生成
std::unordered_map<std::string, ConfigValue> ConfigManager::generateCharacterConfig(const std::string& characterType, int level) {
    auto config = getConfigTemplate("character");
    config["name"] = characterType;
    config["level"] = level;
    config["health"] = 100 + level * 10;
    config["mana"] = 50 + level * 5;
    config["strength"] = 10 + level * 2;
    config["agility"] = 10 + level * 2;
    config["intelligence"] = 10 + level * 2;
    return config;
}

std::unordered_map<std::string, ConfigValue> ConfigManager::generateLevelConfig(int levelId, float difficulty) {
    auto config = getConfigTemplate("level");
    config["id"] = levelId;
    config["name"] = "Level " + std::to_string(levelId);
    config["difficulty"] = difficulty;
    return config;
}

std::unordered_map<std::string, ConfigValue> ConfigManager::generateItemConfig(const std::string& itemType, int level) {
    auto config = getConfigTemplate("item");
    config["id"] = itemType + "_" + std::to_string(level);
    config["name"] = itemType;
    config["type"] = itemType;
    config["level"] = level;
    config["value"] = level * 10;
    return config;
}

std::unordered_map<std::string, ConfigValue> ConfigManager::generateSkillConfig(const std::string& skillType, int level) {
    auto config = getConfigTemplate("skill");
    config["id"] = skillType + "_" + std::to_string(level);
    config["name"] = skillType;
    config["type"] = skillType;
    config["level"] = level;
    config["mana_cost"] = level * 5;
    config["damage"] = level * 10;
    return config;
}

// 配置模板
std::unordered_map<std::string, ConfigValue> ConfigManager::getConfigTemplate(const std::string& configType) {
    auto it = pImpl_->configTemplates_.find(configType);
    if (it != pImpl_->configTemplates_.end()) {
        return it->second;
    }
    return {};
}

bool ConfigManager::applyConfigTemplate(std::unordered_map<std::string, ConfigValue>& config, const std::string& templateType) {
    auto template_ = getConfigTemplate(templateType);
    if (template_.empty()) {
        return false;
    }
    
    for (const auto& [key, value] : template_) {
        if (config.find(key) == config.end()) {
            config[key] = value;
        }
    }
    return true;
}

// 其它方法留空实现或抛异常
void ConfigManager::dumpConfig(const std::string&, std::ostream&) const {}
void ConfigManager::validateAllConfigs() const {}
bool ConfigManager::validateConfig(const std::string&) const { return true; }
bool ConfigManager::validateKey(const std::string&, const ConfigValue&) const { return true; }
void ConfigManager::defineConfig(const std::string&, const std::vector<ConfigItem>&) {}
std::vector<ConfigItem> ConfigManager::getConfigDefinition(const std::string&) const { return {}; }
bool ConfigManager::backupConfig(const std::string&, const std::filesystem::path&) { return false; }
bool ConfigManager::restoreConfig(const std::string&, const std::filesystem::path&) { return false; }
bool ConfigManager::encryptConfig(const std::string&, const std::string&) { return false; }
bool ConfigManager::decryptConfig(const std::string&, const std::string&) { return false; }
bool ConfigManager::exportConfig(const std::string&, const std::filesystem::path&, ConfigFormat) { return false; }
bool ConfigManager::importConfig(const std::string&, const std::filesystem::path&) { return false; }
bool ConfigManager::isConfigModified(const std::string&) const { return false; }

// 工具类实现（示例）
std::string ConfigUtils::toString(const ConfigValue& value) {
    if (std::holds_alternative<std::string>(value)) return std::get<std::string>(value);
    if (std::holds_alternative<int>(value)) return std::to_string(std::get<int>(value));
    if (std::holds_alternative<double>(value)) return std::to_string(std::get<double>(value));
    if (std::holds_alternative<bool>(value)) return std::get<bool>(value) ? "true" : "false";
    if (std::holds_alternative<std::vector<std::string>>(value)) {
        std::ostringstream oss;
        for (const auto& s : std::get<std::vector<std::string>>(value)) oss << s << ",";
        return oss.str();
    }
    return "";
}
int ConfigUtils::toInt(const ConfigValue& value, int def) {
    if (std::holds_alternative<int>(value)) return std::get<int>(value);
    if (std::holds_alternative<std::string>(value)) return std::stoi(std::get<std::string>(value));
    return def;
}
double ConfigUtils::toDouble(const ConfigValue& value, double def) {
    if (std::holds_alternative<double>(value)) return std::get<double>(value);
    if (std::holds_alternative<std::string>(value)) return std::stod(std::get<std::string>(value));
    return def;
}
bool ConfigUtils::toBool(const ConfigValue& value, bool def) {
    if (std::holds_alternative<bool>(value)) return std::get<bool>(value);
    if (std::holds_alternative<std::string>(value)) {
        auto v = std::get<std::string>(value);
        return v == "true" || v == "1" || v == "yes";
    }
    return def;
}
std::vector<std::string> ConfigUtils::toStringArray(const ConfigValue& value) {
    if (std::holds_alternative<std::vector<std::string>>(value)) return std::get<std::vector<std::string>>(value);
    if (std::holds_alternative<std::string>(value)) return {std::get<std::string>(value)};
    return {};
}
ConfigValue ConfigUtils::createValue(const std::string& v) { return v; }
ConfigValue ConfigUtils::createValue(int v) { return v; }
ConfigValue ConfigUtils::createValue(double v) { return v; }
ConfigValue ConfigUtils::createValue(bool v) { return v; }
ConfigValue ConfigUtils::createValue(const std::vector<std::string>& v) { return v; }
bool ConfigUtils::isValidInt(const std::string& v) { try { std::stoi(v); return true; } catch (...) { return false; } }
bool ConfigUtils::isValidDouble(const std::string& v) { try { std::stod(v); return true; } catch (...) { return false; } }
bool ConfigUtils::isValidBool(const std::string& v) { return v == "true" || v == "false" || v == "1" || v == "0"; }
bool ConfigUtils::isValidPath(const std::string& v) { return !v.empty(); }
bool ConfigUtils::isValidEmail(const std::string& v) { return v.find('@') != std::string::npos; }
bool ConfigUtils::isValidIP(const std::string& v) { return v.find('.') != std::string::npos; }
std::string ConfigUtils::formatConfigValue(const ConfigValue& v) { return toString(v); }
std::string ConfigUtils::formatConfigKey(const std::string& k) { return k; }
std::string ConfigUtils::formatConfigPath(const std::filesystem::path& p) { return p.string(); }

} // namespace Fantasy 