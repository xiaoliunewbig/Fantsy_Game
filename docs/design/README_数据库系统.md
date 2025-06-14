# 《幻境传说》数据库系统

## 概述

《幻境传说》数据库系统是一个完整的游戏数据管理解决方案，为游戏提供高效、可靠的数据存储和查询功能。系统采用SQLite数据库，支持所有游戏核心功能，包括角色管理、技能系统、装备系统、任务系统、关卡系统等。

## 系统架构

### 核心组件

1. **数据库设计文档** (`docs/数据库设计.md`)
   - 完整的数据库表结构设计
   - 表关系图和优化策略
   - 数据完整性约束

2. **数据库初始化脚本** (`GAME/resources/scripts/database_init.sql`)
   - 完整的SQL建表语句
   - 索引、视图、触发器定义
   - 默认数据插入

3. **C++数据库管理类** (`GAME/src/data/DatabaseManager.h`)
   - 高级数据库操作接口
   - 查询构建器
   - 事务管理
   - 错误处理

4. **Python数据生成器** (`python/database_seeder.py`)
   - 示例数据生成
   - 数据库初始化
   - 统计信息显示

5. **数据库测试工具** (`python/test_database.py`)
   - 功能测试
   - 性能测试
   - 数据质量检查

6. **使用指南** (`docs/数据库使用指南.md`)
   - 详细的使用说明
   - 代码示例
   - 最佳实践

## 数据库表结构

### 核心表 (7个)
- **characters** - 角色基本信息
- **skills** - 技能定义
- **equipment** - 装备定义
- **items** - 物品定义
- **quests** - 任务定义
- **levels** - 关卡定义
- **achievements** - 成就定义

### 关联表 (6个)
- **character_skills** - 角色技能关联
- **character_equipment** - 角色装备关联
- **character_inventory** - 角色背包
- **character_quests** - 角色任务进度
- **character_level_progress** - 角色关卡进度
- **character_achievements** - 角色成就解锁

### 记录表 (6个)
- **battle_records** - 战斗记录
- **status_effects** - 状态效果
- **save_data** - 游戏存档
- **statistics** - 统计数据
- **config** - 配置数据
- **game_logs** - 游戏日志

## 快速开始

### 1. 初始化数据库

```bash
# 使用Python脚本初始化
python python/database_seeder.py game_data.db init

# 或者直接执行SQL脚本
sqlite3 game_data.db < GAME/resources/scripts/database_init.sql
```

### 2. 生成示例数据

```bash
# 生成所有示例数据
python python/database_seeder.py game_data.db seed

# 完整流程（初始化+生成数据+统计）
python python/database_seeder.py game_data.db full
```

### 3. 测试数据库功能

```bash
# 运行功能测试
python python/test_database.py game_data.db
```

### 4. 在C++代码中使用

```cpp
#include "data/DatabaseManager.h"

// 初始化数据库
DatabaseManager::instance().initialize("game_data.db");

// 创建角色
auto characterId = DatabaseManager::instance().createCharacter({
    {"name", "玩家名"},
    {"class", "warrior"},
    {"level", 1}
});

// 查询角色
auto character = DatabaseManager::instance().getCharacter(characterId);

// 更新角色等级
DatabaseManager::instance().updateCharacterLevel(characterId, 10);
```

## 主要功能

### 角色管理
- 创建、查询、更新、删除角色
- 角色属性管理
- 角色等级和经验值管理
- 角色职业系统

### 技能系统
- 技能学习和升级
- 技能装备和卸下
- 技能效果管理
- 职业技能限制

### 装备系统
- 装备获取和装备
- 装备耐久度管理
- 装备强化和附魔
- 装备类型和槽位管理

### 任务系统
- 任务接受和完成
- 任务进度跟踪
- 任务奖励发放
- 任务类型和章节管理

### 关卡系统
- 关卡解锁和完成
- 关卡评分和星级
- 关卡难度管理
- 关卡奖励系统

### 成就系统
- 成就解锁和进度
- 成就条件检查
- 成就奖励发放
- 成就统计

### 战斗系统
- 战斗记录保存
- 战斗统计
- 战斗奖励
- 战斗日志

### 存档系统
- 游戏存档保存
- 存档加载
- 多存档槽位
- 存档数据完整性

## 性能优化

### 索引优化
- 为常用查询字段创建索引
- 复合索引优化
- 查询计划分析

### 查询优化
- 使用查询构建器
- 避免全表扫描
- 批量操作优化
- 连接查询优化

### 数据优化
- 数据压缩
- 定期清理
- 数据备份
- 性能监控

## 错误处理

### 数据库连接错误
- 连接失败处理
- 重连机制
- 连接池管理

### 查询错误
- SQL语法错误
- 数据不存在
- 约束违反
- 事务回滚

### 数据完整性
- 外键约束检查
- 数据一致性验证
- 数据质量检查
- 异常数据修复

## 监控和维护

### 日志记录
- 操作日志
- 错误日志
- 性能日志
- 安全日志

### 性能监控
- 查询性能
- 连接状态
- 资源使用
- 响应时间

### 数据备份
- 自动备份
- 增量备份
- 备份验证
- 恢复测试

## 扩展开发

### 添加新表
1. 在SQL脚本中添加表结构
2. 在DatabaseManager中添加操作方法
3. 更新数据生成器
4. 添加测试用例

### 添加新功能
1. 在DatabaseManager中添加新方法
2. 实现业务逻辑
3. 添加错误处理
4. 编写文档

### 自定义查询
1. 使用查询构建器
2. 创建复杂查询
3. 优化查询性能
4. 测试查询结果

## 最佳实践

### 代码规范
- 使用DatabaseManager进行所有数据库操作
- 正确处理事务
- 及时释放资源
- 记录错误日志

### 性能优化
- 使用索引字段查询
- 避免N+1查询问题
- 批量操作数据
- 定期优化数据库

### 数据安全
- 参数化查询防止SQL注入
- 数据验证和清理
- 权限控制
- 敏感数据加密

### 测试策略
- 单元测试
- 集成测试
- 性能测试
- 数据质量测试

## 故障排除

### 常见问题

1. **数据库连接失败**
   - 检查文件路径
   - 检查文件权限
   - 检查磁盘空间

2. **查询性能慢**
   - 检查索引使用
   - 优化查询语句
   - 分析查询计划

3. **数据不一致**
   - 检查外键约束
   - 验证数据完整性
   - 修复异常数据

4. **内存使用过高**
   - 检查连接池
   - 优化查询结果
   - 及时释放资源

### 调试工具

1. **SQLite命令行工具**
   ```bash
   sqlite3 game_data.db
   .schema
   .tables
   SELECT * FROM characters LIMIT 5;
   ```

2. **Python调试脚本**
   ```bash
   python python/test_database.py game_data.db
   ```

3. **C++调试输出**
   ```cpp
   qDebug() << "Database operation result:" << result;
   ```

## 版本历史

### v1.0.0 (当前版本)
- 完整的数据库表结构设计
- DatabaseManager类实现
- Python数据生成器
- 测试工具
- 使用文档

### 计划功能
- 数据库迁移工具
- 性能分析工具
- 数据可视化工具
- 自动化测试套件

## 贡献指南

1. Fork项目
2. 创建功能分支
3. 提交更改
4. 创建Pull Request
5. 代码审查
6. 合并到主分支

## 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 联系方式

如有问题或建议，请通过以下方式联系：
- 提交Issue
- 发送邮件
- 参与讨论

---

**注意**: 本数据库系统专为《幻境传说》游戏设计，如需用于其他项目，请根据具体需求进行适配。 