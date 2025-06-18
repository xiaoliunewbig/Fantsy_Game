/**
 * @file data_persistence_example.cpp
 * @brief DataPersistenceManager使用示例
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "core/persistence/DataPersistenceManager.h"
#include "core/characters/Character.h"
#include "core/items/Item.h"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

using namespace Fantasy;

void dataPersistenceExample() {
    std::cout << "=== DataPersistenceManager 使用示例 ===" << std::endl;
    
    // 1. 获取单例实例
    auto& persistenceManager = DataPersistenceManager::getInstance();
    
    // 2. 配置数据持久化管理器
    DataPersistenceConfig config;
    config.enableAutoSave = true;
    config.autoSaveInterval = std::chrono::seconds(30); // 30秒自动保存
    config.enableDataValidation = true;
    config.maxCacheSize = 500;
    config.enableDataCompression = true;
    config.backupDirectory = "game_backups";
    config.maxBackupFiles = 5;
    
    // 3. 初始化
    if (!persistenceManager.initialize(config)) {
        std::cerr << "❌ 初始化DataPersistenceManager失败" << std::endl;
        return;
    }
    std::cout << "✅ DataPersistenceManager初始化成功" << std::endl;
    
    // 4. 订阅数据变更事件
    persistenceManager.subscribeToDataChanges([](const DataChangeEvent& event) {
        std::cout << "📝 数据变更事件: " << event.entityType 
                  << " - " << event.entityId 
                  << " (类型: " << static_cast<int>(event.type) << ")" << std::endl;
    });
    
    // 5. 创建游戏角色
    std::cout << "\n--- 创建游戏角色 ---" << std::endl;
    auto warrior = std::make_shared<Character>("勇敢的战士", CharacterClass::WARRIOR);
    
    // 设置战士属性
    CharacterStats warriorStats;
    warriorStats.level = 15;
    warriorStats.experience = 2500;
    warriorStats.health = 120;
    warriorStats.maxHealth = 120;
    warriorStats.mana = 30;
    warriorStats.maxMana = 30;
    warriorStats.attack = 18;
    warriorStats.defense = 10;
    warriorStats.speed = 12;
    warriorStats.magicAttack = 8;
    warrior->setStats(warriorStats);
    
    // 6. 保存角色
    if (persistenceManager.saveCharacter(warrior)) {
        std::cout << "✅ 角色保存成功: " << warrior->getName() << std::endl;
    } else {
        std::cerr << "❌ 角色保存失败: " << persistenceManager.getLastError() << std::endl;
    }
    
    // 7. 创建法师角色
    auto mage = std::make_shared<Character>("智慧的法师", CharacterClass::MAGE);
    
    // 设置法师属性
    CharacterStats mageStats;
    mageStats.level = 12;
    mageStats.experience = 1800;
    mageStats.health = 80;
    mageStats.maxHealth = 80;
    mageStats.mana = 100;
    mageStats.maxMana = 100;
    mageStats.attack = 6;
    mageStats.defense = 5;
    mageStats.speed = 8;
    mageStats.magicAttack = 20;
    mage->setStats(mageStats);
    
    if (persistenceManager.saveCharacter(mage)) {
        std::cout << "✅ 法师保存成功: " << mage->getName() << std::endl;
    }
    
    // 8. 加载角色
    std::cout << "\n--- 加载角色 ---" << std::endl;
    auto loadedWarrior = persistenceManager.loadCharacter("勇敢的战士");
    if (loadedWarrior) {
        std::cout << "✅ 角色加载成功: " << loadedWarrior->getName() 
                  << " (等级: " << loadedWarrior->getStats().level << ")" << std::endl;
    }
    
    // 9. 更新角色
    std::cout << "\n--- 更新角色 ---" << std::endl;
    CharacterStats updatedStats = loadedWarrior->getStats();
    updatedStats.level = 16;
    updatedStats.experience = 3000;
    updatedStats.health = 130;
    updatedStats.maxHealth = 130;
    loadedWarrior->setStats(updatedStats);
    
    if (persistenceManager.saveCharacter(loadedWarrior)) {
        std::cout << "✅ 角色更新成功: " << loadedWarrior->getName() 
                  << " (新等级: " << loadedWarrior->getStats().level << ")" << std::endl;
    }
    
    // 10. 创建游戏存档
    std::cout << "\n--- 创建游戏存档 ---" << std::endl;
    Vector2D playerPosition(150.5f, 200.3f);
    if (persistenceManager.saveGameState("save_slot_1", warrior, "forest_level", playerPosition)) {
        std::cout << "✅ 游戏存档创建成功: save_slot_1" << std::endl;
    }
    
    // 11. 加载游戏存档
    std::cout << "\n--- 加载游戏存档 ---" << std::endl;
    auto gameState = persistenceManager.loadGameState("save_slot_1");
    if (gameState) {
        std::cout << "✅ 游戏存档加载成功:" << std::endl;
        std::cout << "   角色ID: " << gameState->characterId << std::endl;
        std::cout << "   当前关卡: " << gameState->currentMap << std::endl;
        std::cout << "   位置: (" << gameState->positionX << ", " << gameState->positionY << ")" << std::endl;
    }
    
    // 12. 批量操作示例
    std::cout << "\n--- 批量操作示例 ---" << std::endl;
    std::vector<std::shared_ptr<Character>> characters;
    
    // 创建多个角色
    for (int i = 1; i <= 3; ++i) {
        auto charPtr = std::make_shared<Character>("批量角色 " + std::to_string(i), CharacterClass::WARRIOR);
        CharacterStats batchStats;
        batchStats.level = i * 5;
        batchStats.experience = i * 1000;
        batchStats.health = 50 + i * 10;
        batchStats.maxHealth = 50 + i * 10;
        charPtr->setStats(batchStats);
        characters.push_back(charPtr);
    }
    
    if (persistenceManager.saveCharacters(characters)) {
        std::cout << "✅ 批量保存成功: " << characters.size() << " 个角色" << std::endl;
    }
    
    // 13. 加载所有角色
    std::cout << "\n--- 加载所有角色 ---" << std::endl;
    auto allCharacters = persistenceManager.loadAllCharacters();
    std::cout << "✅ 总共加载了 " << allCharacters.size() << " 个角色" << std::endl;
    
    for (const auto& charPtr : allCharacters) {
        std::cout << "   - " << charPtr->getName() << " (等级 " << charPtr->getStats().level << ")" << std::endl;
    }
    
    // 14. 缓存管理示例
    std::cout << "\n--- 缓存管理 ---" << std::endl;
    size_t cacheSize = persistenceManager.getCacheSize();
    size_t characterCacheSize = persistenceManager.getCacheSize("character");
    std::cout << "缓存统计:" << std::endl;
    std::cout << "   总缓存大小: " << cacheSize << std::endl;
    std::cout << "   角色缓存大小: " << characterCacheSize << std::endl;
    
    // 15. 统计信息
    std::cout << "\n--- 统计信息 ---" << std::endl;
    auto stats = persistenceManager.getStats();
    std::cout << "操作统计:" << std::endl;
    std::cout << "   总保存次数: " << stats.totalSaves << std::endl;
    std::cout << "   成功保存次数: " << stats.successfulSaves << std::endl;
    std::cout << "   总加载次数: " << stats.totalLoads << std::endl;
    std::cout << "   成功加载次数: " << stats.successfulLoads << std::endl;
    std::cout << "   缓存命中次数: " << stats.cacheHits << std::endl;
    std::cout << "   缓存未命中次数: " << stats.cacheMisses << std::endl;
    
    // 16. 数据验证示例
    std::cout << "\n--- 数据验证 ---" << std::endl;
    auto invalidCharacter = std::make_shared<Character>("", CharacterClass::WARRIOR); // 无效名称
    
    if (persistenceManager.validateCharacterData(invalidCharacter)) {
        std::cout << "❌ 数据验证失败: 应该拒绝无效数据" << std::endl;
    } else {
        std::cout << "✅ 数据验证正确: 拒绝了无效数据" << std::endl;
    }
    
    // 17. 角色存在性检查
    std::cout << "\n--- 角色存在性检查 ---" << std::endl;
    if (persistenceManager.characterExists("勇敢的战士")) {
        std::cout << "✅ 角色 勇敢的战士 存在" << std::endl;
    }
    
    if (!persistenceManager.characterExists("non_existent")) {
        std::cout << "✅ 角色 non_existent 不存在" << std::endl;
    }
    
    // 18. 获取角色ID列表
    std::cout << "\n--- 角色ID列表 ---" << std::endl;
    auto characterIds = persistenceManager.getCharacterIds();
    std::cout << "角色ID列表 (" << characterIds.size() << " 个):" << std::endl;
    for (const auto& id : characterIds) {
        std::cout << "   - " << id << std::endl;
    }
    
    // 19. 存档管理
    std::cout << "\n--- 存档管理 ---" << std::endl;
    auto allSaves = persistenceManager.getAllSaveStates();
    std::cout << "所有存档 (" << allSaves.size() << " 个):" << std::endl;
    for (const auto& save : allSaves) {
        std::cout << "   - " << save.saveSlot << " (角色: " << save.characterId << ")" << std::endl;
    }
    
    // 20. 按角色获取存档
    auto warriorSaves = persistenceManager.getSaveStatesByCharacter("勇敢的战士");
    std::cout << "战士的存档数量: " << warriorSaves.size() << std::endl;
    
    // 21. 数据同步
    std::cout << "\n--- 数据同步 ---" << std::endl;
    if (persistenceManager.syncAllData()) {
        std::cout << "✅ 数据同步成功" << std::endl;
    }
    
    // 22. 自动保存测试
    std::cout << "\n--- 自动保存测试 ---" << std::endl;
    std::cout << "自动保存状态: " << (persistenceManager.isAutoSaveEnabled() ? "启用" : "禁用") << std::endl;
    std::cout << "自动保存间隔: " << persistenceManager.getAutoSaveInterval().count() << " 秒" << std::endl;
    
    // 手动触发自动保存
    persistenceManager.triggerAutoSave();
    std::cout << "✅ 手动触发自动保存" << std::endl;
    
    // 23. 配置管理
    std::cout << "\n--- 配置管理 ---" << std::endl;
    auto currentConfig = persistenceManager.getConfig();
    std::cout << "当前配置:" << std::endl;
    std::cout << "   自动保存: " << (currentConfig.enableAutoSave ? "启用" : "禁用") << std::endl;
    std::cout << "   数据验证: " << (currentConfig.enableDataValidation ? "启用" : "禁用") << std::endl;
    std::cout << "   数据压缩: " << (currentConfig.enableDataCompression ? "启用" : "禁用") << std::endl;
    std::cout << "   最大缓存大小: " << currentConfig.maxCacheSize << std::endl;
    
    // 24. 错误处理示例
    std::cout << "\n--- 错误处理 ---" << std::endl;
    auto nonExistent = persistenceManager.loadCharacter("non_existent_id");
    if (!nonExistent) {
        std::cout << "✅ 正确处理了不存在的角色: " << persistenceManager.getLastError() << std::endl;
    }
    
    // 25. 数据备份示例
    std::cout << "\n--- 数据备份 ---" << std::endl;
    if (persistenceManager.backupAllData("example_backup")) {
        std::cout << "✅ 数据备份成功" << std::endl;
    } else {
        std::cout << "❌ 数据备份失败: " << persistenceManager.getLastError() << std::endl;
    }
    
    // 26. 等待一段时间观察自动保存
    std::cout << "\n--- 等待自动保存 (5秒) ---" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // 27. 清理测试数据
    std::cout << "\n--- 清理测试数据 ---" << std::endl;
    persistenceManager.deleteCharacter("批量角色 1");
    persistenceManager.deleteCharacter("批量角色 2");
    persistenceManager.deleteCharacter("批量角色 3");
    persistenceManager.deleteGameState("save_slot_1");
    std::cout << "✅ 测试数据清理完成" << std::endl;
    
    // 28. 关闭管理器
    std::cout << "\n--- 关闭管理器 ---" << std::endl;
    persistenceManager.shutdown();
    std::cout << "✅ DataPersistenceManager已关闭" << std::endl;
    
    std::cout << "\n=== 示例运行完成 ===" << std::endl;
}

int main() {
    try {
        dataPersistenceExample();
        std::cout << "\n🎉 示例运行成功!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ 示例运行失败: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ 示例运行失败: 未知异常" << std::endl;
        return 1;
    }
} 