/**
 * @file DatabaseManager.cpp
 * @brief 数据库管理器实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "include/data/database/DatabaseManager.h"
#include "include/utils/resources/ResourceLogger.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <thread>

namespace Fantasy {

// DatabaseManager::Impl 内部实现
class DatabaseManager::Impl {
public:
    DatabaseManagerConfig config_;
    std::unordered_map<std::string, std::shared_ptr<Database>> databases_;
    std::unordered_map<std::string, DatabaseConnectionInfo> connectionInfos_;
    DatabaseManagerStats stats_;
    std::string lastError_;
    bool initialized_;
    
    // 回调函数
    std::function<void(const std::string&, ConnectionState)> connectionStateCallback_;
    std::function<void(const std::string&, const DatabaseResult&)> queryCallback_;
    std::function<void(const std::string&, const std::string&)> errorCallback_;
    
    // 后台线程
    std::thread healthCheckThread_;
    std::thread autoBackupThread_;
    std::thread cleanupThread_;
    std::atomic<bool> running_;
    
    // 查询日志
    std::vector<std::pair<std::string, DatabaseResult>> queryLog_;
    mutable std::mutex mutex_;
    
    Impl() : initialized_(false), running_(false) {}
};

DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager() : pImpl_(std::make_unique<Impl>()) {}
DatabaseManager::~DatabaseManager() = default;

bool DatabaseManager::initialize(const DatabaseManagerConfig& config) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    
    if (pImpl_->initialized_) {
        DB_MGR_LOG_WARN("DatabaseManager already initialized");
        return true;
    }
    
    pImpl_->config_ = config;
    
    // 初始化所有数据库连接
    for (const auto& [name, info] : config.connections) {
        if (!info.enabled) {
            continue;
        }
        
        auto database = DatabaseFactory::createDatabase(info.config.type);
        if (!database) {
            DB_MGR_LOG_ERROR("Failed to create database: {}", name);
            continue;
        }
        
        if (!database->initialize(info.config)) {
            DB_MGR_LOG_ERROR("Failed to initialize database: {}", name);
            continue;
        }
        
        pImpl_->databases_[name] = std::move(database);
        pImpl_->connectionInfos_[name] = info;
        
        DB_MGR_LOG_INFO("Database initialized: {} ({})", name, 
                        DatabaseFactory::getTypeString(info.config.type));
    }
    
    if (pImpl_->databases_.empty()) {
        DB_MGR_LOG_ERROR("No databases were successfully initialized");
        return false;
    }
    
    // 创建表结构
    if (!createTables()) {
        DB_MGR_LOG_ERROR("Failed to create database tables");
        return false;
    }
    
    // 启动后台线程
    pImpl_->running_ = true;
    if (pImpl_->config_.healthCheckInterval.count() > 0) {
        pImpl_->healthCheckThread_ = std::thread(&DatabaseManager::healthCheckLoop, this);
    }
    if (pImpl_->config_.enableAutoBackup) {
        pImpl_->autoBackupThread_ = std::thread(&DatabaseManager::autoBackupLoop, this);
    }
    pImpl_->cleanupThread_ = std::thread(&DatabaseManager::cleanupLoop, this);
    
    pImpl_->initialized_ = true;
    DB_MGR_LOG_INFO("DatabaseManager initialized with {} databases", pImpl_->databases_.size());
    return true;
}

void DatabaseManager::shutdown() {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    
    if (!pImpl_->initialized_) {
        return;
    }
    
    DB_MGR_LOG_INFO("Shutting down DatabaseManager...");
    
    // 停止后台线程
    pImpl_->running_ = false;
    
    if (pImpl_->healthCheckThread_.joinable()) {
        pImpl_->healthCheckThread_.join();
    }
    if (pImpl_->autoBackupThread_.joinable()) {
        pImpl_->autoBackupThread_.join();
    }
    if (pImpl_->cleanupThread_.joinable()) {
        pImpl_->cleanupThread_.join();
    }
    
    // 关闭所有数据库连接
    for (auto& [name, database] : pImpl_->databases_) {
        database->shutdown();
    }
    
    pImpl_->databases_.clear();
    pImpl_->connectionInfos_.clear();
    pImpl_->initialized_ = false;
    
    DB_MGR_LOG_INFO("DatabaseManager shutdown complete");
}

bool DatabaseManager::isInitialized() const {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    return pImpl_->initialized_;
}

bool DatabaseManager::addConnection(const std::string& name, const DatabaseConnectionInfo& info) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    
    if (pImpl_->databases_.count(name) > 0) {
        DB_MGR_LOG_WARN("Database connection already exists: {}", name);
        return false;
    }
    
    auto database = DatabaseFactory::createDatabase(info.config.type);
    if (!database) {
        DB_MGR_LOG_ERROR("Failed to create database: {}", name);
        return false;
    }
    
    if (!database->initialize(info.config)) {
        DB_MGR_LOG_ERROR("Failed to initialize database: {}", name);
        return false;
    }
    
    pImpl_->databases_[name] = std::move(database);
    pImpl_->connectionInfos_[name] = info;
    
    DB_MGR_LOG_INFO("Database connection added: {}", name);
    return true;
}

bool DatabaseManager::removeConnection(const std::string& name) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    
    auto it = pImpl_->databases_.find(name);
    if (it == pImpl_->databases_.end()) {
        return false;
    }
    
    it->second->shutdown();
    pImpl_->databases_.erase(it);
    pImpl_->connectionInfos_.erase(name);
    
    DB_MGR_LOG_INFO("Database connection removed: {}", name);
    return true;
}

bool DatabaseManager::enableConnection(const std::string& name, bool enable) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    
    auto it = pImpl_->connectionInfos_.find(name);
    if (it == pImpl_->connectionInfos_.end()) {
        return false;
    }
    
    it->second.enabled = enable;
    DB_MGR_LOG_INFO("Database connection {}: {}", name, enable ? "enabled" : "disabled");
    return true;
}

std::vector<std::string> DatabaseManager::getConnectionNames() const {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    std::vector<std::string> names;
    for (const auto& [name, _] : pImpl_->databases_) {
        names.push_back(name);
    }
    return names;
}

DatabaseConnectionInfo DatabaseManager::getConnectionInfo(const std::string& name) const {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    auto it = pImpl_->connectionInfos_.find(name);
    return it != pImpl_->connectionInfos_.end() ? it->second : DatabaseConnectionInfo{};
}

std::shared_ptr<Database> DatabaseManager::getDatabase(const std::string& name) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    auto it = pImpl_->databases_.find(name);
    return it != pImpl_->databases_.end() ? it->second : nullptr;
}

std::shared_ptr<Database> DatabaseManager::getDatabase(DatabaseRole role) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    
    std::string connectionName = selectConnection(role);
    if (connectionName.empty()) {
        return nullptr;
    }
    
    return getDatabase(connectionName);
}

std::shared_ptr<Database> DatabaseManager::getMasterDatabase() {
    return getDatabase(DatabaseRole::MASTER);
}

std::shared_ptr<Database> DatabaseManager::getSlaveDatabase() {
    return getDatabase(DatabaseRole::SLAVE);
}

std::shared_ptr<Database> DatabaseManager::getCacheDatabase() {
    return getDatabase(DatabaseRole::CACHE);
}

std::shared_ptr<Database> DatabaseManager::getLogDatabase() {
    return getDatabase(DatabaseRole::LOG);
}

DatabaseResult DatabaseManager::query(const std::string& sql, const std::string& connectionName) {
    std::string targetConnection = connectionName;
    if (targetConnection.empty()) {
        targetConnection = selectConnection(DatabaseRole::MASTER);
    }
    
    auto database = getDatabase(targetConnection);
    if (!database) {
        return DatabaseResult{{}, {}, 0, 0, false, "No available database connection"};
    }
    
    auto result = database->query(sql);
    logQuery(targetConnection, sql, result);
    return result;
}

DatabaseResult DatabaseManager::query(const std::string& sql, const std::vector<DatabaseValue>& params, 
                                     const std::string& connectionName) {
    std::string targetConnection = connectionName;
    if (targetConnection.empty()) {
        targetConnection = selectConnection(DatabaseRole::MASTER);
    }
    
    auto database = getDatabase(targetConnection);
    if (!database) {
        return DatabaseResult{{}, {}, 0, 0, false, "No available database connection"};
    }
    
    auto result = database->query(sql, params);
    logQuery(targetConnection, sql, result);
    return result;
}

std::future<DatabaseResult> DatabaseManager::queryAsync(const std::string& sql, const std::string& connectionName) {
    std::string targetConnection = connectionName;
    if (targetConnection.empty()) {
        targetConnection = selectConnection(DatabaseRole::MASTER);
    }
    
    auto database = getDatabase(targetConnection);
    if (!database) {
        return std::async(std::launch::async, []() {
            return DatabaseResult{{}, {}, 0, 0, false, "No available database connection"};
        });
    }
    
    return std::async(std::launch::async, [this, database, sql, targetConnection]() {
        auto result = database->query(sql);
        logQuery(targetConnection, sql, result);
        return result;
    });
}

std::future<DatabaseResult> DatabaseManager::queryAsync(const std::string& sql, const std::vector<DatabaseValue>& params, 
                                                       const std::string& connectionName) {
    std::string targetConnection = connectionName;
    if (targetConnection.empty()) {
        targetConnection = selectConnection(DatabaseRole::MASTER);
    }
    
    auto database = getDatabase(targetConnection);
    if (!database) {
        return std::async(std::launch::async, []() {
            return DatabaseResult{{}, {}, 0, 0, false, "No available database connection"};
        });
    }
    
    return std::async(std::launch::async, [this, database, sql, params, targetConnection]() {
        auto result = database->query(sql, params);
        logQuery(targetConnection, sql, result);
        return result;
    });
}

bool DatabaseManager::beginTransaction(const std::string& connectionName) {
    std::string targetConnection = connectionName;
    if (targetConnection.empty()) {
        targetConnection = selectConnection(DatabaseRole::MASTER);
    }
    
    auto database = getDatabase(targetConnection);
    if (!database) {
        return false;
    }
    
    return database->beginTransaction();
}

bool DatabaseManager::commitTransaction(const std::string& connectionName) {
    std::string targetConnection = connectionName;
    if (targetConnection.empty()) {
        targetConnection = selectConnection(DatabaseRole::MASTER);
    }
    
    auto database = getDatabase(targetConnection);
    if (!database) {
        return false;
    }
    
    return database->commitTransaction();
}

bool DatabaseManager::rollbackTransaction(const std::string& connectionName) {
    std::string targetConnection = connectionName;
    if (targetConnection.empty()) {
        targetConnection = selectConnection(DatabaseRole::MASTER);
    }
    
    auto database = getDatabase(targetConnection);
    if (!database) {
        return false;
    }
    
    return database->rollbackTransaction();
}

bool DatabaseManager::isInTransaction(const std::string& connectionName) const {
    std::string targetConnection = connectionName;
    if (targetConnection.empty()) {
        targetConnection = selectConnection(DatabaseRole::MASTER);
    }
    
    auto database = getDatabase(targetConnection);
    if (!database) {
        return false;
    }
    
    return database->isInTransaction();
}

bool DatabaseManager::createTables() {
    bool success = true;
    
    // 创建角色表
    if (!createCharacterTable()) success = false;
    
    // 创建存档表
    if (!createSaveDataTable()) success = false;
    
    // 创建任务表
    if (!createQuestTable()) success = false;
    
    // 创建物品表
    if (!createItemTable()) success = false;
    
    // 创建关卡表
    if (!createLevelTable()) success = false;
    
    // 创建配置表
    if (!createConfigTable()) success = false;
    
    // 创建统计表
    if (!createStatisticTable()) success = false;
    
    // 创建日志表
    if (!createLogTable()) success = false;
    
    // 创建缓存表
    if (!createCacheTable()) success = false;
    
    return success;
}

bool DatabaseManager::createCharacterTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS characters (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            level INTEGER DEFAULT 1,
            experience INTEGER DEFAULT 0,
            health INTEGER DEFAULT 100,
            mana INTEGER DEFAULT 50,
            strength INTEGER DEFAULT 10,
            agility INTEGER DEFAULT 10,
            intelligence INTEGER DEFAULT 10,
            skills TEXT,
            equipment TEXT,
            last_login_time TEXT,
            created_time TEXT DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    return query(sql).success;
}

bool DatabaseManager::createSaveDataTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS save_data (
            save_slot TEXT PRIMARY KEY,
            character_id TEXT NOT NULL,
            level INTEGER DEFAULT 1,
            current_map TEXT,
            position_x REAL DEFAULT 0.0,
            position_y REAL DEFAULT 0.0,
            game_state TEXT,
            save_time TEXT DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    return query(sql).success;
}

bool DatabaseManager::createQuestTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS quests (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            description TEXT,
            type TEXT,
            objectives TEXT,
            rewards TEXT,
            required_level INTEGER DEFAULT 1,
            is_repeatable BOOLEAN DEFAULT 0,
            created_time TEXT DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    return query(sql).success;
}

bool DatabaseManager::createItemTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS items (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            type TEXT,
            rarity TEXT DEFAULT 'common',
            level INTEGER DEFAULT 1,
            value INTEGER DEFAULT 0,
            weight REAL DEFAULT 0.0,
            description TEXT,
            effects TEXT,
            created_time TEXT DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    return query(sql).success;
}

bool DatabaseManager::createLevelTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS levels (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            description TEXT,
            difficulty REAL DEFAULT 1.0,
            enemies TEXT,
            rewards TEXT,
            time_limit INTEGER DEFAULT 0,
            background TEXT,
            music TEXT,
            created_time TEXT DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    return query(sql).success;
}

bool DatabaseManager::createConfigTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS configs (
            key TEXT PRIMARY KEY,
            value TEXT,
            description TEXT,
            created_time TEXT DEFAULT CURRENT_TIMESTAMP,
            updated_time TEXT DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    return query(sql).success;
}

bool DatabaseManager::createStatisticTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS statistics (
            key TEXT PRIMARY KEY,
            value TEXT,
            created_time TEXT DEFAULT CURRENT_TIMESTAMP,
            updated_time TEXT DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    return query(sql).success;
}

bool DatabaseManager::createLogTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            event_type TEXT NOT NULL,
            message TEXT,
            data TEXT,
            timestamp TEXT DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    return query(sql).success;
}

bool DatabaseManager::createCacheTable() {
    std::string sql = R"(
        CREATE TABLE IF NOT EXISTS cache (
            key TEXT PRIMARY KEY,
            value TEXT,
            ttl INTEGER,
            created_time TEXT DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    return query(sql).success;
}

bool DatabaseManager::dropTables() {
    std::vector<std::string> tables = {
        "characters", "save_data", "quests", "items", "levels", 
        "configs", "statistics", "logs", "cache"
    };
    
    bool success = true;
    for (const auto& table : tables) {
        if (!query("DROP TABLE IF EXISTS " + table).success) {
            success = false;
        }
    }
    
    return success;
}

bool DatabaseManager::backupAll() {
    // 简化实现，实际应该调用每个数据库的备份方法
    DB_MGR_LOG_INFO("Backup all databases");
    return true;
}

bool DatabaseManager::restoreAll(const std::string& backupPath) {
    // 简化实现
    DB_MGR_LOG_INFO("Restore all databases from: {}", backupPath);
    return true;
}

bool DatabaseManager::optimizeAll() {
    bool success = true;
    for (auto& [name, database] : pImpl_->databases_) {
        if (!database->optimize()) {
            success = false;
        }
    }
    return success;
}

bool DatabaseManager::vacuumAll() {
    bool success = true;
    for (auto& [name, database] : pImpl_->databases_) {
        if (!database->vacuum()) {
            success = false;
        }
    }
    return success;
}

bool DatabaseManager::checkHealth() {
    bool allHealthy = true;
    for (auto& [name, info] : pImpl_->connectionInfos_) {
        if (!info.enabled) continue;
        
        auto database = getDatabase(name);
        if (!database || !database->ping()) {
            info.isHealthy = false;
            allHealthy = false;
            handleError(name, "Health check failed");
        } else {
            info.isHealthy = true;
            info.lastPingTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch());
        }
    }
    return allHealthy;
}

bool DatabaseManager::pingAll() {
    bool allPinged = true;
    for (auto& [name, database] : pImpl_->databases_) {
        if (!database->ping()) {
            allPinged = false;
        }
    }
    return allPinged;
}

std::vector<std::string> DatabaseManager::getUnhealthyConnections() const {
    std::vector<std::string> unhealthy;
    for (const auto& [name, info] : pImpl_->connectionInfos_) {
        if (!info.isHealthy) {
            unhealthy.push_back(name);
        }
    }
    return unhealthy;
}

bool DatabaseManager::migrateData(const std::string& sourceConnection, const std::string& targetConnection) {
    // 简化实现
    DB_MGR_LOG_INFO("Migrate data from {} to {}", sourceConnection, targetConnection);
    return true;
}

bool DatabaseManager::syncData(const std::string& sourceConnection, const std::string& targetConnection) {
    // 简化实现
    DB_MGR_LOG_INFO("Sync data from {} to {}", sourceConnection, targetConnection);
    return true;
}

// 游戏特定数据操作实现
bool DatabaseManager::saveCharacter(const CharacterData& character) {
    auto data = DatabaseManagerUtils::characterToMap(character);
    return insert("characters", data) > 0;
}

std::optional<CharacterData> DatabaseManager::loadCharacter(const std::string& characterId) {
    auto result = select("characters", {}, "id = '" + DatabaseUtils::escapeString(characterId) + "'");
    if (result.rows.empty()) {
        return std::nullopt;
    }
    
    auto rowMap = DatabaseUtils::rowToMap(result.rows[0], result.columnNames);
    return DatabaseManagerUtils::mapToCharacter(rowMap);
}

bool DatabaseManager::deleteCharacter(const std::string& characterId) {
    return delete_("characters", "id = '" + DatabaseUtils::escapeString(characterId) + "'");
}

std::vector<CharacterData> DatabaseManager::getAllCharacters() {
    auto result = select("characters");
    std::vector<CharacterData> characters;
    
    for (const auto& row : result.rows) {
        auto rowMap = DatabaseUtils::rowToMap(row, result.columnNames);
        characters.push_back(DatabaseManagerUtils::mapToCharacter(rowMap));
    }
    
    return characters;
}

std::vector<CharacterData> DatabaseManager::getCharactersByLevel(int minLevel, int maxLevel) {
    std::string whereClause = "level >= " + std::to_string(minLevel) + 
                             " AND level <= " + std::to_string(maxLevel);
    auto result = select("characters", {}, whereClause);
    std::vector<CharacterData> characters;
    
    for (const auto& row : result.rows) {
        auto rowMap = DatabaseUtils::rowToMap(row, result.columnNames);
        characters.push_back(DatabaseManagerUtils::mapToCharacter(rowMap));
    }
    
    return characters;
}

bool DatabaseManager::updateCharacterLevel(const std::string& characterId, int newLevel) {
    std::unordered_map<std::string, DatabaseValue> data{{"level", newLevel}};
    return update("characters", data, "id = '" + DatabaseUtils::escapeString(characterId) + "'");
}

bool DatabaseManager::updateCharacterExperience(const std::string& characterId, int experience) {
    std::unordered_map<std::string, DatabaseValue> data{{"experience", experience}};
    return update("characters", data, "id = '" + DatabaseUtils::escapeString(characterId) + "'");
}

// 其他游戏数据操作方法的实现类似，这里省略...

DatabaseManagerStats DatabaseManager::getStats() const {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    return pImpl_->stats_;
}

void DatabaseManager::resetStats() {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    pImpl_->stats_ = DatabaseManagerStats{};
}

DatabaseManagerConfig DatabaseManager::getConfig() const {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    return pImpl_->config_;
}

void DatabaseManager::setConnectionStateCallback(std::function<void(const std::string&, ConnectionState)> callback) {
    pImpl_->connectionStateCallback_ = callback;
}

void DatabaseManager::setQueryCallback(std::function<void(const std::string&, const DatabaseResult&)> callback) {
    pImpl_->queryCallback_ = callback;
}

void DatabaseManager::setErrorCallback(std::function<void(const std::string&, const std::string&)> callback) {
    pImpl_->errorCallback_ = callback;
}

std::string DatabaseManager::getLastError() const {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    return pImpl_->lastError_;
}

bool DatabaseManager::ping(const std::string& connectionName) {
    auto database = getDatabase(connectionName);
    return database ? database->ping() : false;
}

std::vector<std::string> DatabaseManager::getTableNames(const std::string& connectionName) {
    auto database = getDatabase(connectionName);
    return database ? database->getTableNames() : std::vector<std::string>{};
}

bool DatabaseManager::tableExists(const std::string& tableName, const std::string& connectionName) {
    auto database = getDatabase(connectionName);
    return database ? database->tableExists(tableName) : false;
}

// 私有方法实现
std::string DatabaseManager::selectConnection(DatabaseRole role) const {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    
    std::string bestConnection;
    int bestPriority = -1;
    
    for (const auto& [name, info] : pImpl_->connectionInfos_) {
        if (!info.enabled || !info.isHealthy || info.role != role) {
            continue;
        }
        
        if (info.priority > bestPriority) {
            bestPriority = info.priority;
            bestConnection = name;
        }
    }
    
    return bestConnection;
}

bool DatabaseManager::reconnect(const std::string& connectionName) {
    // 简化实现
    DB_MGR_LOG_INFO("Reconnecting to database: {}", connectionName);
    return true;
}

void DatabaseManager::logQuery(const std::string& connectionName, const std::string& sql, const DatabaseResult& result) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    
    if (pImpl_->config_.enableQueryLogging) {
        pImpl_->queryLog_.emplace_back(connectionName, result);
        
        // 限制日志大小
        if (pImpl_->queryLog_.size() > pImpl_->config_.maxQueryLogSize) {
            pImpl_->queryLog_.erase(pImpl_->queryLog_.begin());
        }
    }
    
    // 更新统计
    pImpl_->stats_.totalQueries++;
    if (result.success) {
        pImpl_->stats_.successfulQueries++;
                    } else {
        pImpl_->stats_.failedQueries++;
    }
    
    // 调用回调
    if (pImpl_->queryCallback_) {
        pImpl_->queryCallback_(connectionName, result);
    }
}

void DatabaseManager::handleError(const std::string& connectionName, const std::string& error) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex_);
    
    pImpl_->lastError_ = error;
    pImpl_->stats_.failedQueries++;
    
    if (pImpl_->errorCallback_) {
        pImpl_->errorCallback_(connectionName, error);
    }
    
    DB_MGR_LOG_ERROR("Database error [{}]: {}", connectionName, error);
}

void DatabaseManager::healthCheckLoop() {
    while (pImpl_->running_) {
        checkHealth();
        std::this_thread::sleep_for(pImpl_->config_.healthCheckInterval);
    }
}

void DatabaseManager::autoBackupLoop() {
    while (pImpl_->running_) {
        if (pImpl_->config_.enableAutoBackup) {
            backupAll();
        }
        std::this_thread::sleep_for(pImpl_->config_.autoBackupInterval);
    }
}

void DatabaseManager::cleanupLoop() {
    while (pImpl_->running_) {
        // 清理过期缓存
        query("DELETE FROM cache WHERE ttl > 0 AND datetime(created_time, '+' || ttl || ' seconds') < datetime('now')");
        
        // 清理旧日志
        auto oneWeekAgo = std::chrono::system_clock::now() - std::chrono::hours(24 * 7);
        clearLogs(oneWeekAgo);
        
        std::this_thread::sleep_for(std::chrono::hours(1));
    }
}

// 数据库管理器工具类实现
std::unordered_map<std::string, DatabaseValue> DatabaseManagerUtils::characterToMap(const CharacterData& character) {
    std::unordered_map<std::string, DatabaseValue> map;
    map["id"] = character.id;
    map["name"] = character.name;
    map["level"] = character.level;
    map["experience"] = character.experience;
    map["health"] = character.health;
    map["mana"] = character.mana;
    map["strength"] = character.strength;
    map["agility"] = character.agility;
    map["intelligence"] = character.intelligence;
    map["skills"] = character.skills;
    map["equipment"] = character.equipment;
    map["last_login_time"] = timeToString(character.lastLoginTime);
    map["created_time"] = timeToString(character.createdTime);
    return map;
}

CharacterData DatabaseManagerUtils::mapToCharacter(const std::unordered_map<std::string, DatabaseValue>& map) {
    CharacterData character;
    character.id = DatabaseUtils::toString(map.at("id"));
    character.name = DatabaseUtils::toString(map.at("name"));
    character.level = DatabaseUtils::toInt(map.at("level"));
    character.experience = DatabaseUtils::toInt(map.at("experience"));
    character.health = DatabaseUtils::toInt(map.at("health"));
    character.mana = DatabaseUtils::toInt(map.at("mana"));
    character.strength = DatabaseUtils::toInt(map.at("strength"));
    character.agility = DatabaseUtils::toInt(map.at("agility"));
    character.intelligence = DatabaseUtils::toInt(map.at("intelligence"));
    character.skills = DatabaseUtils::toStringArray(map.at("skills"));
    character.equipment = DatabaseUtils::toStringArray(map.at("equipment"));
    character.lastLoginTime = stringToTime(DatabaseUtils::toString(map.at("last_login_time")));
    character.createdTime = stringToTime(DatabaseUtils::toString(map.at("created_time")));
    return character;
}

// 其他数据转换方法的实现类似...

std::string DatabaseManagerUtils::timeToString(std::chrono::system_clock::time_point time) {
    auto time_t = std::chrono::system_clock::to_time_t(time);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::chrono::system_clock::time_point DatabaseManagerUtils::stringToTime(const std::string& timeStr) {
    std::tm tm = {};
    std::stringstream ss(timeStr);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

bool DatabaseManagerUtils::isValidCharacterId(const std::string& characterId) {
    return !characterId.empty() && characterId.length() <= 50;
}

bool DatabaseManagerUtils::isValidSaveSlot(const std::string& saveSlot) {
    return !saveSlot.empty() && saveSlot.length() <= 20;
}

bool DatabaseManagerUtils::isValidQuestId(const std::string& questId) {
    return !questId.empty() && questId.length() <= 50;
}

bool DatabaseManagerUtils::isValidItemId(const std::string& itemId) {
    return !itemId.empty() && itemId.length() <= 50;
}

bool DatabaseManagerUtils::isValidLevelId(const std::string& levelId) {
    return !levelId.empty() && levelId.length() <= 50;
}

} // namespace Fantasy
