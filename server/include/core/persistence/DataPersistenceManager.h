/**
 * @file DataPersistenceManager.h
 * @brief 数据持久化管理器 - 集成Core游戏数据与DatabaseManager
 * @author [pengchengkang]
 * @date 2025.06.17
 * 
 * 功能特性:
 * - 角色数据持久化
 * - 游戏存档管理
 * - 物品数据持久化
 * - 任务数据持久化
 * - 关卡数据持久化
 * - 自动保存机制
 * - 数据同步
 * - 缓存管理
 */

#pragma once

#include "data/database/DatabaseManager.h"
#include "core/characters/Character.h"
#include "core/items/Item.h"
#include "core/quests/Quest.h"
#include "core/levels/Level.h"
#include "core/skills/Skill.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>

namespace Fantasy {

// 数据持久化配置
struct DataPersistenceConfig {
    bool enableAutoSave = true;
    std::chrono::seconds autoSaveInterval{300}; // 5分钟
    bool enableDataCompression = true;
    bool enableDataEncryption = false;
    std::string encryptionKey = "";
    size_t maxCacheSize = 1000;
    bool enableDataValidation = true;
    bool enableBackupOnSave = true;
    std::string backupDirectory = "backups";
    size_t maxBackupFiles = 10;
};

// 数据同步状态
enum class SyncStatus {
    SYNCED,         // 已同步
    PENDING_SAVE,   // 等待保存
    PENDING_LOAD,   // 等待加载
    CONFLICT,       // 数据冲突
    ERROR           // 同步错误
};

// 数据变更事件
enum class DataChangeType {
    CHARACTER_CREATED,
    CHARACTER_UPDATED,
    CHARACTER_DELETED,
    ITEM_CREATED,
    ITEM_UPDATED,
    ITEM_DELETED,
    QUEST_CREATED,
    QUEST_UPDATED,
    QUEST_DELETED,
    LEVEL_CREATED,
    LEVEL_UPDATED,
    LEVEL_DELETED,
    SAVE_CREATED,
    SAVE_UPDATED,
    SAVE_DELETED
};

// 数据变更事件结构
struct DataChangeEvent {
    DataChangeType type;
    std::string entityId;
    std::string entityType;
    std::chrono::system_clock::time_point timestamp;
    std::unordered_map<std::string, std::string> changes;
    
    DataChangeEvent(DataChangeType t, const std::string& id, const std::string& entityType)
        : type(t), entityId(id), entityType(entityType), timestamp(std::chrono::system_clock::now()) {}
};

// 数据变更回调函数类型
using DataChangeCallback = std::function<void(const DataChangeEvent&)>;

/**
 * @brief 数据持久化管理器
 * 
 * 负责将Core中的游戏数据与DatabaseManager集成，
 * 提供数据持久化和加载功能
 */
class DataPersistenceManager {
public:
    // 单例模式
    static DataPersistenceManager& getInstance();
    
    // 禁用拷贝和赋值
    DataPersistenceManager(const DataPersistenceManager&) = delete;
    DataPersistenceManager& operator=(const DataPersistenceManager&) = delete;
    
    // 初始化和清理
    bool initialize(const DataPersistenceConfig& config);
    void shutdown();
    bool isInitialized() const;
    
    // 配置管理
    void setConfig(const DataPersistenceConfig& config);
    DataPersistenceConfig getConfig() const;
    
    // 角色数据持久化
    bool saveCharacter(const std::shared_ptr<Character>& character);
    bool saveCharacterAsync(const std::shared_ptr<Character>& character);
    std::shared_ptr<Character> loadCharacter(const std::string& characterId);
    std::shared_ptr<Character> loadCharacterAsync(const std::string& characterId);
    bool deleteCharacter(const std::string& characterId);
    std::vector<std::shared_ptr<Character>> loadAllCharacters();
    std::vector<std::string> getCharacterIds();
    bool characterExists(const std::string& characterId);
    
    // 游戏存档管理
    bool saveGameState(const std::string& saveSlot, const std::shared_ptr<Character>& character, 
                      const std::string& currentLevel, const Vector2D& position);
    bool saveGameStateAsync(const std::string& saveSlot, const std::shared_ptr<Character>& character,
                           const std::string& currentLevel, const Vector2D& position);
    std::optional<GameSaveData> loadGameState(const std::string& saveSlot);
    std::optional<GameSaveData> loadGameStateAsync(const std::string& saveSlot);
    bool deleteGameState(const std::string& saveSlot);
    std::vector<GameSaveData> getAllSaveStates();
    std::vector<GameSaveData> getSaveStatesByCharacter(const std::string& characterId);
    bool saveSlotExists(const std::string& saveSlot);
    
    // 物品数据持久化
    bool saveItem(const std::shared_ptr<Item>& item);
    bool saveItemAsync(const std::shared_ptr<Item>& item);
    std::shared_ptr<Item> loadItem(const std::string& itemId);
    std::shared_ptr<Item> loadItemAsync(const std::string& itemId);
    bool deleteItem(const std::string& itemId);
    std::vector<std::shared_ptr<Item>> loadAllItems();
    std::vector<std::shared_ptr<Item>> loadItemsByType(const std::string& type);
    std::vector<std::shared_ptr<Item>> loadItemsByRarity(const std::string& rarity);
    bool itemExists(const std::string& itemId);
    
    // 任务数据持久化
    bool saveQuest(const std::shared_ptr<Quest>& quest);
    bool saveQuestAsync(const std::shared_ptr<Quest>& quest);
    std::shared_ptr<Quest> loadQuest(const std::string& questId);
    std::shared_ptr<Quest> loadQuestAsync(const std::string& questId);
    bool deleteQuest(const std::string& questId);
    std::vector<std::shared_ptr<Quest>> loadAllQuests();
    std::vector<std::shared_ptr<Quest>> loadQuestsByLevel(int level);
    std::vector<std::shared_ptr<Quest>> loadQuestsByType(const std::string& type);
    bool questExists(const std::string& questId);
    
    // 关卡数据持久化
    bool saveLevel(const std::shared_ptr<Level>& level);
    bool saveLevelAsync(const std::shared_ptr<Level>& level);
    std::shared_ptr<Level> loadLevel(const std::string& levelId);
    std::shared_ptr<Level> loadLevelAsync(const std::string& levelId);
    bool deleteLevel(const std::string& levelId);
    std::vector<std::shared_ptr<Level>> loadAllLevels();
    std::vector<std::shared_ptr<Level>> loadLevelsByDifficulty(double minDifficulty, double maxDifficulty);
    bool levelExists(const std::string& levelId);
    
    // 技能数据持久化
    bool saveSkill(const std::shared_ptr<Skill>& skill);
    bool saveSkillAsync(const std::shared_ptr<Skill>& skill);
    std::shared_ptr<Skill> loadSkill(const std::string& skillId);
    std::shared_ptr<Skill> loadSkillAsync(const std::string& skillId);
    bool deleteSkill(const std::string& skillId);
    std::vector<std::shared_ptr<Skill>> loadAllSkills();
    std::vector<std::shared_ptr<Skill>> loadSkillsByType(const std::string& type);
    bool skillExists(const std::string& skillId);
    
    // 批量操作
    bool saveCharacters(const std::vector<std::shared_ptr<Character>>& characters);
    bool saveItems(const std::vector<std::shared_ptr<Item>>& items);
    bool saveQuests(const std::vector<std::shared_ptr<Quest>>& quests);
    bool saveLevels(const std::vector<std::shared_ptr<Level>>& levels);
    bool saveSkills(const std::vector<std::shared_ptr<Skill>>& skills);
    
    // 数据同步
    bool syncAllData();
    bool syncData(const std::string& entityType, const std::string& entityId);
    SyncStatus getSyncStatus(const std::string& entityType, const std::string& entityId);
    std::vector<std::string> getPendingSaves();
    std::vector<std::string> getPendingLoads();
    
    // 缓存管理
    void clearCache();
    void clearCache(const std::string& entityType);
    void removeFromCache(const std::string& entityType, const std::string& entityId);
    size_t getCacheSize() const;
    size_t getCacheSize(const std::string& entityType) const;
    
    // 自动保存
    void enableAutoSave(bool enable);
    bool isAutoSaveEnabled() const;
    void setAutoSaveInterval(std::chrono::seconds interval);
    std::chrono::seconds getAutoSaveInterval() const;
    void triggerAutoSave();
    
    // 数据验证
    bool validateCharacterData(const std::shared_ptr<Character>& character);
    bool validateItemData(const std::shared_ptr<Item>& item);
    bool validateQuestData(const std::shared_ptr<Quest>& quest);
    bool validateLevelData(const std::shared_ptr<Level>& level);
    bool validateSkillData(const std::shared_ptr<Skill>& skill);
    
    // 数据备份和恢复
    bool backupAllData(const std::string& backupPath);
    bool restoreAllData(const std::string& backupPath);
    bool backupData(const std::string& entityType, const std::string& backupPath);
    bool restoreData(const std::string& entityType, const std::string& backupPath);
    
    // 事件系统
    void subscribeToDataChanges(DataChangeCallback callback);
    void unsubscribeFromDataChanges(DataChangeCallback callback);
    void emitDataChangeEvent(const DataChangeEvent& event);
    
    // 统计信息
    struct PersistenceStats {
        size_t totalSaves;
        size_t totalLoads;
        size_t successfulSaves;
        size_t successfulLoads;
        size_t failedSaves;
        size_t failedLoads;
        std::chrono::milliseconds averageSaveTime;
        std::chrono::milliseconds averageLoadTime;
        size_t cacheHits;
        size_t cacheMisses;
        std::chrono::system_clock::time_point lastSaveTime;
        std::chrono::system_clock::time_point lastLoadTime;
    };
    
    PersistenceStats getStats() const;
    void resetStats();
    
    // 工具方法
    std::string getLastError() const;
    bool isDataValid(const std::string& entityType, const std::string& entityId);
    std::vector<std::string> getInvalidData();
    bool repairData(const std::string& entityType, const std::string& entityId);
    
private:
    DataPersistenceManager();
    ~DataPersistenceManager();
    
    // 内部实现
    class Impl;
    std::unique_ptr<Impl> pImpl_;
    
    // 私有方法
    bool initializeDatabase();
    bool createTables();
    void autoSaveLoop();
    void cleanupLoop();
    
    // 错误处理
    void setLastError(const std::string& error);
    
    // 统计信息
    void updateStats(bool isSave, bool success, 
                    std::chrono::high_resolution_clock::time_point startTime,
                    bool cacheHit = false);
    
    // 同步状态管理
    void updateSyncStatus(const std::string& entityType, 
                         const std::string& entityId, 
                         SyncStatus status);
    
    // 自动保存
    void startAutoSaveThread();
    
    // 数据转换方法
    CharacterData characterToData(const std::shared_ptr<Character>& character);
    std::shared_ptr<Character> dataToCharacter(const CharacterData& data);
    
    ItemData itemToData(const std::shared_ptr<Item>& item);
    std::shared_ptr<Item> dataToItem(const ItemData& data);
    
    QuestData questToData(const std::shared_ptr<Quest>& quest);
    std::shared_ptr<Quest> dataToQuest(const QuestData& data);
    
    LevelData levelToData(const std::shared_ptr<Level>& level);
    std::shared_ptr<Level> dataToLevel(const LevelData& data);
    
    // 缓存管理
    void addToCache(const std::string& entityType, const std::string& entityId, const std::shared_ptr<void>& data);
    std::shared_ptr<void> getFromCache(const std::string& entityType, const std::string& entityId);
    
    // 数据验证
    bool validateData(const std::string& entityType, const std::shared_ptr<void>& data);
    std::string generateValidationError(const std::string& entityType, const std::string& entityId);
};

} // namespace Fantasy 