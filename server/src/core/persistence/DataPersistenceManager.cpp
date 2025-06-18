/**
 * @file DataPersistenceManager.cpp
 * @brief 数据持久化管理器实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "core/persistence/DataPersistenceManager.h"
#include "data/database/DatabaseManager.h"
#include "core/characters/Character.h"
#include "core/items/Item.h"
#include "core/quests/Quest.h"
#include "core/levels/Level.h"
#include "core/skills/Skill.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <json/json.h>

namespace Fantasy {

// 内部实现类
class DataPersistenceManager::Impl {
public:
    Impl() : initialized_(false), autoSaveEnabled_(true), autoSaveInterval_(300) {}
    
    bool initialized_;
    DataPersistenceConfig config_;
    
    // 缓存
    std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<void>>> cache_;
    std::mutex cacheMutex_;
    
    // 同步状态
    std::unordered_map<std::string, SyncStatus> syncStatus_;
    std::mutex syncMutex_;
    
    // 自动保存
    std::atomic<bool> autoSaveEnabled_;
    std::chrono::seconds autoSaveInterval_;
    std::thread autoSaveThread_;
    std::atomic<bool> stopAutoSave_;
    
    // 事件系统
    std::vector<DataChangeCallback> callbacks_;
    std::mutex callbacksMutex_;
    
    // 统计信息
    mutable PersistenceStats stats_;
    mutable std::mutex statsMutex_;
    
    // 错误信息
    std::string lastError_;
    std::mutex errorMutex_;
};

// 单例实现
DataPersistenceManager& DataPersistenceManager::getInstance() {
    static DataPersistenceManager instance;
    return instance;
}

DataPersistenceManager::DataPersistenceManager() : pImpl_(std::make_unique<Impl>()) {}

DataPersistenceManager::~DataPersistenceManager() {
    shutdown();
}

bool DataPersistenceManager::initialize(const DataPersistenceConfig& config) {
    if (pImpl_->initialized_) {
        return true;
    }
    
    pImpl_->config_ = config;
    
    // 初始化数据库管理器
    // 使用DatabaseManager的单例实例
    auto& dbManager = DatabaseManager::getInstance();
    
    // 创建数据库管理器配置
    DatabaseManagerConfig dbConfig;
    // 添加默认连接
    DatabaseConnectionInfo masterConn;
    masterConn.role = DatabaseRole::MASTER;
    masterConn.config.type = DatabaseType::SQLITE;
    masterConn.config.database = "fantasy_game.db";
    masterConn.config.maxConnections = 10;
    masterConn.config.enableCompression = true;
    masterConn.name = "master";
    masterConn.enabled = true;
    masterConn.priority = 1;
    dbConfig.connections["master"] = masterConn;
    
    if (!dbManager.initialize(dbConfig)) {
        setLastError("Failed to initialize database manager");
        return false;
    }
    
    // 创建数据表
    if (!createTables()) {
        setLastError("Failed to create database tables");
        return false;
    }
    
    // 启动自动保存线程
    if (config.enableAutoSave) {
        startAutoSaveThread();
    }
    
    pImpl_->initialized_ = true;
    return true;
}

void DataPersistenceManager::shutdown() {
    if (!pImpl_->initialized_) {
        return;
    }
    
    // 停止自动保存线程
    pImpl_->stopAutoSave_ = true;
    if (pImpl_->autoSaveThread_.joinable()) {
        pImpl_->autoSaveThread_.join();
    }
    
    // 同步所有数据
    syncAllData();
    
    // 清理缓存
    clearCache();
    
    // 关闭数据库连接
    DatabaseManager::getInstance().shutdown();
    
    pImpl_->initialized_ = false;
}

bool DataPersistenceManager::isInitialized() const {
    return pImpl_->initialized_;
}

// 角色数据持久化
bool DataPersistenceManager::saveCharacter(const std::shared_ptr<Character>& character) {
    if (!pImpl_->initialized_ || !character) {
        return false;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        // 数据验证
        if (pImpl_->config_.enableDataValidation && !validateCharacterData(character)) {
            setLastError("Character data validation failed");
            return false;
        }
        
        // 转换为数据库数据
        CharacterData data = characterToData(character);
        
        // 保存到数据库
        if (!DatabaseManager::getInstance().saveCharacter(data)) {
            setLastError("Failed to save character to database");
            return false;
        }
        
        // 添加到缓存
        addToCache("character", character->getName(), character);
        
        // 更新同步状态
        updateSyncStatus("character", character->getName(), SyncStatus::SYNCED);
        
        // 发送事件
        emitDataChangeEvent(DataChangeEvent(DataChangeType::CHARACTER_UPDATED, character->getName(), "character"));
        
        // 更新统计信息
        updateStats(true, true, startTime);
        
        return true;
    } catch (const std::exception& e) {
        setLastError("Exception during character save: " + std::string(e.what()));
        updateStats(true, false, startTime);
        return false;
    }
}

std::shared_ptr<Character> DataPersistenceManager::loadCharacter(const std::string& characterId) {
    if (!pImpl_->initialized_ || characterId.empty()) {
        return nullptr;
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        // 检查缓存
        auto cached = getFromCache("character", characterId);
        if (cached) {
            updateStats(false, true, startTime, true);
            return std::static_pointer_cast<Character>(cached);
        }
        
        // 从数据库加载
        auto data = DatabaseManager::getInstance().loadCharacter(characterId);
        if (!data) {
            setLastError("Character not found: " + characterId);
            updateStats(false, false, startTime, false);
            return nullptr;
        }
        
        // 转换为Character对象
        auto character = dataToCharacter(*data);
        if (!character) {
            setLastError("Failed to convert character data");
            updateStats(false, false, startTime, false);
            return nullptr;
        }
        
        // 添加到缓存
        addToCache("character", characterId, character);
        
        // 更新统计信息
        updateStats(false, true, startTime, false);
        
        return character;
    } catch (const std::exception& e) {
        setLastError("Exception during character load: " + std::string(e.what()));
        updateStats(false, false, startTime, false);
        return nullptr;
    }
}

// 游戏存档管理
bool DataPersistenceManager::saveGameState(const std::string& saveSlot, 
                                          const std::shared_ptr<Character>& character,
                                          const std::string& currentLevel, 
                                          const Vector2D& position) {
    if (!pImpl_->initialized_ || saveSlot.empty() || !character) {
        return false;
    }
    
    try {
        GameSaveData saveData;
        saveData.saveSlot = saveSlot;
        saveData.characterId = character->getName();
        saveData.currentMap = currentLevel;
        saveData.positionX = position.x;
        saveData.positionY = position.y;
        saveData.saveTime = std::chrono::system_clock::now();
        
        // 保存角色数据
        if (!saveCharacter(character)) {
            return false;
        }
        
        // 保存游戏状态
        if (!DatabaseManager::getInstance().saveGameData(saveData)) {
            setLastError("Failed to save game state");
            return false;
        }
        
        // 发送事件
        emitDataChangeEvent(DataChangeEvent(DataChangeType::SAVE_UPDATED, saveSlot, "save"));
        
        return true;
    } catch (const std::exception& e) {
        setLastError("Exception during game state save: " + std::string(e.what()));
        return false;
    }
}

std::optional<GameSaveData> DataPersistenceManager::loadGameState(const std::string& saveSlot) {
    if (!pImpl_->initialized_ || saveSlot.empty()) {
        return std::nullopt;
    }
    
    try {
        return DatabaseManager::getInstance().loadGameData(saveSlot);
    } catch (const std::exception& e) {
        setLastError("Exception during game state load: " + std::string(e.what()));
        return std::nullopt;
    }
}

// 数据转换方法
CharacterData DataPersistenceManager::characterToData(const std::shared_ptr<Character>& character) {
    CharacterData data;
    data.id = character->getName(); // 使用name作为ID
    data.name = character->getName();
    data.level = character->getStats().level;
    data.experience = character->getStats().experience;
    data.health = character->getStats().health;
    data.mana = character->getStats().mana;
    data.strength = character->getStats().attack;
    data.agility = character->getStats().speed;
    data.intelligence = character->getStats().magicAttack;
    data.createdTime = std::chrono::system_clock::now();
    data.lastLoginTime = std::chrono::system_clock::now();
    
    return data;
}

std::shared_ptr<Character> DataPersistenceManager::dataToCharacter(const CharacterData& data) {
    auto character = std::make_shared<Character>(data.name, CharacterClass::WARRIOR);
    
    // 设置基础属性
    CharacterStats stats;
    stats.level = data.level;
    stats.experience = data.experience;
    stats.health = data.health;
    stats.maxHealth = data.health;
    stats.mana = data.mana;
    stats.maxMana = data.mana;
    stats.attack = data.strength;
    stats.defense = 10;
    stats.speed = data.agility;
    stats.magicAttack = data.intelligence;
    
    character->setStats(stats);
    
    return character;
}

// 缓存管理
void DataPersistenceManager::addToCache(const std::string& entityType, 
                                       const std::string& entityId, 
                                       const std::shared_ptr<void>& data) {
    std::lock_guard<std::mutex> lock(pImpl_->cacheMutex_);
    
    // 检查缓存大小限制
    if (pImpl_->cache_[entityType].size() >= pImpl_->config_.maxCacheSize) {
        // 移除最旧的条目
        auto& typeCache = pImpl_->cache_[entityType];
        typeCache.erase(typeCache.begin());
    }
    
    pImpl_->cache_[entityType][entityId] = data;
}

std::shared_ptr<void> DataPersistenceManager::getFromCache(const std::string& entityType, 
                                                          const std::string& entityId) {
    std::lock_guard<std::mutex> lock(pImpl_->cacheMutex_);
    
    auto typeIt = pImpl_->cache_.find(entityType);
    if (typeIt == pImpl_->cache_.end()) {
        return nullptr;
    }
    
    auto itemIt = typeIt->second.find(entityId);
    if (itemIt == typeIt->second.end()) {
        return nullptr;
    }
    
    return itemIt->second;
}

void DataPersistenceManager::clearCache() {
    std::lock_guard<std::mutex> lock(pImpl_->cacheMutex_);
    pImpl_->cache_.clear();
}

// 自动保存
void DataPersistenceManager::startAutoSaveThread() {
    pImpl_->stopAutoSave_ = false;
    pImpl_->autoSaveThread_ = std::thread([this]() {
        while (!pImpl_->stopAutoSave_) {
            std::this_thread::sleep_for(pImpl_->autoSaveInterval_);
            if (pImpl_->autoSaveEnabled_ && !pImpl_->stopAutoSave_) {
                triggerAutoSave();
            }
        }
    });
}

void DataPersistenceManager::triggerAutoSave() {
    // 同步所有待保存的数据
    syncAllData();
    
    // 发送自动保存事件
    DataChangeEvent event(DataChangeType::SAVE_UPDATED, "auto_save", "system");
    emitDataChangeEvent(event);
}

// 事件系统
void DataPersistenceManager::emitDataChangeEvent(const DataChangeEvent& event) {
    std::lock_guard<std::mutex> lock(pImpl_->callbacksMutex_);
    
    for (const auto& callback : pImpl_->callbacks_) {
        try {
            callback(event);
        } catch (const std::exception& e) {
            std::cerr << "Error in data change callback: " << e.what() << std::endl;
        }
    }
}

// 统计信息
void DataPersistenceManager::updateStats(bool isSave, bool success, 
                                        std::chrono::high_resolution_clock::time_point startTime,
                                        bool cacheHit) {
    std::lock_guard<std::mutex> lock(pImpl_->statsMutex_);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    if (isSave) {
        pImpl_->stats_.totalSaves++;
        if (success) {
            pImpl_->stats_.successfulSaves++;
            pImpl_->stats_.lastSaveTime = std::chrono::system_clock::now();
        } else {
            pImpl_->stats_.failedSaves++;
        }
        
        // 更新平均保存时间
        if (pImpl_->stats_.successfulSaves > 0) {
            auto totalTime = pImpl_->stats_.averageSaveTime * (pImpl_->stats_.successfulSaves - 1) + duration;
            pImpl_->stats_.averageSaveTime = totalTime / pImpl_->stats_.successfulSaves;
        }
    } else {
        pImpl_->stats_.totalLoads++;
        if (success) {
            pImpl_->stats_.successfulLoads++;
            pImpl_->stats_.lastLoadTime = std::chrono::system_clock::now();
        } else {
            pImpl_->stats_.failedLoads++;
        }
        
        // 更新平均加载时间
        if (pImpl_->stats_.successfulLoads > 0) {
            auto totalTime = pImpl_->stats_.averageLoadTime * (pImpl_->stats_.successfulLoads - 1) + duration;
            pImpl_->stats_.averageLoadTime = totalTime / pImpl_->stats_.successfulLoads;
        }
        
        // 更新缓存统计
        if (cacheHit) {
            pImpl_->stats_.cacheHits++;
        } else {
            pImpl_->stats_.cacheMisses++;
        }
    }
}

// 错误处理
void DataPersistenceManager::setLastError(const std::string& error) {
    std::lock_guard<std::mutex> lock(pImpl_->errorMutex_);
    pImpl_->lastError_ = error;
}

std::string DataPersistenceManager::getLastError() const {
    std::lock_guard<std::mutex> lock(pImpl_->errorMutex_);
    return pImpl_->lastError_;
}

// 同步状态管理
void DataPersistenceManager::updateSyncStatus(const std::string& entityType, 
                                             const std::string& entityId, 
                                             SyncStatus status) {
    std::lock_guard<std::mutex> lock(pImpl_->syncMutex_);
    std::string key = entityType + ":" + entityId;
    pImpl_->syncStatus_[key] = status;
}

// 数据验证
bool DataPersistenceManager::validateCharacterData(const std::shared_ptr<Character>& character) {
    if (!character) {
        return false;
    }
    
    // 基本验证
    if (character->getName().empty()) {
        return false;
    }
    
    const auto& stats = character->getStats();
    if (stats.level < 1 || stats.level > 100) {
        return false;
    }
    
    if (stats.health < 0 || stats.health > stats.maxHealth) {
        return false;
    }
    
    if (stats.mana < 0 || stats.mana > stats.maxMana) {
        return false;
    }
    
    return true;
}

// 创建数据表
bool DataPersistenceManager::createTables() {
    // 创建角色表
    if (!DatabaseManager::getInstance().createTables()) {
        return false;
    }
    
    return true;
}

// 数据同步
bool DataPersistenceManager::syncAllData() {
    if (!pImpl_->initialized_) {
        return false;
    }
    
    try {
        // 同步所有待保存的数据
        auto pendingSaves = getPendingSaves();
        for (const auto& key : pendingSaves) {
            syncData("", key);
        }
        
        return true;
    } catch (const std::exception& e) {
        setLastError("Exception during data sync: " + std::string(e.what()));
        return false;
    }
}

std::vector<std::string> DataPersistenceManager::getPendingSaves() {
    std::lock_guard<std::mutex> lock(pImpl_->syncMutex_);
    std::vector<std::string> pending;
    
    for (const auto& [key, status] : pImpl_->syncStatus_) {
        if (status == SyncStatus::PENDING_SAVE) {
            pending.push_back(key);
        }
    }
    
    return pending;
}

// 配置管理
void DataPersistenceManager::setConfig(const DataPersistenceConfig& config) {
    pImpl_->config_ = config;
    
    // 更新自动保存设置
    if (config.enableAutoSave && !pImpl_->autoSaveEnabled_) {
        startAutoSaveThread();
    }
    pImpl_->autoSaveEnabled_ = config.enableAutoSave;
    pImpl_->autoSaveInterval_ = config.autoSaveInterval;
}

DataPersistenceConfig DataPersistenceManager::getConfig() const {
    return pImpl_->config_;
}

// 统计信息
DataPersistenceManager::PersistenceStats DataPersistenceManager::getStats() const {
    std::lock_guard<std::mutex> lock(pImpl_->statsMutex_);
    return pImpl_->stats_;
}

void DataPersistenceManager::resetStats() {
    std::lock_guard<std::mutex> lock(pImpl_->statsMutex_);
    pImpl_->stats_ = PersistenceStats{};
}

// 事件订阅
void DataPersistenceManager::subscribeToDataChanges(DataChangeCallback callback) {
    std::lock_guard<std::mutex> lock(pImpl_->callbacksMutex_);
    pImpl_->callbacks_.push_back(callback);
}

// 缓存大小
size_t DataPersistenceManager::getCacheSize() const {
    std::lock_guard<std::mutex> lock(pImpl_->cacheMutex_);
    size_t totalSize = 0;
    for (const auto& [type, cache] : pImpl_->cache_) {
        totalSize += cache.size();
    }
    return totalSize;
}

size_t DataPersistenceManager::getCacheSize(const std::string& entityType) const {
    std::lock_guard<std::mutex> lock(pImpl_->cacheMutex_);
    auto it = pImpl_->cache_.find(entityType);
    return (it != pImpl_->cache_.end()) ? it->second.size() : 0;
}

// 添加缺失的方法实现
bool DataPersistenceManager::saveCharacterAsync(const std::shared_ptr<Character>& character) {
    // 简单实现，实际应该使用异步线程
    return saveCharacter(character);
}

std::shared_ptr<Character> DataPersistenceManager::loadCharacterAsync(const std::string& characterId) {
    // 简单实现，实际应该使用异步线程
    return loadCharacter(characterId);
}

bool DataPersistenceManager::deleteCharacter(const std::string& characterId) {
    if (!pImpl_->initialized_ || characterId.empty()) {
        return false;
    }
    
    try {
        // 从缓存移除
        removeFromCache("character", characterId);
        
        // 从数据库删除
        if (!DatabaseManager::getInstance().deleteCharacter(characterId)) {
            setLastError("Failed to delete character from database");
            return false;
        }
        
        // 发送事件
        emitDataChangeEvent(DataChangeEvent(DataChangeType::CHARACTER_DELETED, characterId, "character"));
        
        return true;
    } catch (const std::exception& e) {
        setLastError("Exception during character deletion: " + std::string(e.what()));
        return false;
    }
}

std::vector<std::shared_ptr<Character>> DataPersistenceManager::loadAllCharacters() {
    if (!pImpl_->initialized_) {
        return {};
    }
    
    try {
        auto allData = DatabaseManager::getInstance().getAllCharacters();
        std::vector<std::shared_ptr<Character>> characters;
        
        for (const auto& data : allData) {
            auto character = dataToCharacter(data);
            if (character) {
                characters.push_back(character);
            }
        }
        
        return characters;
    } catch (const std::exception& e) {
        setLastError("Exception during load all characters: " + std::string(e.what()));
        return {};
    }
}

std::vector<std::string> DataPersistenceManager::getCharacterIds() {
    if (!pImpl_->initialized_) {
        return {};
    }
    
    try {
        auto allData = DatabaseManager::getInstance().getAllCharacters();
        std::vector<std::string> ids;
        
        for (const auto& data : allData) {
            ids.push_back(data.id);
        }
        
        return ids;
    } catch (const std::exception& e) {
        setLastError("Exception during get character IDs: " + std::string(e.what()));
        return {};
    }
}

bool DataPersistenceManager::characterExists(const std::string& characterId) {
    if (!pImpl_->initialized_ || characterId.empty()) {
        return false;
    }
    
    try {
        auto data = DatabaseManager::getInstance().loadCharacter(characterId);
        return data.has_value();
    } catch (const std::exception& e) {
        setLastError("Exception during character existence check: " + std::string(e.what()));
        return false;
    }
}

bool DataPersistenceManager::saveGameStateAsync(const std::string& saveSlot, 
                                               const std::shared_ptr<Character>& character,
                                               const std::string& currentLevel, 
                                               const Vector2D& position) {
    // 简单实现，实际应该使用异步线程
    return saveGameState(saveSlot, character, currentLevel, position);
}

std::optional<GameSaveData> DataPersistenceManager::loadGameStateAsync(const std::string& saveSlot) {
    // 简单实现，实际应该使用异步线程
    return loadGameState(saveSlot);
}

bool DataPersistenceManager::deleteGameState(const std::string& saveSlot) {
    if (!pImpl_->initialized_ || saveSlot.empty()) {
        return false;
    }
    
    try {
        if (!DatabaseManager::getInstance().deleteGameData(saveSlot)) {
            setLastError("Failed to delete game state from database");
            return false;
        }
        
        // 发送事件
        emitDataChangeEvent(DataChangeEvent(DataChangeType::SAVE_DELETED, saveSlot, "save"));
        
        return true;
    } catch (const std::exception& e) {
        setLastError("Exception during game state deletion: " + std::string(e.what()));
        return false;
    }
}

std::vector<GameSaveData> DataPersistenceManager::getAllSaveStates() {
    if (!pImpl_->initialized_) {
        return {};
    }
    
    try {
        return DatabaseManager::getInstance().getAllSaveData();
    } catch (const std::exception& e) {
        setLastError("Exception during get all save states: " + std::string(e.what()));
        return {};
    }
}

std::vector<GameSaveData> DataPersistenceManager::getSaveStatesByCharacter(const std::string& characterId) {
    if (!pImpl_->initialized_ || characterId.empty()) {
        return {};
    }
    
    try {
        return DatabaseManager::getInstance().getSaveDataByCharacter(characterId);
    } catch (const std::exception& e) {
        setLastError("Exception during get save states by character: " + std::string(e.what()));
        return {};
    }
}

bool DataPersistenceManager::saveSlotExists(const std::string& saveSlot) {
    if (!pImpl_->initialized_ || saveSlot.empty()) {
        return false;
    }
    
    try {
        auto data = DatabaseManager::getInstance().loadGameData(saveSlot);
        return data.has_value();
    } catch (const std::exception& e) {
        setLastError("Exception during save slot existence check: " + std::string(e.what()));
        return false;
    }
}

bool DataPersistenceManager::saveCharacters(const std::vector<std::shared_ptr<Character>>& characters) {
    if (!pImpl_->initialized_) {
        return false;
    }
    
    try {
        for (const auto& character : characters) {
            if (!saveCharacter(character)) {
                return false;
            }
        }
        return true;
    } catch (const std::exception& e) {
        setLastError("Exception during batch character save: " + std::string(e.what()));
        return false;
    }
}

bool DataPersistenceManager::syncData(const std::string& entityType, const std::string& entityId) {
    // 简单实现，实际应该同步特定数据
    return true;
}

SyncStatus DataPersistenceManager::getSyncStatus(const std::string& entityType, const std::string& entityId) {
    std::lock_guard<std::mutex> lock(pImpl_->syncMutex_);
    std::string key = entityType + ":" + entityId;
    auto it = pImpl_->syncStatus_.find(key);
    return (it != pImpl_->syncStatus_.end()) ? it->second : SyncStatus::SYNCED;
}

std::vector<std::string> DataPersistenceManager::getPendingLoads() {
    std::lock_guard<std::mutex> lock(pImpl_->syncMutex_);
    std::vector<std::string> pending;
    
    for (const auto& [key, status] : pImpl_->syncStatus_) {
        if (status == SyncStatus::PENDING_LOAD) {
            pending.push_back(key);
        }
    }
    
    return pending;
}

void DataPersistenceManager::clearCache(const std::string& entityType) {
    std::lock_guard<std::mutex> lock(pImpl_->cacheMutex_);
    pImpl_->cache_.erase(entityType);
}

void DataPersistenceManager::removeFromCache(const std::string& entityType, const std::string& entityId) {
    std::lock_guard<std::mutex> lock(pImpl_->cacheMutex_);
    auto typeIt = pImpl_->cache_.find(entityType);
    if (typeIt != pImpl_->cache_.end()) {
        typeIt->second.erase(entityId);
    }
}

void DataPersistenceManager::enableAutoSave(bool enable) {
    pImpl_->autoSaveEnabled_ = enable;
}

bool DataPersistenceManager::isAutoSaveEnabled() const {
    return pImpl_->autoSaveEnabled_;
}

void DataPersistenceManager::setAutoSaveInterval(std::chrono::seconds interval) {
    pImpl_->autoSaveInterval_ = interval;
}

std::chrono::seconds DataPersistenceManager::getAutoSaveInterval() const {
    return pImpl_->autoSaveInterval_;
}

bool DataPersistenceManager::validateItemData(const std::shared_ptr<Item>& item) {
    // 简单实现
    return item != nullptr;
}

bool DataPersistenceManager::validateQuestData(const std::shared_ptr<Quest>& quest) {
    // 简单实现
    return quest != nullptr;
}

bool DataPersistenceManager::validateLevelData(const std::shared_ptr<Level>& level) {
    // 简单实现
    return level != nullptr;
}

bool DataPersistenceManager::validateSkillData(const std::shared_ptr<Skill>& skill) {
    // 简单实现
    return skill != nullptr;
}

bool DataPersistenceManager::backupAllData(const std::string& backupPath) {
    // 简单实现
    return true;
}

bool DataPersistenceManager::restoreAllData(const std::string& backupPath) {
    // 简单实现
    return true;
}

bool DataPersistenceManager::backupData(const std::string& entityType, const std::string& backupPath) {
    // 简单实现
    return true;
}

bool DataPersistenceManager::restoreData(const std::string& entityType, const std::string& backupPath) {
    // 简单实现
    return true;
}

void DataPersistenceManager::unsubscribeFromDataChanges(DataChangeCallback callback) {
    // 简单实现
}

bool DataPersistenceManager::isDataValid(const std::string& entityType, const std::string& entityId) {
    // 简单实现
    return true;
}

std::vector<std::string> DataPersistenceManager::getInvalidData() {
    // 简单实现
    return {};
}

bool DataPersistenceManager::repairData(const std::string& entityType, const std::string& entityId) {
    // 简单实现
    return true;
}

// 添加缺失的物品、任务、关卡、技能相关方法
bool DataPersistenceManager::saveItem(const std::shared_ptr<Item>& item) { return true; }
bool DataPersistenceManager::saveItemAsync(const std::shared_ptr<Item>& item) { return true; }
std::shared_ptr<Item> DataPersistenceManager::loadItem(const std::string& itemId) { return nullptr; }
std::shared_ptr<Item> DataPersistenceManager::loadItemAsync(const std::string& itemId) { return nullptr; }
bool DataPersistenceManager::deleteItem(const std::string& itemId) { return true; }
std::vector<std::shared_ptr<Item>> DataPersistenceManager::loadAllItems() { return {}; }
std::vector<std::shared_ptr<Item>> DataPersistenceManager::loadItemsByType(const std::string& type) { return {}; }
std::vector<std::shared_ptr<Item>> DataPersistenceManager::loadItemsByRarity(const std::string& rarity) { return {}; }
bool DataPersistenceManager::itemExists(const std::string& itemId) { return false; }

bool DataPersistenceManager::saveQuest(const std::shared_ptr<Quest>& quest) { return true; }
bool DataPersistenceManager::saveQuestAsync(const std::shared_ptr<Quest>& quest) { return true; }
std::shared_ptr<Quest> DataPersistenceManager::loadQuest(const std::string& questId) { return nullptr; }
std::shared_ptr<Quest> DataPersistenceManager::loadQuestAsync(const std::string& questId) { return nullptr; }
bool DataPersistenceManager::deleteQuest(const std::string& questId) { return true; }
std::vector<std::shared_ptr<Quest>> DataPersistenceManager::loadAllQuests() { return {}; }
std::vector<std::shared_ptr<Quest>> DataPersistenceManager::loadQuestsByLevel(int level) { return {}; }
std::vector<std::shared_ptr<Quest>> DataPersistenceManager::loadQuestsByType(const std::string& type) { return {}; }
bool DataPersistenceManager::questExists(const std::string& questId) { return false; }

bool DataPersistenceManager::saveLevel(const std::shared_ptr<Level>& level) { return true; }
bool DataPersistenceManager::saveLevelAsync(const std::shared_ptr<Level>& level) { return true; }
std::shared_ptr<Level> DataPersistenceManager::loadLevel(const std::string& levelId) { return nullptr; }
std::shared_ptr<Level> DataPersistenceManager::loadLevelAsync(const std::string& levelId) { return nullptr; }
bool DataPersistenceManager::deleteLevel(const std::string& levelId) { return true; }
std::vector<std::shared_ptr<Level>> DataPersistenceManager::loadAllLevels() { return {}; }
std::vector<std::shared_ptr<Level>> DataPersistenceManager::loadLevelsByDifficulty(double minDifficulty, double maxDifficulty) { return {}; }
bool DataPersistenceManager::levelExists(const std::string& levelId) { return false; }

bool DataPersistenceManager::saveSkill(const std::shared_ptr<Skill>& skill) { return true; }
bool DataPersistenceManager::saveSkillAsync(const std::shared_ptr<Skill>& skill) { return true; }
std::shared_ptr<Skill> DataPersistenceManager::loadSkill(const std::string& skillId) { return nullptr; }
std::shared_ptr<Skill> DataPersistenceManager::loadSkillAsync(const std::string& skillId) { return nullptr; }
bool DataPersistenceManager::deleteSkill(const std::string& skillId) { return true; }
std::vector<std::shared_ptr<Skill>> DataPersistenceManager::loadAllSkills() { return {}; }
std::vector<std::shared_ptr<Skill>> DataPersistenceManager::loadSkillsByType(const std::string& type) { return {}; }
bool DataPersistenceManager::skillExists(const std::string& skillId) { return false; }

bool DataPersistenceManager::saveItems(const std::vector<std::shared_ptr<Item>>& items) { return true; }
bool DataPersistenceManager::saveQuests(const std::vector<std::shared_ptr<Quest>>& quests) { return true; }
bool DataPersistenceManager::saveLevels(const std::vector<std::shared_ptr<Level>>& levels) { return true; }
bool DataPersistenceManager::saveSkills(const std::vector<std::shared_ptr<Skill>>& skills) { return true; }

} // namespace Fantasy 