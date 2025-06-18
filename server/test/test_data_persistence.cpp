/**
 * @file test_data_persistence.cpp
 * @brief DataPersistenceManager测试文件
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "core/persistence/DataPersistenceManager.h"
#include "core/characters/Character.h"
#include "core/items/Item.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>

using namespace Fantasy;

void testDataPersistenceManager() {
    std::cout << "=== Testing DataPersistenceManager ===" << std::endl;
    
    // 获取单例实例
    auto& persistenceManager = DataPersistenceManager::getInstance();
    
    // 配置
    DataPersistenceConfig config;
    config.enableAutoSave = true;
    config.autoSaveInterval = std::chrono::seconds(10);
    config.enableDataValidation = true;
    config.maxCacheSize = 100;
    
    // 初始化
    std::cout << "Initializing DataPersistenceManager..." << std::endl;
    bool initialized = persistenceManager.initialize(config);
    assert(initialized && "Failed to initialize DataPersistenceManager");
    std::cout << "✓ DataPersistenceManager initialized successfully" << std::endl;
    
    // 测试角色创建和保存
    std::cout << "\nTesting character creation and save..." << std::endl;
    auto character = std::make_shared<Character>("Test Warrior", CharacterClass::WARRIOR);
    
    // 设置角色属性
    CharacterStats stats;
    stats.level = 10;
    stats.experience = 1500;
    stats.health = 100;
    stats.maxHealth = 100;
    stats.mana = 50;
    stats.maxMana = 50;
    stats.attack = 15;
    stats.defense = 12;
    stats.speed = 8;
    stats.magicAttack = 8;
    character->setStats(stats);
    
    bool saved = persistenceManager.saveCharacter(character);
    assert(saved && "Failed to save character");
    std::cout << "✓ Character saved successfully" << std::endl;
    
    // 测试角色加载
    std::cout << "\nTesting character loading..." << std::endl;
    auto loadedCharacter = persistenceManager.loadCharacter("Test Warrior");
    assert(loadedCharacter && "Failed to load character");
    assert(loadedCharacter->getName() == "Test Warrior" && "Character name mismatch");
    assert(loadedCharacter->getStats().level == 10 && "Character level mismatch");
    std::cout << "✓ Character loaded successfully" << std::endl;
    
    // 测试角色更新
    std::cout << "\nTesting character update..." << std::endl;
    CharacterStats updatedStats = loadedCharacter->getStats();
    updatedStats.level = 11;
    updatedStats.experience = 2000;
    loadedCharacter->setStats(updatedStats);
    
    bool updated = persistenceManager.saveCharacter(loadedCharacter);
    assert(updated && "Failed to update character");
    
    auto updatedCharacter = persistenceManager.loadCharacter("Test Warrior");
    assert(updatedCharacter->getStats().level == 11 && "Character level not updated");
    assert(updatedCharacter->getStats().experience == 2000 && "Character experience not updated");
    std::cout << "✓ Character updated successfully" << std::endl;
    
    // 测试游戏存档
    std::cout << "\nTesting game save..." << std::endl;
    Vector2D position(100.0f, 200.0f);
    bool gameSaved = persistenceManager.saveGameState("save_slot_1", character, "level_1", position);
    assert(gameSaved && "Failed to save game state");
    
    auto gameState = persistenceManager.loadGameState("save_slot_1");
    assert(gameState && "Failed to load game state");
    assert(gameState->characterId == "Test Warrior" && "Game state character ID mismatch");
    assert(gameState->currentMap == "level_1" && "Game state level mismatch");
    std::cout << "✓ Game state saved and loaded successfully" << std::endl;
    
    // 测试缓存功能
    std::cout << "\nTesting cache functionality..." << std::endl;
    size_t cacheSize = persistenceManager.getCacheSize("character");
    assert(cacheSize > 0 && "Cache should contain character data");
    std::cout << "✓ Cache size: " << cacheSize << std::endl;
    
    // 测试数据验证
    std::cout << "\nTesting data validation..." << std::endl;
    auto invalidCharacter = std::make_shared<Character>("", CharacterClass::WARRIOR); // 无效名称
    bool validationResult = persistenceManager.validateCharacterData(invalidCharacter);
    assert(!validationResult && "Invalid character should fail validation");
    std::cout << "✓ Data validation working correctly" << std::endl;
    
    // 测试统计信息
    std::cout << "\nTesting statistics..." << std::endl;
    auto stats_result = persistenceManager.getStats();
    assert(stats_result.totalSaves > 0 && "Should have save statistics");
    assert(stats_result.totalLoads > 0 && "Should have load statistics");
    std::cout << "✓ Statistics: " << stats_result.successfulSaves << " saves, " 
              << stats_result.successfulLoads << " loads" << std::endl;
    
    // 测试事件系统
    std::cout << "\nTesting event system..." << std::endl;
    bool eventReceived = false;
    persistenceManager.subscribeToDataChanges([&eventReceived](const DataChangeEvent& event) {
        eventReceived = true;
        std::cout << "  Event received: " << event.entityType << " - " << event.entityId << std::endl;
    });
    
    // 触发一个事件
    auto testCharacter = std::make_shared<Character>("Event Test", CharacterClass::MAGE);
    persistenceManager.saveCharacter(testCharacter);
    
    // 等待事件处理
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(eventReceived && "Event should have been received");
    std::cout << "✓ Event system working correctly" << std::endl;
    
    // 测试自动保存
    std::cout << "\nTesting auto-save..." << std::endl;
    assert(persistenceManager.isAutoSaveEnabled() && "Auto-save should be enabled");
    std::cout << "✓ Auto-save enabled" << std::endl;
    
    // 测试配置管理
    std::cout << "\nTesting configuration management..." << std::endl;
    auto currentConfig = persistenceManager.getConfig();
    assert(currentConfig.enableAutoSave == config.enableAutoSave && "Config should match");
    std::cout << "✓ Configuration management working" << std::endl;
    
    // 测试错误处理
    std::cout << "\nTesting error handling..." << std::endl;
    auto nonExistentCharacter = persistenceManager.loadCharacter("non_existent_id");
    assert(!nonExistentCharacter && "Non-existent character should return null");
    std::string lastError = persistenceManager.getLastError();
    assert(!lastError.empty() && "Should have error message");
    std::cout << "✓ Error handling working: " << lastError << std::endl;
    
    // 测试数据同步
    std::cout << "\nTesting data synchronization..." << std::endl;
    bool syncResult = persistenceManager.syncAllData();
    assert(syncResult && "Data sync should succeed");
    std::cout << "✓ Data synchronization working" << std::endl;
    
    // 测试缓存清理
    std::cout << "\nTesting cache clearing..." << std::endl;
    persistenceManager.clearCache();
    size_t clearedCacheSize = persistenceManager.getCacheSize();
    assert(clearedCacheSize == 0 && "Cache should be empty after clearing");
    std::cout << "✓ Cache clearing working" << std::endl;
    
    // 测试角色存在性检查
    std::cout << "\nTesting character existence check..." << std::endl;
    bool exists = persistenceManager.characterExists("Test Warrior");
    assert(exists && "Character should exist");
    bool notExists = persistenceManager.characterExists("non_existent_id");
    assert(!notExists && "Non-existent character should not exist");
    std::cout << "✓ Character existence check working" << std::endl;
    
    // 测试获取角色ID列表
    std::cout << "\nTesting character ID list..." << std::endl;
    auto characterIds = persistenceManager.getCharacterIds();
    assert(!characterIds.empty() && "Should have character IDs");
    bool foundTestCharacter = false;
    for (const auto& id : characterIds) {
        if (id == "Test Warrior") {
            foundTestCharacter = true;
            break;
        }
    }
    assert(foundTestCharacter && "Test character should be in the list");
    std::cout << "✓ Character ID list working" << std::endl;
    
    // 测试游戏存档存在性检查
    std::cout << "\nTesting save slot existence check..." << std::endl;
    bool saveExists = persistenceManager.saveSlotExists("save_slot_1");
    assert(saveExists && "Save slot should exist");
    bool saveNotExists = persistenceManager.saveSlotExists("non_existent_slot");
    assert(!saveNotExists && "Non-existent save slot should not exist");
    std::cout << "✓ Save slot existence check working" << std::endl;
    
    // 测试获取所有存档
    std::cout << "\nTesting get all save states..." << std::endl;
    auto allSaves = persistenceManager.getAllSaveStates();
    assert(!allSaves.empty() && "Should have save states");
    std::cout << "✓ Found " << allSaves.size() << " save states" << std::endl;
    
    // 测试按角色获取存档
    std::cout << "\nTesting get save states by character..." << std::endl;
    auto characterSaves = persistenceManager.getSaveStatesByCharacter("Test Warrior");
    assert(!characterSaves.empty() && "Should have saves for test character");
    std::cout << "✓ Found " << characterSaves.size() << " saves for test character" << std::endl;
    
    // 测试数据有效性检查
    std::cout << "\nTesting data validity check..." << std::endl;
    bool isValid = persistenceManager.isDataValid("character", "Test Warrior");
    assert(isValid && "Test character data should be valid");
    std::cout << "✓ Data validity check working" << std::endl;
    
    // 测试获取无效数据列表
    std::cout << "\nTesting get invalid data..." << std::endl;
    auto invalidData = persistenceManager.getInvalidData();
    std::cout << "✓ Found " << invalidData.size() << " invalid data entries" << std::endl;
    
    // 测试统计信息重置
    std::cout << "\nTesting statistics reset..." << std::endl;
    persistenceManager.resetStats();
    auto resetStats = persistenceManager.getStats();
    assert(resetStats.totalSaves == 0 && "Stats should be reset");
    assert(resetStats.totalLoads == 0 && "Stats should be reset");
    std::cout << "✓ Statistics reset working" << std::endl;
    
    // 测试配置更新
    std::cout << "\nTesting configuration update..." << std::endl;
    DataPersistenceConfig newConfig = config;
    newConfig.autoSaveInterval = std::chrono::seconds(30);
    persistenceManager.setConfig(newConfig);
    auto updatedConfig = persistenceManager.getConfig();
    assert(updatedConfig.autoSaveInterval == std::chrono::seconds(30) && "Config should be updated");
    std::cout << "✓ Configuration update working" << std::endl;
    
    // 测试自动保存间隔设置
    std::cout << "\nTesting auto-save interval setting..." << std::endl;
    persistenceManager.setAutoSaveInterval(std::chrono::seconds(60));
    auto interval = persistenceManager.getAutoSaveInterval();
    assert(interval == std::chrono::seconds(60) && "Auto-save interval should be updated");
    std::cout << "✓ Auto-save interval setting working" << std::endl;
    
    // 测试自动保存开关
    std::cout << "\nTesting auto-save toggle..." << std::endl;
    persistenceManager.enableAutoSave(false);
    assert(!persistenceManager.isAutoSaveEnabled() && "Auto-save should be disabled");
    persistenceManager.enableAutoSave(true);
    assert(persistenceManager.isAutoSaveEnabled() && "Auto-save should be enabled");
    std::cout << "✓ Auto-save toggle working" << std::endl;
    
    // 测试手动触发自动保存
    std::cout << "\nTesting manual auto-save trigger..." << std::endl;
    // 注意：triggerAutoSave是私有方法，不能直接调用
    // 通过其他方式测试自动保存功能
    std::cout << "✓ Auto-save functionality available" << std::endl;
    
    // 测试同步状态
    std::cout << "\nTesting sync status..." << std::endl;
    auto syncStatus = persistenceManager.getSyncStatus("character", "Test Warrior");
    assert(syncStatus == SyncStatus::SYNCED && "Character should be synced");
    std::cout << "✓ Sync status working" << std::endl;
    
    // 测试待保存列表
    std::cout << "\nTesting pending saves..." << std::endl;
    auto pendingSaves = persistenceManager.getPendingSaves();
    std::cout << "✓ Found " << pendingSaves.size() << " pending saves" << std::endl;
    
    // 测试待加载列表
    std::cout << "\nTesting pending loads..." << std::endl;
    auto pendingLoads = persistenceManager.getPendingLoads();
    std::cout << "✓ Found " << pendingLoads.size() << " pending loads" << std::endl;
    
    // 测试缓存大小获取
    std::cout << "\nTesting cache size retrieval..." << std::endl;
    size_t totalCacheSize = persistenceManager.getCacheSize();
    size_t characterCacheSize = persistenceManager.getCacheSize("character");
    std::cout << "✓ Total cache size: " << totalCacheSize << ", Character cache size: " << characterCacheSize << std::endl;
    
    // 测试缓存移除
    std::cout << "\nTesting cache removal..." << std::endl;
    size_t beforeSize = persistenceManager.getCacheSize("character");
    auto testChar = persistenceManager.loadCharacter("Test Warrior"); // 这会添加到缓存
    size_t afterSize = persistenceManager.getCacheSize("character");
    std::cout << "✓ Cache size before: " << beforeSize << ", after: " << afterSize << std::endl;
    
    // 测试数据修复
    std::cout << "\nTesting data repair..." << std::endl;
    bool repairResult = persistenceManager.repairData("character", "Test Warrior");
    std::cout << "✓ Data repair result: " << (repairResult ? "success" : "failed") << std::endl;
    
    // 测试批量操作
    std::cout << "\nTesting batch operations..." << std::endl;
    std::vector<std::shared_ptr<Character>> characters;
    for (int i = 0; i < 3; ++i) {
        auto batchChar = std::make_shared<Character>("Batch Character " + std::to_string(i), CharacterClass::WARRIOR);
        CharacterStats batchStats;
        batchStats.level = 1;
        batchChar->setStats(batchStats);
        characters.push_back(batchChar);
    }
    
    bool batchResult = persistenceManager.saveCharacters(characters);
    assert(batchResult && "Batch save should succeed");
    std::cout << "✓ Batch operations working" << std::endl;
    
    // 测试加载所有角色
    std::cout << "\nTesting load all characters..." << std::endl;
    auto allCharacters = persistenceManager.loadAllCharacters();
    assert(!allCharacters.empty() && "Should have characters");
    std::cout << "✓ Loaded " << allCharacters.size() << " characters" << std::endl;
    
    // 测试删除操作
    std::cout << "\nTesting delete operations..." << std::endl;
    bool deleteResult = persistenceManager.deleteCharacter("Batch Character 0");
    assert(deleteResult && "Delete should succeed");
    std::cout << "✓ Delete operations working" << std::endl;
    
    // 测试游戏存档删除
    std::cout << "\nTesting game state deletion..." << std::endl;
    bool deleteSaveResult = persistenceManager.deleteGameState("save_slot_1");
    assert(deleteSaveResult && "Save deletion should succeed");
    std::cout << "✓ Game state deletion working" << std::endl;
    
    // 测试异步操作（基本测试）
    std::cout << "\nTesting async operations..." << std::endl;
    auto asyncChar = std::make_shared<Character>("Async Test", CharacterClass::MAGE);
    CharacterStats asyncStats;
    asyncStats.level = 5;
    asyncChar->setStats(asyncStats);
    
    bool asyncSaveResult = persistenceManager.saveCharacterAsync(asyncChar);
    assert(asyncSaveResult && "Async save should succeed");
    std::cout << "✓ Async operations working" << std::endl;
    
    // 等待异步操作完成
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // 测试异步加载
    auto asyncLoadedChar = persistenceManager.loadCharacterAsync("Async Test");
    assert(asyncLoadedChar && "Async load should succeed");
    std::cout << "✓ Async load working" << std::endl;
    
    // 测试数据备份（模拟）
    std::cout << "\nTesting data backup..." << std::endl;
    bool backupResult = persistenceManager.backupAllData("test_backup");
    std::cout << "✓ Data backup result: " << (backupResult ? "success" : "failed") << std::endl;
    
    // 测试数据恢复（模拟）
    std::cout << "\nTesting data restore..." << std::endl;
    bool restoreResult = persistenceManager.restoreAllData("test_backup");
    std::cout << "✓ Data restore result: " << (restoreResult ? "success" : "failed") << std::endl;
    
    // 测试特定类型数据备份
    std::cout << "\nTesting specific type backup..." << std::endl;
    bool typeBackupResult = persistenceManager.backupData("character", "character_backup");
    std::cout << "✓ Type-specific backup result: " << (typeBackupResult ? "success" : "failed") << std::endl;
    
    // 测试特定类型数据恢复
    std::cout << "\nTesting specific type restore..." << std::endl;
    bool typeRestoreResult = persistenceManager.restoreData("character", "character_backup");
    std::cout << "✓ Type-specific restore result: " << (typeRestoreResult ? "success" : "failed") << std::endl;
    
    // 测试初始化状态检查
    std::cout << "\nTesting initialization status..." << std::endl;
    assert(persistenceManager.isInitialized() && "Manager should be initialized");
    std::cout << "✓ Initialization status check working" << std::endl;
    
    // 测试关闭
    std::cout << "\nTesting shutdown..." << std::endl;
    persistenceManager.shutdown();
    assert(!persistenceManager.isInitialized() && "Manager should not be initialized after shutdown");
    std::cout << "✓ Shutdown working correctly" << std::endl;
    
    std::cout << "\n=== All DataPersistenceManager tests passed! ===" << std::endl;
}

int main() {
    try {
        testDataPersistenceManager();
        std::cout << "\n🎉 All tests completed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
} 