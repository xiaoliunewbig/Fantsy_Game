# DataPersistenceManager 数据持久化管理器

## 概述

DataPersistenceManager 是 Fantasy Legend 游戏服务器的核心数据持久化组件，负责将游戏中的各种数据（角色、物品、任务、关卡等）与数据库管理系统集成，提供高效的数据存储、加载和同步功能。

## 主要功能

### 🎯 核心功能
- **角色数据持久化**: 保存和加载玩家角色信息
- **游戏存档管理**: 管理游戏进度和状态
- **物品数据持久化**: 处理游戏物品的存储
- **任务数据持久化**: 管理游戏任务数据
- **关卡数据持久化**: 处理关卡信息
- **技能数据持久化**: 管理角色技能数据

### ⚡ 高级特性
- **自动保存机制**: 定期自动保存游戏数据
- **数据同步**: 确保数据一致性
- **缓存管理**: 提高数据访问性能
- **数据验证**: 确保数据完整性
- **事件系统**: 数据变更通知
- **批量操作**: 高效处理大量数据
- **数据备份**: 自动备份和恢复

## 快速开始

### 1. 初始化

```cpp
#include "include/core/persistence/DataPersistenceManager.h"

// 获取单例实例
auto& persistenceManager = DataPersistenceManager::getInstance();

// 配置
DataPersistenceConfig config;
config.enableAutoSave = true;
config.autoSaveInterval = std::chrono::seconds(300); // 5分钟
config.enableDataValidation = true;
config.maxCacheSize = 1000;

// 初始化
if (!persistenceManager.initialize(config)) {
    std::cerr << "初始化失败!" << std::endl;
    return;
}
```

### 2. 角色数据操作

```cpp
// 创建角色
auto character = std::make_shared<Character>();
character->setId("player_001");
character->setName("勇敢的战士");
character->setLevel(10);

// 保存角色
if (persistenceManager.saveCharacter(character)) {
    std::cout << "角色保存成功!" << std::endl;
}

// 加载角色
auto loadedCharacter = persistenceManager.loadCharacter("player_001");
if (loadedCharacter) {
    std::cout << "角色加载成功: " << loadedCharacter->getName() << std::endl;
}

// 更新角色
loadedCharacter->setLevel(11);
persistenceManager.saveCharacter(loadedCharacter);
```

### 3. 游戏存档管理

```cpp
// 保存游戏状态
Vector2D position(100.0f, 200.0f);
if (persistenceManager.saveGameState("save_slot_1", character, "level_1", position)) {
    std::cout << "游戏存档成功!" << std::endl;
}

// 加载游戏状态
auto gameState = persistenceManager.loadGameState("save_slot_1");
if (gameState) {
    std::cout << "当前关卡: " << gameState->currentLevel << std::endl;
    std::cout << "位置: (" << gameState->position.x << ", " << gameState->position.y << ")" << std::endl;
}
```

### 4. 事件监听

```cpp
// 订阅数据变更事件
persistenceManager.subscribeToDataChanges([](const DataChangeEvent& event) {
    std::cout << "数据变更: " << event.entityType << " - " << event.entityId << std::endl;
});
```

### 5. 批量操作

```cpp
// 批量保存角色
std::vector<std::shared_ptr<Character>> characters;
// ... 添加角色到vector
persistenceManager.saveCharacters(characters);

// 加载所有角色
auto allCharacters = persistenceManager.loadAllCharacters();
```

## 配置选项

### DataPersistenceConfig 结构

```cpp
struct DataPersistenceConfig {
    bool enableAutoSave = true;                    // 启用自动保存
    std::chrono::seconds autoSaveInterval{300};    // 自动保存间隔
    bool enableDataCompression = true;             // 启用数据压缩
    bool enableDataEncryption = false;             // 启用数据加密
    std::string encryptionKey = "";                // 加密密钥
    size_t maxCacheSize = 1000;                    // 最大缓存大小
    bool enableDataValidation = true;              // 启用数据验证
    bool enableBackupOnSave = true;                // 保存时自动备份
    std::string backupDirectory = "backups";       // 备份目录
    size_t maxBackupFiles = 10;                    // 最大备份文件数
};
```

## API 参考

### 角色管理

```cpp
// 保存角色
bool saveCharacter(const std::shared_ptr<Character>& character);
bool saveCharacterAsync(const std::shared_ptr<Character>& character);

// 加载角色
std::shared_ptr<Character> loadCharacter(const std::string& characterId);
std::shared_ptr<Character> loadCharacterAsync(const std::string& characterId);

// 删除角色
bool deleteCharacter(const std::string& characterId);

// 批量操作
bool saveCharacters(const std::vector<std::shared_ptr<Character>>& characters);
std::vector<std::shared_ptr<Character>> loadAllCharacters();

// 查询操作
std::vector<std::string> getCharacterIds();
bool characterExists(const std::string& characterId);
```

### 游戏存档管理

```cpp
// 保存游戏状态
bool saveGameState(const std::string& saveSlot, const std::shared_ptr<Character>& character, 
                  const std::string& currentLevel, const Vector2D& position);

// 加载游戏状态
std::optional<GameSaveData> loadGameState(const std::string& saveSlot);

// 删除存档
bool deleteGameState(const std::string& saveSlot);

// 查询存档
std::vector<GameSaveData> getAllSaveStates();
std::vector<GameSaveData> getSaveStatesByCharacter(const std::string& characterId);
bool saveSlotExists(const std::string& saveSlot);
```

### 缓存管理

```cpp
// 缓存操作
void clearCache();
void clearCache(const std::string& entityType);
void removeFromCache(const std::string& entityType, const std::string& entityId);

// 缓存统计
size_t getCacheSize() const;
size_t getCacheSize(const std::string& entityType) const;
```

### 数据同步

```cpp
// 同步操作
bool syncAllData();
bool syncData(const std::string& entityType, const std::string& entityId);

// 同步状态
SyncStatus getSyncStatus(const std::string& entityType, const std::string& entityId);
std::vector<std::string> getPendingSaves();
std::vector<std::string> getPendingLoads();
```

### 自动保存

```cpp
// 自动保存控制
void enableAutoSave(bool enable);
bool isAutoSaveEnabled() const;
void setAutoSaveInterval(std::chrono::seconds interval);
std::chrono::seconds getAutoSaveInterval() const;
void triggerAutoSave();
```

### 数据验证

```cpp
// 数据验证
bool validateCharacterData(const std::shared_ptr<Character>& character);
bool validateItemData(const std::shared_ptr<Item>& item);
bool validateQuestData(const std::shared_ptr<Quest>& quest);
bool validateLevelData(const std::shared_ptr<Level>& level);
bool validateSkillData(const std::shared_ptr<Skill>& skill);
```

### 数据备份

```cpp
// 备份和恢复
bool backupAllData(const std::string& backupPath);
bool restoreAllData(const std::string& backupPath);
bool backupData(const std::string& entityType, const std::string& backupPath);
bool restoreData(const std::string& entityType, const std::string& backupPath);
```

### 统计信息

```cpp
// 获取统计信息
PersistenceStats getStats() const;
void resetStats();
```

## 事件系统

### DataChangeEvent 结构

```cpp
struct DataChangeEvent {
    DataChangeType type;                                    // 事件类型
    std::string entityId;                                   // 实体ID
    std::string entityType;                                 // 实体类型
    std::chrono::system_clock::time_point timestamp;        // 时间戳
    std::unordered_map<std::string, std::string> changes;   // 变更详情
};
```

### 事件类型

```cpp
enum class DataChangeType {
    CHARACTER_CREATED,    // 角色创建
    CHARACTER_UPDATED,    // 角色更新
    CHARACTER_DELETED,    // 角色删除
    ITEM_CREATED,         // 物品创建
    ITEM_UPDATED,         // 物品更新
    ITEM_DELETED,         // 物品删除
    QUEST_CREATED,        // 任务创建
    QUEST_UPDATED,        // 任务更新
    QUEST_DELETED,        // 任务删除
    LEVEL_CREATED,        // 关卡创建
    LEVEL_UPDATED,        // 关卡更新
    LEVEL_DELETED,        // 关卡删除
    SAVE_CREATED,         // 存档创建
    SAVE_UPDATED,         // 存档更新
    SAVE_DELETED          // 存档删除
};
```

## 性能优化

### 缓存策略
- 使用LRU缓存算法管理内存
- 支持按实体类型分别缓存
- 可配置最大缓存大小

### 异步操作
- 提供异步保存和加载接口
- 避免阻塞主线程
- 支持并发操作

### 批量操作
- 批量保存提高效率
- 减少数据库连接开销
- 支持事务处理

## 错误处理

### 错误获取
```cpp
std::string getLastError() const;
```

### 数据验证
```cpp
bool isDataValid(const std::string& entityType, const std::string& entityId);
std::vector<std::string> getInvalidData();
bool repairData(const std::string& entityType, const std::string& entityId);
```

## 最佳实践

### 1. 初始化
- 在应用启动时初始化
- 配置合适的自动保存间隔
- 启用数据验证

### 2. 数据操作
- 使用异步接口处理大量数据
- 定期清理缓存
- 监控统计信息

### 3. 错误处理
- 检查操作返回值
- 处理异常情况
- 记录错误日志

### 4. 性能优化
- 合理设置缓存大小
- 使用批量操作
- 避免频繁的小数据操作

## 示例代码

完整的使用示例请参考：
- `examples/data_persistence_example.cpp` - 基本使用示例
- `test/test_data_persistence.cpp` - 测试代码

## 依赖项

- DatabaseManager - 数据库管理
- Character - 角色类
- Item - 物品类
- Quest - 任务类
- Level - 关卡类
- Skill - 技能类

## 注意事项

1. **线程安全**: 所有公共接口都是线程安全的
2. **内存管理**: 使用智能指针管理对象生命周期
3. **异常处理**: 所有操作都有异常保护
4. **资源清理**: 调用shutdown()确保资源正确释放
5. **数据一致性**: 使用事务确保数据一致性

## 版本历史

- v1.0.0 (2025.06.17) - 初始版本
  - 基本数据持久化功能
  - 自动保存机制
  - 缓存管理
  - 事件系统 