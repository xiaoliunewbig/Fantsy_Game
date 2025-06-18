/**
 * @file GameConfigManager.h
 * @brief 游戏配置管理器 - 扩展ConfigManager以支持游戏特定配置
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#pragma once

#include "utils/config/ConfigManager.h"
#include "core/GameEngine.h"
#include "core/characters/Character.h"
#include "core/levels/Level.h"
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace Fantasy {

/**
 * @brief 游戏配置管理器
 * 
 * 扩展ConfigManager以支持游戏特定配置：
 * - 角色配置加载与保存
 * - 关卡配置加载与保存
 * - 物品配置加载与保存
 * - 技能配置加载与保存
 * - 任务配置加载与保存
 */
class GameConfigManager {
public:
    // 单例模式
    static GameConfigManager& getInstance();
    
    // 构造和析构
    GameConfigManager();
    virtual ~GameConfigManager();
    
    // 初始化和清理
    bool Init(const std::filesystem::path& configDir);
    void shutdown();
    
    // 配置值访问
    template<typename T>
    T getValue(const std::string& key, const T& defaultValue = T{}) const;
    
    template<typename T>
    bool setValue(const std::string& key, const T& value, ConfigLevel level = ConfigLevel::APPLICATION);
    
    // 配置项管理
    bool hasKey(const std::string& key) const;
    std::vector<std::string> getAllKeys() const;
    
    // 角色配置
    CharacterStats loadCharacterStats(const std::string& characterId);
    bool saveCharacterStats(const std::string& characterId, const CharacterStats& stats);
    
    std::unordered_map<std::string, CharacterSkill> loadCharacterSkills(const std::string& characterId);
    bool saveCharacterSkills(const std::string& characterId, const std::unordered_map<std::string, CharacterSkill>& skills);
    
    std::unordered_map<EquipmentSlot, std::string> loadCharacterEquipment(const std::string& characterId);
    bool saveCharacterEquipment(const std::string& characterId, const std::unordered_map<EquipmentSlot, std::string>& equipment);
    
    // 关卡配置
    LevelConfig loadLevelConfig(const std::string& levelId);
    bool saveLevelConfig(const std::string& levelId, const LevelConfig& config);
    
    std::vector<TerrainTile> loadLevelTerrain(const std::string& levelId);
    bool saveLevelTerrain(const std::string& levelId, const std::vector<TerrainTile>& terrain);
    
    std::vector<std::string> loadLevelEntities(const std::string& levelId);
    bool saveLevelEntities(const std::string& levelId, const std::vector<std::string>& entities);
    
    // 物品配置
    std::unordered_map<std::string, ConfigValue> loadItemConfig(const std::string& itemId);
    bool saveItemConfig(const std::string& itemId, const std::unordered_map<std::string, ConfigValue>& config);
    
    // 技能配置
    std::unordered_map<std::string, ConfigValue> loadSkillConfig(const std::string& skillId);
    bool saveSkillConfig(const std::string& skillId, const std::unordered_map<std::string, ConfigValue>& config);
    
    // 任务配置
    std::unordered_map<std::string, ConfigValue> loadQuestConfig(const std::string& questId);
    bool saveQuestConfig(const std::string& questId, const std::unordered_map<std::string, ConfigValue>& config);
    
    // 游戏配置
    GameConfig loadGameConfig();
    bool saveGameConfig(const GameConfig& config);
    
    // 系统配置
    SystemConfig loadSystemConfig();
    bool saveSystemConfig(const SystemConfig& config);
    
    // 配置路径
    std::filesystem::path getCharacterConfigPath(const std::string& characterId) const;
    std::filesystem::path getLevelConfigPath(const std::string& levelId) const;
    std::filesystem::path getItemConfigPath(const std::string& itemId) const;
    std::filesystem::path getSkillConfigPath(const std::string& skillId) const;
    std::filesystem::path getQuestConfigPath(const std::string& questId) const;
    
private:
    // 辅助方法
    CharacterStats parseCharacterStats(const std::unordered_map<std::string, ConfigValue>& data);
    std::unordered_map<std::string, ConfigValue> serializeCharacterStats(const CharacterStats& stats);
    
    LevelConfig parseLevelConfig(const std::unordered_map<std::string, ConfigValue>& data);
    std::unordered_map<std::string, ConfigValue> serializeLevelConfig(const LevelConfig& config);
    
    TerrainTile parseTerrainTile(const std::unordered_map<std::string, ConfigValue>& data);
    std::unordered_map<std::string, ConfigValue> serializeTerrainTile(const TerrainTile& tile);
    
    CharacterSkill parseCharacterSkill(const std::unordered_map<std::string, ConfigValue>& data);
    std::unordered_map<std::string, ConfigValue> serializeCharacterSkill(const CharacterSkill& skill);
    
    // 私有辅助方法 - 配置文件操作
    std::unordered_map<std::string, ConfigValue> loadCharacterConfigFromFile(const std::filesystem::path& filePath);
    bool saveCharacterConfig(const std::string& characterId, const std::unordered_map<std::string, ConfigValue>& config);
    
    std::unordered_map<std::string, ConfigValue> loadLevelConfigFromFile(const std::filesystem::path& filePath);
    bool saveLevelConfig(const std::string& levelId, const std::unordered_map<std::string, ConfigValue>& config);
    
    std::unordered_map<std::string, ConfigValue> loadItemConfigFromFile(const std::filesystem::path& filePath);
    
    std::unordered_map<std::string, ConfigValue> loadSkillConfigFromFile(const std::filesystem::path& filePath);
    
    std::unordered_map<std::string, ConfigValue> loadQuestConfigFromFile(const std::filesystem::path& filePath);
    
    // 配置目录
    std::filesystem::path charactersDir_;
    std::filesystem::path levelsDir_;
    std::filesystem::path itemsDir_;
    std::filesystem::path skillsDir_;
    std::filesystem::path questsDir_;
    
    // 配置存储
    std::unordered_map<std::string, ConfigValue> configs_;
};

// 模板方法实现
template<typename T>
T GameConfigManager::getValue(const std::string& key, const T& defaultValue) const {
    if (!hasKey(key)) {
        return defaultValue;
    }
    
    try {
        const auto& value = configs_.at(key);
        if constexpr (std::is_same_v<T, std::string>) {
            return ConfigUtils::toString(value);
        } else if constexpr (std::is_same_v<T, int>) {
            return ConfigUtils::toInt(value);
        } else if constexpr (std::is_same_v<T, double>) {
            return ConfigUtils::toDouble(value);
        } else if constexpr (std::is_same_v<T, bool>) {
            return ConfigUtils::toBool(value);
        } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
            return ConfigUtils::toStringArray(value);
        } else {
            return defaultValue;
        }
    } catch (...) {
        return defaultValue;
    }
}

template<typename T>
bool GameConfigManager::setValue(const std::string& key, const T& value, ConfigLevel) {
    try {
        if constexpr (std::is_same_v<T, std::string>) {
            configs_[key] = ConfigUtils::createValue(value);
        } else if constexpr (std::is_same_v<T, int>) {
            configs_[key] = ConfigUtils::createValue(value);
        } else if constexpr (std::is_same_v<T, double>) {
            configs_[key] = ConfigUtils::createValue(value);
        } else if constexpr (std::is_same_v<T, bool>) {
            configs_[key] = ConfigUtils::createValue(value);
        } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
            configs_[key] = ConfigUtils::createValue(value);
        } else {
            return false;
        }
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace Fantasy 
 