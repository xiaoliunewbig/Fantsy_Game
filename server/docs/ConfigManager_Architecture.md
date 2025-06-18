# Fantasy Legend 配置管理器架构文档

## 概述

配置管理器（ConfigManager）是 Fantasy Legend 基础框架的核心组件之一，与资源管理系统（ResourceSystem）和日志系统（Logger）并列，共同构成了游戏的基础架构。

## 架构设计

### 三大核心系统关系

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   日志系统      │    │   配置管理器    │    │   资源管理      │
│   (Logger)      │    │ (ConfigManager) │    │ (ResourceSystem)│
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         │                       │                       │
         └───────────────────────┼───────────────────────┘
                                 │
                    ┌─────────────────┐
                    │   游戏引擎      │
                    │   (GameEngine)  │
                    └─────────────────┘
```

### 系统职责分工

1. **日志系统（Logger）**
   - 提供统一的日志记录功能
   - 支持多种日志输出方式（控制台、文件、网络）
   - 提供不同级别的日志记录
   - 支持异步日志记录

2. **配置管理器（ConfigManager）**
   - 统一管理系统配置
   - 支持多种配置格式（JSON、XML、YAML、INI、TOML）
   - 提供配置热重载功能
   - 支持配置变更监听
   - 提供配置验证和默认值

3. **资源管理系统（ResourceSystem）**
   - 管理游戏资源（纹理、模型、音频等）
   - 提供资源加载、缓存、压缩功能
   - 支持异步资源加载
   - 提供资源版本控制

## 配置管理器特性

### 1. 分层配置管理

```cpp
enum class ConfigLevel {
    SYSTEM,     // 系统级配置 (只读)
    APPLICATION, // 应用级配置 (可写)
    USER        // 用户级配置 (可写)
};
```

- **系统级配置**: 游戏引擎核心配置，运行时不可修改
- **应用级配置**: 游戏应用配置，可在运行时修改
- **用户级配置**: 用户个性化配置，优先级最高

### 2. 多格式支持

```cpp
enum class ConfigFormat {
    JSON,    // JSON格式
    XML,     // XML格式
    YAML,    // YAML格式
    INI,     // INI格式
    TOML,    // TOML格式
    BINARY,  // 二进制格式
    UNKNOWN  // 未知格式
};
```

### 3. 配置值类型

```cpp
using ConfigValue = std::variant<
    std::string,           // 字符串
    int,                   // 整数
    double,                // 浮点数
    bool,                  // 布尔值
    std::vector<std::string> // 字符串数组
>;
```

### 4. 配置变更监听

```cpp
struct ConfigChangeEvent {
    std::string key;                                    // 配置键
    ConfigValue oldValue;                               // 旧值
    ConfigValue newValue;                               // 新值
    ConfigLevel level;                                  // 配置级别
    std::chrono::system_clock::time_point timestamp;   // 时间戳
};
```

## 与资源管理系统的集成

### 1. 资源配置管理

配置管理器可以控制资源管理系统的行为：

```cpp
// 资源配置示例
{
    "texture.quality": "high",           // 纹理质量
    "model.lod_level": 2,               // 模型LOD级别
    "audio.format": "ogg",              // 音频格式
    "compression.enabled": true,        // 启用压缩
    "compression.level": 6,             // 压缩级别
    "cache.max_size": 100,              // 缓存大小
    "async_loading.enabled": true,      // 启用异步加载
    "async_loading.threads": 4          // 加载线程数
}
```

### 2. 动态配置调整

```cpp
// 根据配置调整资源系统
std::string textureQuality = CONFIG_GET_VALUE("texture.quality", "medium");
bool compressionEnabled = CONFIG_GET_VALUE("compression.enabled", false);
int compressionLevel = CONFIG_GET_VALUE("compression.level", 6);

if (compressionEnabled) {
    resourceSystem.setCompressionLevel(compressionLevel);
}
```

### 3. 性能优化配置

```cpp
// 性能相关配置
{
    "performance.memory_limit": 2048,    // 内存限制(MB)
    "performance.cache_strategy": "lru", // 缓存策略
    "performance.preload_enabled": true, // 启用预加载
    "performance.compression_ratio": 0.7 // 压缩比例
}
```

## 与日志系统的集成

### 1. 日志配置管理

配置管理器控制日志系统的行为：

```cpp
// 日志配置示例
{
    "logging.level": "info",             // 日志级别
    "logging.console_enabled": true,     // 启用控制台输出
    "logging.file_enabled": true,        // 启用文件输出
    "logging.file_path": "logs/",        // 日志文件路径
    "logging.max_file_size": 10,         // 最大文件大小(MB)
    "logging.rotation_enabled": true,    // 启用日志轮转
    "logging.async_enabled": true        // 启用异步日志
}
```

### 2. 配置变更日志

```cpp
// 配置变更时自动记录日志
configManager.addChangeListener("logging.level", [](const ConfigChangeEvent& event) {
    std::string oldLevel = ConfigUtils::toString(event.oldValue);
    std::string newLevel = ConfigUtils::toString(event.newValue);
    FANTASY_LOG_INFO("日志级别变更: {} -> {}", oldLevel, newLevel);
});
```

### 3. 调试配置

```cpp
// 调试相关配置
{
    "debug.enabled": true,               // 启用调试模式
    "debug.verbose_logging": true,       // 详细日志
    "debug.performance_monitoring": true, // 性能监控
    "debug.resource_tracking": true      // 资源跟踪
}
```

## 使用示例

### 1. 基本使用

```cpp
#include "include/utils/config/ConfigManager.h"

int main() {
    // 初始化配置管理器
    auto& configManager = ConfigManager::getInstance();
    configManager.initialize("config/");
    
    // 加载配置文件
    configManager.loadConfig("game");
    configManager.loadConfig("resources");
    
    // 读取配置值
    std::string serverIP = CONFIG_GET_VALUE("server.ip", "127.0.0.1");
    int serverPort = CONFIG_GET_VALUE("server.port", 8080);
    bool debugMode = CONFIG_GET_VALUE("game.debug_mode", false);
    
    // 设置配置值
    CONFIG_SET_VALUE("audio.master_volume", 0.8);
    CONFIG_SET_VALUE("graphics.resolution", "1920x1080");
    
    return 0;
}
```

### 2. 配置监听

```cpp
// 监听配置变更
configManager.addChangeListener("audio.master_volume", [](const ConfigChangeEvent& event) {
    double oldVolume = ConfigUtils::toDouble(event.oldValue, 1.0);
    double newVolume = ConfigUtils::toDouble(event.newValue, 1.0);
    
    // 更新音频系统
    audioSystem.setMasterVolume(newVolume);
    
    // 记录日志
    FANTASY_LOG_INFO("主音量已更新: {} -> {}", oldVolume, newVolume);
});
```

### 3. 配置验证

```cpp
// 配置验证
if (!ConfigUtils::isValidIP(serverIP)) {
    FANTASY_LOG_ERROR("无效的服务器IP地址: {}", serverIP);
    return -1;
}

if (serverPort < 1 || serverPort > 65535) {
    FANTASY_LOG_ERROR("无效的端口号: {}", serverPort);
    return -1;
}
```

### 4. 热重载

```cpp
// 启用热重载
configManager.enableHotReload(true);

// 配置文件变更时自动重新加载
if (configManager.reloadConfig("game")) {
    FANTASY_LOG_INFO("游戏配置已重新加载");
    
    // 读取更新后的配置
    std::string newServerIP = CONFIG_GET_VALUE("server.ip", "127.0.0.1");
    std::cout << "服务器IP已更新: " << newServerIP << std::endl;
}
```

## 性能优化

### 1. 内存管理

- 使用智能指针管理配置对象
- 实现配置值的内存池
- 优化配置键的字符串存储

### 2. 缓存策略

- 缓存已解析的配置值
- 使用哈希表快速查找配置项
- 实现配置变更的增量更新

### 3. 并发安全

- 使用互斥锁保护配置数据
- 支持多线程安全访问
- 实现无锁的配置读取

## 扩展性设计

### 1. 插件化架构

```cpp
// 自定义配置格式支持
class CustomConfigParser : public IConfigParser {
public:
    bool parse(const std::string& content, ConfigMap& config) override;
    std::string serialize(const ConfigMap& config) override;
};

// 注册自定义解析器
configManager.registerParser("custom", std::make_shared<CustomConfigParser>());
```

### 2. 配置继承

```cpp
// 支持配置继承
{
    "base_config": "default.cfg",        // 基础配置
    "user_config": "user.cfg",           // 用户配置
    "override_config": "override.cfg"    // 覆盖配置
}
```

### 3. 环境变量支持

```cpp
// 支持环境变量
{
    "database.url": "${DB_URL}",         // 从环境变量读取
    "api.key": "${API_KEY}",             // 从环境变量读取
    "debug.mode": "${DEBUG_MODE}"        // 从环境变量读取
}
```

## 最佳实践

### 1. 配置组织

```
config/
├── system/           # 系统级配置
│   ├── engine.cfg
│   └── network.cfg
├── application/      # 应用级配置
│   ├── game.cfg
│   ├── graphics.cfg
│   └── audio.cfg
└── user/            # 用户级配置
    ├── settings.cfg
    └── preferences.cfg
```

### 2. 配置命名规范

- 使用点分隔的层次结构：`category.subcategory.key`
- 使用小写字母和下划线
- 避免使用特殊字符
- 保持命名一致性

### 3. 配置文档

- 为每个配置项提供详细说明
- 指定配置值的有效范围
- 提供配置示例
- 记录配置变更历史

## 总结

配置管理器作为 Fantasy Legend 基础框架的核心组件，与资源管理系统和日志系统紧密集成，为游戏提供了灵活、高效的配置管理能力。通过分层配置、多格式支持、热重载等特性，配置管理器能够满足游戏开发中的各种配置需求，同时保持良好的性能和扩展性。 