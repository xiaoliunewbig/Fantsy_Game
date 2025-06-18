/**
 * @file DatabaseManager.h
 * @brief 数据库管理器 - 统一管理多个数据库连接
 * @author [pengchengkang]
 * @date 2025.06.17
 * 
 * 功能特性:
 * - 多数据库支持 (主数据库、缓存数据库、日志数据库)
 * - 连接池管理
 * - 自动重连机制
 * - 负载均衡
 * - 读写分离
 * - 数据迁移
 * - 性能监控
 * - 游戏特定数据模型
 */

#pragma once

#include "Database.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <atomic>
#include <mutex>
#include <thread>

namespace Fantasy {

// 数据库角色类型
enum class DatabaseRole {
    MASTER,     // 主数据库
    SLAVE,      // 从数据库
    CACHE,      // 缓存数据库
    LOG,        // 日志数据库
    ANALYTICS   // 分析数据库
};

// 数据库连接信息
struct DatabaseConnectionInfo {
    DatabaseRole role;
    DatabaseConfig config;
    std::string name;
    bool enabled;
    int priority;
    std::chrono::milliseconds lastPingTime;
    bool isHealthy;
};

// 数据库管理器配置
struct DatabaseManagerConfig {
    std::unordered_map<std::string, DatabaseConnectionInfo> connections;
    size_t maxRetryAttempts = 3;
    std::chrono::milliseconds retryDelay{1000};
    std::chrono::milliseconds healthCheckInterval{30000};
    bool enableAutoReconnect = true;
    bool enableLoadBalancing = true;
    bool enableReadWriteSeparation = true;
    bool enablePerformanceMonitoring = true;
    bool enableQueryLogging = true;
    size_t maxQueryLogSize = 1000;
    std::string backupDirectory = "backups";
    bool enableAutoBackup = true;
    std::chrono::hours autoBackupInterval{24};
    size_t maxBackupFiles = 7;
};

// 数据库管理器统计信息
struct DatabaseManagerStats {
    size_t totalConnections;
    size_t activeConnections;
    size_t failedConnections;
    size_t totalQueries;
    size_t successfulQueries;
    size_t failedQueries;
    std::chrono::milliseconds averageQueryTime;
    std::chrono::milliseconds totalQueryTime;
    size_t totalBytesTransferred;
    std::chrono::system_clock::time_point lastQueryTime;
    std::chrono::system_clock::time_point lastBackupTime;
    size_t totalBackups;
    size_t successfulBackups;
    size_t failedBackups;
};

// 游戏数据模型
struct CharacterData {
    std::string id;
    std::string name;
    int level;
    int experience;
    int health;
    int mana;
    int strength;
    int agility;
    int intelligence;
    std::vector<std::string> skills;
    std::vector<std::string> equipment;
    std::chrono::system_clock::time_point lastLoginTime;
    std::chrono::system_clock::time_point createdTime;
};

struct GameSaveData {
    std::string saveSlot;
    std::string characterId;
    int level;
    std::string currentMap;
    double positionX;
    double positionY;
    std::unordered_map<std::string, std::string> gameState;
    std::chrono::system_clock::time_point saveTime;
};

struct QuestData {
    std::string id;
    std::string name;
    std::string description;
    std::string type;
    std::vector<std::string> objectives;
    std::vector<std::string> rewards;
    int requiredLevel;
    bool isRepeatable;
    std::chrono::system_clock::time_point createdTime;
};

struct ItemData {
    std::string id;
    std::string name;
    std::string type;
    std::string rarity;
    int level;
    int value;
    double weight;
    std::string description;
    std::vector<std::string> effects;
    std::chrono::system_clock::time_point createdTime;
};

struct LevelData {
    std::string id;
    std::string name;
    std::string description;
    double difficulty;
    std::vector<std::string> enemies;
    std::vector<std::string> rewards;
    int timeLimit;
    std::string background;
    std::string music;
    std::chrono::system_clock::time_point createdTime;
};

/**
 * @brief 数据库管理器主类
 */
class DatabaseManager {
public:
    // 单例模式
    static DatabaseManager& getInstance();
    
    // 禁用拷贝和赋值
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    
    // 初始化和清理
    bool initialize(const DatabaseManagerConfig& config);
    void shutdown();
    bool isInitialized() const;
    
    // 连接管理
    bool addConnection(const std::string& name, const DatabaseConnectionInfo& info);
    bool removeConnection(const std::string& name);
    bool enableConnection(const std::string& name, bool enable);
    std::vector<std::string> getConnectionNames() const;
    DatabaseConnectionInfo getConnectionInfo(const std::string& name) const;
    
    // 数据库操作
    std::shared_ptr<Database> getDatabase(const std::string& name);
    std::shared_ptr<Database> getDatabase(DatabaseRole role);
    std::shared_ptr<Database> getMasterDatabase();
    std::shared_ptr<Database> getSlaveDatabase();
    std::shared_ptr<Database> getCacheDatabase();
    std::shared_ptr<Database> getLogDatabase();
    
    // 便捷查询方法
    DatabaseResult query(const std::string& sql, const std::string& connectionName = "");
    DatabaseResult query(const std::string& sql, const std::vector<DatabaseValue>& params, const std::string& connectionName = "");
    std::future<DatabaseResult> queryAsync(const std::string& sql, const std::string& connectionName = "");
    std::future<DatabaseResult> queryAsync(const std::string& sql, const std::vector<DatabaseValue>& params, const std::string& connectionName = "");
    
    // 事务管理
    bool beginTransaction(const std::string& connectionName = "");
    bool commitTransaction(const std::string& connectionName = "");
    bool rollbackTransaction(const std::string& connectionName = "");
    bool isInTransaction(const std::string& connectionName = "") const;
    
    // 数据库维护
    bool createTables();
    bool dropTables();
    bool backupAll();
    bool restoreAll(const std::string& backupPath);
    bool optimizeAll();
    bool vacuumAll();
    
    // 健康检查
    bool checkHealth();
    bool pingAll();
    std::vector<std::string> getUnhealthyConnections() const;
    
    // 数据迁移
    bool migrateData(const std::string& sourceConnection, const std::string& targetConnection);
    bool syncData(const std::string& sourceConnection, const std::string& targetConnection);
    
    // 游戏特定数据操作
    
    // 角色数据
    bool saveCharacter(const CharacterData& character);
    std::optional<CharacterData> loadCharacter(const std::string& characterId);
    bool deleteCharacter(const std::string& characterId);
    std::vector<CharacterData> getAllCharacters();
    std::vector<CharacterData> getCharactersByLevel(int minLevel, int maxLevel);
    bool updateCharacterLevel(const std::string& characterId, int newLevel);
    bool updateCharacterExperience(const std::string& characterId, int experience);
    
    // 存档数据
    bool saveGameData(const GameSaveData& saveData);
    std::optional<GameSaveData> loadGameData(const std::string& saveSlot);
    bool deleteGameData(const std::string& saveSlot);
    std::vector<GameSaveData> getAllSaveData();
    std::vector<GameSaveData> getSaveDataByCharacter(const std::string& characterId);
    
    // 任务数据
    bool saveQuest(const QuestData& quest);
    std::optional<QuestData> loadQuest(const std::string& questId);
    bool deleteQuest(const std::string& questId);
    std::vector<QuestData> getAllQuests();
    std::vector<QuestData> getQuestsByLevel(int level);
    std::vector<QuestData> getQuestsByType(const std::string& type);
    
    // 物品数据
    bool saveItem(const ItemData& item);
    std::optional<ItemData> loadItem(const std::string& itemId);
    bool deleteItem(const std::string& itemId);
    std::vector<ItemData> getAllItems();
    std::vector<ItemData> getItemsByType(const std::string& type);
    std::vector<ItemData> getItemsByRarity(const std::string& rarity);
    
    // 关卡数据
    bool saveLevel(const LevelData& level);
    std::optional<LevelData> loadLevel(const std::string& levelId);
    bool deleteLevel(const std::string& levelId);
    std::vector<LevelData> getAllLevels();
    std::vector<LevelData> getLevelsByDifficulty(double minDifficulty, double maxDifficulty);
    
    // 配置数据
    bool saveConfig(const std::string& key, const DatabaseValue& value);
    std::optional<DatabaseValue> loadConfig(const std::string& key);
    bool deleteConfig(const std::string& key);
    std::vector<std::string> getAllConfigKeys();
    
    // 统计数据
    bool saveStatistic(const std::string& key, const DatabaseValue& value);
    std::optional<DatabaseValue> loadStatistic(const std::string& key);
    bool deleteStatistic(const std::string& key);
    std::vector<std::string> getAllStatKeys();
    
    // 日志数据
    bool logEvent(const std::string& eventType, const std::string& message, const std::unordered_map<std::string, DatabaseValue>& data = {});
    std::vector<DatabaseRow> getLogs(const std::string& eventType, std::chrono::system_clock::time_point startTime, std::chrono::system_clock::time_point endTime);
    bool clearLogs(std::chrono::system_clock::time_point before);
    
    // 缓存操作
    bool setCache(const std::string& key, const DatabaseValue& value, std::chrono::seconds ttl = std::chrono::seconds(3600));
    std::optional<DatabaseValue> getCache(const std::string& key);
    bool deleteCache(const std::string& key);
    bool clearCache();
    
    // 批量操作
    bool insertBatch(const std::string& tableName, const std::vector<std::unordered_map<std::string, DatabaseValue>>& data, const std::string& connectionName = "");
    bool updateBatch(const std::string& tableName, const std::vector<std::pair<std::unordered_map<std::string, DatabaseValue>, std::string>>& data, const std::string& connectionName = "");
    
    // 统计信息
    DatabaseManagerStats getStats() const;
    void resetStats();
    DatabaseManagerConfig getConfig() const;
    
    // 事件回调
    void setConnectionStateCallback(std::function<void(const std::string&, ConnectionState)> callback);
    void setQueryCallback(std::function<void(const std::string&, const DatabaseResult&)> callback);
    void setErrorCallback(std::function<void(const std::string&, const std::string&)> callback);
    
    // 工具方法
    std::string getLastError() const;
    bool ping(const std::string& connectionName);
    std::vector<std::string> getTableNames(const std::string& connectionName = "");
    bool tableExists(const std::string& tableName, const std::string& connectionName = "");
    
private:
    DatabaseManager();
    ~DatabaseManager();
    
    // 内部实现
    class Impl;
    std::unique_ptr<Impl> pImpl_;
    
    // 私有方法
    bool initializeTables();
    bool createCharacterTable();
    bool createSaveDataTable();
    bool createQuestTable();
    bool createItemTable();
    bool createLevelTable();
    bool createConfigTable();
    bool createStatisticTable();
    bool createLogTable();
    bool createCacheTable();
    
    void healthCheckLoop();
    void autoBackupLoop();
    void cleanupLoop();
    
    std::string selectConnection(DatabaseRole role) const;
    bool reconnect(const std::string& connectionName);
    void logQuery(const std::string& connectionName, const std::string& sql, const DatabaseResult& result);
    void handleError(const std::string& connectionName, const std::string& error);
};

// 数据库管理器工具类
class DatabaseManagerUtils {
public:
    // 数据转换
    static std::unordered_map<std::string, DatabaseValue> characterToMap(const CharacterData& character);
    static CharacterData mapToCharacter(const std::unordered_map<std::string, DatabaseValue>& map);
    
    static std::unordered_map<std::string, DatabaseValue> saveDataToMap(const GameSaveData& saveData);
    static GameSaveData mapToSaveData(const std::unordered_map<std::string, DatabaseValue>& map);
    
    static std::unordered_map<std::string, DatabaseValue> questToMap(const QuestData& quest);
    static QuestData mapToQuest(const std::unordered_map<std::string, DatabaseValue>& map);
    
    static std::unordered_map<std::string, DatabaseValue> itemToMap(const ItemData& item);
    static ItemData mapToItem(const std::unordered_map<std::string, DatabaseValue>& map);
    
    static std::unordered_map<std::string, DatabaseValue> levelToMap(const LevelData& level);
    static LevelData mapToLevel(const std::unordered_map<std::string, DatabaseValue>& map);
    
    // 时间转换
    static std::string timeToString(std::chrono::system_clock::time_point time);
    static std::chrono::system_clock::time_point stringToTime(const std::string& timeStr);
    
    // 验证
    static bool isValidCharacterId(const std::string& characterId);
    static bool isValidSaveSlot(const std::string& saveSlot);
    static bool isValidQuestId(const std::string& questId);
    static bool isValidItemId(const std::string& itemId);
    static bool isValidLevelId(const std::string& levelId);
};

// 数据库管理器日志宏
#define DB_MGR_LOG_TRACE(msg, ...) \
    FANTASY_LOG_TRACE("[DatabaseManager] " msg, ##__VA_ARGS__)

#define DB_MGR_LOG_DEBUG(msg, ...) \
    FANTASY_LOG_DEBUG("[DatabaseManager] " msg, ##__VA_ARGS__)

#define DB_MGR_LOG_INFO(msg, ...) \
    FANTASY_LOG_INFO("[DatabaseManager] " msg, ##__VA_ARGS__)

#define DB_MGR_LOG_WARN(msg, ...) \
    FANTASY_LOG_WARN("[DatabaseManager] " msg, ##__VA_ARGS__)

#define DB_MGR_LOG_ERROR(msg, ...) \
    FANTASY_LOG_ERROR("[DatabaseManager] " msg, ##__VA_ARGS__)

} // namespace Fantasy 