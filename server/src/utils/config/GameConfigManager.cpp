/**
 * @file GameConfigManager.cpp
 * @brief 游戏配置管理器实现 - 扩展ConfigManager以支持游戏特定配置
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "include/utils/config/GameConfigManager.h"
#include "include/utils/config/ConfigManager.h"
#include "include/utils/logging/Logger.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace Fantasy {

// 单例模式
GameConfigManager& GameConfigManager::getInstance() {
    static GameConfigManager instance;
    return instance;
}

// 构造和析构
GameConfigManager::GameConfigManager() {
    // 默认配置目录
    charactersDir_ = "config/characters";
    levelsDir_ = "config/levels";
    itemsDir_ = "config/items";
    skillsDir_ = "config/skills";
    questsDir_ = "config/quests";
}

GameConfigManager::~GameConfigManager() {
    shutdown();
}

// 初始化和清理
bool GameConfigManager::Init(const std::filesystem::path& configDir) {
    try {
        // 设置配置目录
        charactersDir_ = configDir / "characters";
        levelsDir_ = configDir / "levels";
        itemsDir_ = configDir / "items";
        skillsDir_ = configDir / "skills";
        questsDir_ = configDir / "quests";
        
        // 创建必要的目录
        std::filesystem::create_directories(charactersDir_);
        std::filesystem::create_directories(levelsDir_);
        std::filesystem::create_directories(itemsDir_);
        std::filesystem::create_directories(skillsDir_);
        std::filesystem::create_directories(questsDir_);
        
        // 加载默认配置
        // TODO: 加载默认配置文件
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing GameConfigManager: " << e.what() << std::endl;
        return false;
    }
}

void GameConfigManager::shutdown() {
    // 保存所有配置
    // TODO: 保存所有配置文件
}

// 辅助方法 - 配置文件操作
std::unordered_map<std::string, ConfigValue> GameConfigManager::loadCharacterConfigFromFile(const std::filesystem::path& filePath) {
    std::unordered_map<std::string, ConfigValue> config;
    
    // TODO: 实现配置文件加载
    // 这里简化处理，实际应该根据文件格式选择不同的解析方法
    
    return config;
}

bool GameConfigManager::saveCharacterConfig(const std::string& characterId, const std::unordered_map<std::string, ConfigValue>& config) {
    auto path = getCharacterConfigPath(characterId);
    
    // TODO: 实现配置文件保存
    // 这里简化处理，实际应该根据文件格式选择不同的序列化方法
    
    return true;
}

std::unordered_map<std::string, ConfigValue> GameConfigManager::loadLevelConfigFromFile(const std::filesystem::path& filePath) {
    std::unordered_map<std::string, ConfigValue> config;
    
    // TODO: 实现配置文件加载
    
    return config;
}

bool GameConfigManager::saveLevelConfig(const std::string& levelId, const std::unordered_map<std::string, ConfigValue>& config) {
    auto path = getLevelConfigPath(levelId);
    
    // TODO: 实现配置文件保存
    
    return true;
}

std::unordered_map<std::string, ConfigValue> GameConfigManager::loadItemConfigFromFile(const std::filesystem::path& filePath) {
    std::unordered_map<std::string, ConfigValue> config;
    
    // TODO: 实现配置文件加载
    
    return config;
}

bool GameConfigManager::saveItemConfig(const std::string& itemId, const std::unordered_map<std::string, ConfigValue>& config) {
    auto path = getItemConfigPath(itemId);
    
    // TODO: 实现配置文件保存
    
    return true;
}

std::unordered_map<std::string, ConfigValue> GameConfigManager::loadSkillConfigFromFile(const std::filesystem::path& filePath) {
    std::unordered_map<std::string, ConfigValue> config;
    
    // TODO: 实现配置文件加载
    
    return config;
}

bool GameConfigManager::saveSkillConfig(const std::string& skillId, const std::unordered_map<std::string, ConfigValue>& config) {
    auto path = getSkillConfigPath(skillId);
    
    // TODO: 实现配置文件保存
    
    return true;
}

std::unordered_map<std::string, ConfigValue> GameConfigManager::loadQuestConfigFromFile(const std::filesystem::path& filePath) {
    std::unordered_map<std::string, ConfigValue> config;
    
    // TODO: 实现配置文件加载
    
    return config;
}

bool GameConfigManager::saveQuestConfig(const std::string& questId, const std::unordered_map<std::string, ConfigValue>& config) {
    auto path = getQuestConfigPath(questId);
    
    // TODO: 实现配置文件保存
    
    return true;
}

// 角色配置方法
CharacterStats GameConfigManager::loadCharacterStats(const std::string& characterId) {
    auto path = getCharacterConfigPath(characterId);
    std::unordered_map<std::string, ConfigValue> data = loadCharacterConfigFromFile(path);
    return parseCharacterStats(data);
}

bool GameConfigManager::saveCharacterStats(const std::string& characterId, const CharacterStats& stats) {
    auto path = getCharacterConfigPath(characterId);
    auto data = serializeCharacterStats(stats);
    return saveCharacterConfig(characterId, data);
}

std::unordered_map<std::string, CharacterSkill> GameConfigManager::loadCharacterSkills(const std::string& characterId) {
    std::unordered_map<std::string, CharacterSkill> skills;
    auto path = getCharacterConfigPath(characterId + "_skills");
    std::unordered_map<std::string, ConfigValue> data = loadCharacterConfigFromFile(path);
    
    // 解析技能数据
    for (const auto& [key, value] : data) {
        if (key.find("skill_") == 0) {
            std::string skillId = key.substr(6); // 移除 "skill_" 前缀
            if (std::holds_alternative<std::string>(value)) {
                // 简单格式: skill_id=serialized_data
                std::string serializedSkill = std::get<std::string>(value);
                // 这里需要一个辅助方法来解析序列化的技能数据
                // 暂时使用一个简单的实现
                CharacterSkill skill;
                skill.id = skillId;
                skill.name = skillId;
                skills[skillId] = skill;
            }
        }
    }
    
    return skills;
}

bool GameConfigManager::saveCharacterSkills(const std::string& characterId, 
                                       const std::unordered_map<std::string, CharacterSkill>& skills) {
    std::unordered_map<std::string, ConfigValue> data;
    
    // 序列化技能数据
    for (const auto& [skillId, skill] : skills) {
        // 简单格式: skill_id=serialized_data
        data["skill_" + skillId] = skill.id + ":" + skill.name;
    }
    
    auto path = getCharacterConfigPath(characterId + "_skills");
    return saveCharacterConfig(characterId + "_skills", data);
}

std::unordered_map<EquipmentSlot, std::string> GameConfigManager::loadCharacterEquipment(const std::string& characterId) {
    std::unordered_map<EquipmentSlot, std::string> equipment;
    auto path = getCharacterConfigPath(characterId + "_equipment");
    std::unordered_map<std::string, ConfigValue> data = loadCharacterConfigFromFile(path);
    
    // 解析装备数据
    for (const auto& [key, value] : data) {
        if (key.find("slot_") == 0) {
            std::string slotStr = key.substr(5); // 移除 "slot_" 前缀
            int slotInt = std::stoi(slotStr);
            EquipmentSlot slot = static_cast<EquipmentSlot>(slotInt);
            
            if (std::holds_alternative<std::string>(value)) {
                equipment[slot] = std::get<std::string>(value);
            }
        }
    }
    
    return equipment;
}

bool GameConfigManager::saveCharacterEquipment(const std::string& characterId, 
                                          const std::unordered_map<EquipmentSlot, std::string>& equipment) {
    std::unordered_map<std::string, ConfigValue> data;
    
    // 序列化装备数据
    for (const auto& [slot, itemId] : equipment) {
        data["slot_" + std::to_string(static_cast<int>(slot))] = itemId;
    }
    
    auto path = getCharacterConfigPath(characterId + "_equipment");
    return saveCharacterConfig(characterId + "_equipment", data);
}

// 关卡配置方法
LevelConfig GameConfigManager::loadLevelConfig(const std::string& levelId) {
    auto path = getLevelConfigPath(levelId);
    std::unordered_map<std::string, ConfigValue> data = loadLevelConfigFromFile(path);
    return parseLevelConfig(data);
}

bool GameConfigManager::saveLevelConfig(const std::string& levelId, const LevelConfig& config) {
    auto path = getLevelConfigPath(levelId);
    auto data = serializeLevelConfig(config);
    return saveLevelConfig(levelId, data);
}

std::vector<TerrainTile> GameConfigManager::loadLevelTerrain(const std::string& levelId) {
    std::vector<TerrainTile> terrain;
    auto path = getLevelConfigPath(levelId + "_terrain");
    std::unordered_map<std::string, ConfigValue> data = loadLevelConfigFromFile(path);
    
    // 解析地形数据
    int tileCount = 0;
    if (data.count("tile_count") > 0 && std::holds_alternative<int>(data["tile_count"])) {
        tileCount = std::get<int>(data["tile_count"]);
    }
    
    for (int i = 0; i < tileCount; ++i) {
        std::string prefix = "tile_" + std::to_string(i) + "_";
        std::unordered_map<std::string, ConfigValue> tileData;
        
        // 收集单个地形块的所有属性
        for (const auto& [key, value] : data) {
            if (key.find(prefix) == 0) {
                std::string propName = key.substr(prefix.length());
                tileData[propName] = value;
            }
        }
        
        if (!tileData.empty()) {
            terrain.push_back(parseTerrainTile(tileData));
        }
    }
    
    return terrain;
}

bool GameConfigManager::saveLevelTerrain(const std::string& levelId, const std::vector<TerrainTile>& terrain) {
    std::unordered_map<std::string, ConfigValue> data;
    data["tile_count"] = static_cast<int>(terrain.size());
    
    // 序列化地形数据
    for (size_t i = 0; i < terrain.size(); ++i) {
        std::string prefix = "tile_" + std::to_string(i) + "_";
        auto tileData = serializeTerrainTile(terrain[i]);
        
        for (const auto& [propName, value] : tileData) {
            data[prefix + propName] = value;
        }
    }
    
    auto path = getLevelConfigPath(levelId + "_terrain");
    return saveLevelConfig(levelId + "_terrain", data);
}

std::vector<std::string> GameConfigManager::loadLevelEntities(const std::string& levelId) {
    std::vector<std::string> entities;
    auto path = getLevelConfigPath(levelId + "_entities");
    std::unordered_map<std::string, ConfigValue> data = loadLevelConfigFromFile(path);
    
    // 解析实体数据
    int entityCount = 0;
    if (data.count("entity_count") > 0 && std::holds_alternative<int>(data["entity_count"])) {
        entityCount = std::get<int>(data["entity_count"]);
    }
    
    for (int i = 0; i < entityCount; ++i) {
        std::string key = "entity_" + std::to_string(i);
        if (data.count(key) > 0 && std::holds_alternative<std::string>(data[key])) {
            entities.push_back(std::get<std::string>(data[key]));
        }
    }
    
    return entities;
}

bool GameConfigManager::saveLevelEntities(const std::string& levelId, const std::vector<std::string>& entities) {
    std::unordered_map<std::string, ConfigValue> data;
    data["entity_count"] = static_cast<int>(entities.size());
    
    // 序列化实体数据
    for (size_t i = 0; i < entities.size(); ++i) {
        data["entity_" + std::to_string(i)] = entities[i];
    }
    
    auto path = getLevelConfigPath(levelId + "_entities");
    return saveLevelConfig(levelId + "_entities", data);
}

// 物品配置方法
std::unordered_map<std::string, ConfigValue> GameConfigManager::loadItemConfig(const std::string& itemId) {
    auto path = getItemConfigPath(itemId);
    return loadItemConfigFromFile(path);
}

bool GameConfigManager::saveItemConfig(const std::string& itemId, const std::unordered_map<std::string, ConfigValue>& config) {
    auto path = getItemConfigPath(itemId);
    return saveItemConfig(itemId, config);
}

// 技能配置方法
std::unordered_map<std::string, ConfigValue> GameConfigManager::loadSkillConfig(const std::string& skillId) {
    auto path = getSkillConfigPath(skillId);
    return loadSkillConfigFromFile(path);
}

bool GameConfigManager::saveSkillConfig(const std::string& skillId, const std::unordered_map<std::string, ConfigValue>& config) {
    auto path = getSkillConfigPath(skillId);
    return saveSkillConfig(skillId, config);
}

// 任务配置方法
std::unordered_map<std::string, ConfigValue> GameConfigManager::loadQuestConfig(const std::string& questId) {
    auto path = getQuestConfigPath(questId);
    return loadQuestConfigFromFile(path);
}

bool GameConfigManager::saveQuestConfig(const std::string& questId, const std::unordered_map<std::string, ConfigValue>& config) {
    auto path = getQuestConfigPath(questId);
    return saveQuestConfig(questId, config);
}

// 游戏配置方法
GameConfig GameConfigManager::loadGameConfig() {
    GameConfig config;
    
    // 从ConfigManager中获取游戏配置
    if (hasKey("gameTitle"))
        config.gameTitle = ConfigUtils::toString(getValue("gameTitle"), "Fantasy Legend");
    if (hasKey("version"))
        config.version = ConfigUtils::toString(getValue("version"), "1.0.0");
    if (hasKey("targetFPS"))
        config.targetFPS = ConfigUtils::toInt(getValue("targetFPS"), 60);
    if (hasKey("enableVSync"))
        config.enableVSync = ConfigUtils::toBool(getValue("enableVSync"), true);
    if (hasKey("enableFullscreen"))
        config.enableFullscreen = ConfigUtils::toBool(getValue("enableFullscreen"), false);
    if (hasKey("windowWidth"))
        config.windowWidth = ConfigUtils::toInt(getValue("windowWidth"), 1280);
    if (hasKey("windowHeight"))
        config.windowHeight = ConfigUtils::toInt(getValue("windowHeight"), 720);
    if (hasKey("defaultLanguage"))
        config.defaultLanguage = ConfigUtils::toString(getValue("defaultLanguage"), "zh_CN");
    if (hasKey("enableDebugMode"))
        config.enableDebugMode = ConfigUtils::toBool(getValue("enableDebugMode"), false);
    if (hasKey("enableProfiling"))
        config.enableProfiling = ConfigUtils::toBool(getValue("enableProfiling"), false);
    if (hasKey("saveDirectory"))
        config.saveDirectory = ConfigUtils::toString(getValue("saveDirectory"), "saves");
    if (hasKey("logDirectory"))
        config.logDirectory = ConfigUtils::toString(getValue("logDirectory"), "logs");
    if (hasKey("resourceDirectory"))
        config.resourceDirectory = ConfigUtils::toString(getValue("resourceDirectory"), "resources");
    if (hasKey("configDirectory"))
        config.configDirectory = ConfigUtils::toString(getValue("configDirectory"), "config");
    
    return config;
}

bool GameConfigManager::saveGameConfig(const GameConfig& config) {
    // 将游戏配置保存到ConfigManager
    setValue("gameTitle", config.gameTitle);
    setValue("version", config.version);
    setValue("targetFPS", config.targetFPS);
    setValue("enableVSync", config.enableVSync);
    setValue("enableFullscreen", config.enableFullscreen);
    setValue("windowWidth", config.windowWidth);
    setValue("windowHeight", config.windowHeight);
    setValue("defaultLanguage", config.defaultLanguage);
    setValue("enableDebugMode", config.enableDebugMode);
    setValue("enableProfiling", config.enableProfiling);
    setValue("saveDirectory", config.saveDirectory);
    setValue("logDirectory", config.logDirectory);
    setValue("resourceDirectory", config.resourceDirectory);
    setValue("configDirectory", config.configDirectory);
    
    return true;
}

// 系统配置方法
SystemConfig GameConfigManager::loadSystemConfig() {
    SystemConfig config;
    
    // 从ConfigManager中获取系统配置
    if (hasKey("autoSave"))
        config.autoSave = ConfigUtils::toBool(getValue("autoSave"), true);
    if (hasKey("autoSaveInterval"))
        config.autoSaveInterval = ConfigUtils::toInt(getValue("autoSaveInterval"), 300);
    if (hasKey("maxSaveSlots"))
        config.maxSaveSlots = ConfigUtils::toInt(getValue("maxSaveSlots"), 10);
    if (hasKey("logLevel"))
        config.logLevel = ConfigUtils::toString(getValue("logLevel"), "info");
    if (hasKey("debugMode"))
        config.debugMode = ConfigUtils::toBool(getValue("debugMode"), false);
    
    return config;
}

bool GameConfigManager::saveSystemConfig(const SystemConfig& config) {
    // 将系统配置保存到ConfigManager
    setValue("autoSave", config.autoSave);
    setValue("autoSaveInterval", config.autoSaveInterval);
    setValue("maxSaveSlots", config.maxSaveSlots);
    setValue("logLevel", config.logLevel);
    setValue("debugMode", config.debugMode);
    
    return true;
}

// 配置路径方法
std::filesystem::path GameConfigManager::getCharacterConfigPath(const std::string& characterId) const {
    return charactersDir_ / (characterId + ".cfg");
}

std::filesystem::path GameConfigManager::getLevelConfigPath(const std::string& levelId) const {
    return levelsDir_ / (levelId + ".cfg");
}

std::filesystem::path GameConfigManager::getItemConfigPath(const std::string& itemId) const {
    return itemsDir_ / (itemId + ".cfg");
}

std::filesystem::path GameConfigManager::getSkillConfigPath(const std::string& skillId) const {
    return skillsDir_ / (skillId + ".cfg");
}

std::filesystem::path GameConfigManager::getQuestConfigPath(const std::string& questId) const {
    return questsDir_ / (questId + ".cfg");
}

// 辅助方法实现
CharacterStats GameConfigManager::parseCharacterStats(const std::unordered_map<std::string, ConfigValue>& data) {
    CharacterStats stats;
    
    // 解析基本属性
    if (data.count("level") > 0)
        stats.level = ConfigUtils::toInt(data.at("level"), 1);
    if (data.count("experience") > 0)
        stats.experience = ConfigUtils::toInt(data.at("experience"), 0);
    if (data.count("experienceToNext") > 0)
        stats.experienceToNext = ConfigUtils::toInt(data.at("experienceToNext"), 100);
    
    // 解析生命、魔法和耐力
    if (data.count("health") > 0)
        stats.health = ConfigUtils::toInt(data.at("health"), 100);
    if (data.count("maxHealth") > 0)
        stats.maxHealth = ConfigUtils::toInt(data.at("maxHealth"), 100);
    if (data.count("mana") > 0)
        stats.mana = ConfigUtils::toInt(data.at("mana"), 50);
    if (data.count("maxMana") > 0)
        stats.maxMana = ConfigUtils::toInt(data.at("maxMana"), 50);
    if (data.count("stamina") > 0)
        stats.stamina = ConfigUtils::toInt(data.at("stamina"), 100);
    if (data.count("maxStamina") > 0)
        stats.maxStamina = ConfigUtils::toInt(data.at("maxStamina"), 100);
    
    // 解析战斗属性
    if (data.count("attack") > 0)
        stats.attack = ConfigUtils::toInt(data.at("attack"), 15);
    if (data.count("defense") > 0)
        stats.defense = ConfigUtils::toInt(data.at("defense"), 10);
    if (data.count("magicAttack") > 0)
        stats.magicAttack = ConfigUtils::toInt(data.at("magicAttack"), 10);
    if (data.count("magicDefense") > 0)
        stats.magicDefense = ConfigUtils::toInt(data.at("magicDefense"), 8);
    if (data.count("speed") > 0)
        stats.speed = ConfigUtils::toInt(data.at("speed"), 5);
    if (data.count("criticalRate") > 0)
        stats.criticalRate = ConfigUtils::toInt(data.at("criticalRate"), 5);
    if (data.count("criticalDamage") > 0)
        stats.criticalDamage = ConfigUtils::toInt(data.at("criticalDamage"), 150);
    if (data.count("dodgeRate") > 0)
        stats.dodgeRate = ConfigUtils::toInt(data.at("dodgeRate"), 3);
    if (data.count("blockRate") > 0)
        stats.blockRate = ConfigUtils::toInt(data.at("blockRate"), 2);
    
    // 解析元素抗性
    if (data.count("fireResistance") > 0)
        stats.fireResistance = ConfigUtils::toInt(data.at("fireResistance"), 0);
    if (data.count("iceResistance") > 0)
        stats.iceResistance = ConfigUtils::toInt(data.at("iceResistance"), 0);
    if (data.count("lightningResistance") > 0)
        stats.lightningResistance = ConfigUtils::toInt(data.at("lightningResistance"), 0);
    if (data.count("poisonResistance") > 0)
        stats.poisonResistance = ConfigUtils::toInt(data.at("poisonResistance"), 0);
    if (data.count("holyResistance") > 0)
        stats.holyResistance = ConfigUtils::toInt(data.at("holyResistance"), 0);
    if (data.count("darkResistance") > 0)
        stats.darkResistance = ConfigUtils::toInt(data.at("darkResistance"), 0);
    
    return stats;
}

std::unordered_map<std::string, ConfigValue> GameConfigManager::serializeCharacterStats(const CharacterStats& stats) {
    std::unordered_map<std::string, ConfigValue> data;
    
    // 序列化基本属性
    data["level"] = stats.level;
    data["experience"] = stats.experience;
    data["experienceToNext"] = stats.experienceToNext;
    
    // 序列化生命、魔法和耐力
    data["health"] = stats.health;
    data["maxHealth"] = stats.maxHealth;
    data["mana"] = stats.mana;
    data["maxMana"] = stats.maxMana;
    data["stamina"] = stats.stamina;
    data["maxStamina"] = stats.maxStamina;
    
    // 序列化战斗属性
    data["attack"] = stats.attack;
    data["defense"] = stats.defense;
    data["magicAttack"] = stats.magicAttack;
    data["magicDefense"] = stats.magicDefense;
    data["speed"] = stats.speed;
    data["criticalRate"] = stats.criticalRate;
    data["criticalDamage"] = stats.criticalDamage;
    data["dodgeRate"] = stats.dodgeRate;
    data["blockRate"] = stats.blockRate;
    
    // 序列化元素抗性
    data["fireResistance"] = stats.fireResistance;
    data["iceResistance"] = stats.iceResistance;
    data["lightningResistance"] = stats.lightningResistance;
    data["poisonResistance"] = stats.poisonResistance;
    data["holyResistance"] = stats.holyResistance;
    data["darkResistance"] = stats.darkResistance;
    
    return data;
}

LevelConfig GameConfigManager::parseLevelConfig(const std::unordered_map<std::string, ConfigValue>& data) {
    LevelConfig config;
    
    // 解析基本信息
    if (data.count("id") > 0)
        config.id = ConfigUtils::toString(data.at("id"));
    if (data.count("name") > 0)
        config.name = ConfigUtils::toString(data.at("name"));
    if (data.count("description") > 0)
        config.description = ConfigUtils::toString(data.at("description"));
    if (data.count("type") > 0)
        config.type = static_cast<LevelType>(ConfigUtils::toInt(data.at("type"), 0));
    
    // 解析尺寸和图块信息
    if (data.count("width") > 0)
        config.width = ConfigUtils::toInt(data.at("width"), 100);
    if (data.count("height") > 0)
        config.height = ConfigUtils::toInt(data.at("height"), 100);
    if (data.count("tileSize") > 0)
        config.tileSize = ConfigUtils::toInt(data.at("tileSize"), 32);
    
    // 解析音频
    if (data.count("backgroundMusic") > 0)
        config.backgroundMusic = ConfigUtils::toString(data.at("backgroundMusic"));
    if (data.count("ambientSound") > 0)
        config.ambientSound = ConfigUtils::toString(data.at("ambientSound"));
    
    // 解析游戏规则
    if (data.count("timeLimit") > 0)
        config.timeLimit = ConfigUtils::toInt(data.at("timeLimit"), 0);
    if (data.count("maxPlayers") > 0)
        config.maxPlayers = ConfigUtils::toInt(data.at("maxPlayers"), 1);
    if (data.count("allowRespawn") > 0)
        config.allowRespawn = ConfigUtils::toBool(data.at("allowRespawn"), true);
    
    // 解析数组
    if (data.count("requiredItems") > 0 && std::holds_alternative<std::vector<std::string>>(data.at("requiredItems"))) {
        config.requiredItems = std::get<std::vector<std::string>>(data.at("requiredItems"));
    }
    
    if (data.count("objectives") > 0 && std::holds_alternative<std::vector<std::string>>(data.at("objectives"))) {
        config.objectives = std::get<std::vector<std::string>>(data.at("objectives"));
    }
    
    return config;
}

std::unordered_map<std::string, ConfigValue> GameConfigManager::serializeLevelConfig(const LevelConfig& config) {
    std::unordered_map<std::string, ConfigValue> data;
    
    // 序列化基本信息
    data["id"] = config.id;
    data["name"] = config.name;
    data["description"] = config.description;
    data["type"] = static_cast<int>(config.type);
    
    // 序列化尺寸和图块信息
    data["width"] = config.width;
    data["height"] = config.height;
    data["tileSize"] = config.tileSize;
    
    // 序列化音频
    data["backgroundMusic"] = config.backgroundMusic;
    data["ambientSound"] = config.ambientSound;
    
    // 序列化游戏规则
    data["timeLimit"] = config.timeLimit;
    data["maxPlayers"] = config.maxPlayers;
    data["allowRespawn"] = config.allowRespawn;
    
    // 序列化数组
    data["requiredItems"] = config.requiredItems;
    data["objectives"] = config.objectives;
    
    return data;
}

TerrainTile GameConfigManager::parseTerrainTile(const std::unordered_map<std::string, ConfigValue>& data) {
    TerrainTile tile;
    
    // 解析地形类型
    if (data.count("type") > 0)
        tile.type = static_cast<TerrainType>(ConfigUtils::toInt(data.at("type"), 0));
    
    // 解析位置和边界
    float x = 0.0f, y = 0.0f, width = 32.0f, height = 32.0f;
    
    if (data.count("x") > 0)
        x = ConfigUtils::toDouble(data.at("x"), 0.0);
    if (data.count("y") > 0)
        y = ConfigUtils::toDouble(data.at("y"), 0.0);
    if (data.count("width") > 0)
        width = ConfigUtils::toDouble(data.at("width"), 32.0);
    if (data.count("height") > 0)
        height = ConfigUtils::toDouble(data.at("height"), 32.0);
    
    tile.position = Vector2D(x, y);
    tile.bounds = Rectangle(x, y, width, height);
    
    // 解析移动属性
    if (data.count("walkable") > 0)
        tile.walkable = ConfigUtils::toBool(data.at("walkable"), true);
    if (data.count("swimable") > 0)
        tile.swimable = ConfigUtils::toBool(data.at("swimable"), false);
    if (data.count("flyable") > 0)
        tile.flyable = ConfigUtils::toBool(data.at("flyable"), true);
    if (data.count("movementCost") > 0)
        tile.movementCost = ConfigUtils::toDouble(data.at("movementCost"), 1.0);
    
    return tile;
}

std::unordered_map<std::string, ConfigValue> GameConfigManager::serializeTerrainTile(const TerrainTile& tile) {
    std::unordered_map<std::string, ConfigValue> data;
    
    // 序列化地形类型
    data["type"] = static_cast<int>(tile.type);
    
    // 序列化位置和边界
    data["x"] = tile.position.x;
    data["y"] = tile.position.y;
    data["width"] = tile.bounds.width;
    data["height"] = tile.bounds.height;
    
    // 序列化移动属性
    data["walkable"] = tile.walkable;
    data["swimable"] = tile.swimable;
    data["flyable"] = tile.flyable;
    data["movementCost"] = tile.movementCost;
    
    return data;
}

CharacterSkill GameConfigManager::parseCharacterSkill(const std::unordered_map<std::string, ConfigValue>& data) {
    CharacterSkill skill;
    
    // 解析基本信息
    if (data.count("id") > 0)
        skill.id = ConfigUtils::toString(data.at("id"));
    if (data.count("name") > 0)
        skill.name = ConfigUtils::toString(data.at("name"));
    if (data.count("description") > 0)
        skill.description = ConfigUtils::toString(data.at("description"));
    
    // 解析等级信息
    if (data.count("level") > 0)
        skill.level = ConfigUtils::toInt(data.at("level"), 1);
    if (data.count("maxLevel") > 0)
        skill.maxLevel = ConfigUtils::toInt(data.at("maxLevel"), 10);
    
    // 解析消耗和冷却
    if (data.count("manaCost") > 0)
        skill.manaCost = ConfigUtils::toInt(data.at("manaCost"), 0);
    if (data.count("staminaCost") > 0)
        skill.staminaCost = ConfigUtils::toInt(data.at("staminaCost"), 0);
    if (data.count("cooldown") > 0)
        skill.cooldown = ConfigUtils::toDouble(data.at("cooldown"), 0.0);
    if (data.count("currentCooldown") > 0)
        skill.currentCooldown = ConfigUtils::toDouble(data.at("currentCooldown"), 0.0);
    
    // 解析战斗属性
    if (data.count("range") > 0)
        skill.range = ConfigUtils::toDouble(data.at("range"), 1.0);
    if (data.count("damage") > 0)
        skill.damage = ConfigUtils::toDouble(data.at("damage"), 0.0);
    
    // 解析状态
    if (data.count("isActive") > 0)
        skill.isActive = ConfigUtils::toBool(data.at("isActive"), true);
    
    return skill;
}

std::unordered_map<std::string, ConfigValue> GameConfigManager::serializeCharacterSkill(const CharacterSkill& skill) {
    std::unordered_map<std::string, ConfigValue> data;
    
    // 序列化基本信息
    data["id"] = skill.id;
    data["name"] = skill.name;
    data["description"] = skill.description;
    
    // 序列化等级信息
    data["level"] = skill.level;
    data["maxLevel"] = skill.maxLevel;
    
    // 序列化消耗和冷却
    data["manaCost"] = skill.manaCost;
    data["staminaCost"] = skill.staminaCost;
    data["cooldown"] = skill.cooldown;
    data["currentCooldown"] = skill.currentCooldown;
    
    // 序列化战斗属性
    data["range"] = skill.range;
    data["damage"] = skill.damage;
    
    // 序列化状态
    data["isActive"] = skill.isActive;
    
    return data;
}

// 配置项管理
bool GameConfigManager::hasKey(const std::string& key) const {
    return configs_.find(key) != configs_.end();
}

std::vector<std::string> GameConfigManager::getAllKeys() const {
    std::vector<std::string> keys;
    keys.reserve(configs_.size());
    for (const auto& [key, _] : configs_) {
        keys.push_back(key);
    }
    return keys;
}

} // namespace Fantasy