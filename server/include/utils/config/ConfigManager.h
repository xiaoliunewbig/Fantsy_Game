/**
 * @file ConfigManager.h
 * @brief 配置管理器 - 统一管理系统配置
 * @author [pengchengkang]
 * @date 2025.06.17
 * 
 * 功能特性:
 * - 支持多种配置格式 (JSON, XML, YAML, INI, TOML)
 * - 分层配置管理 (系统级、应用级、用户级)
 * - 配置热重载
 * - 配置验证和默认值
 * - 配置变更通知
 * - 配置加密支持
 * - 配置备份和恢复
 * - 游戏特定配置 (角色、关卡、物品、技能、任务)
 * - 动态配置生成
 */

#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <filesystem>
#include <variant>
#include <mutex>
#include <atomic>
#include <chrono>

namespace Fantasy {

// 配置值类型
using ConfigValue = std::variant<std::string, int, double, bool, std::vector<std::string>>;

// 配置层级
enum class ConfigLevel {
    SYSTEM,     // 系统级配置 (只读)
    APPLICATION, // 应用级配置 (可写)
    USER        // 用户级配置 (可写)
};

// 配置格式
enum class ConfigFormat {
    JSON,
    XML,
    YAML,
    INI,
    TOML,
    BINARY,
    UNKNOWN
};

// 配置项定义
struct ConfigItem {
    std::string key;
    ConfigValue defaultValue;
    std::string description;
    bool required;
    bool encrypted;
    std::vector<std::string> validValues; // 可选值列表
    std::function<bool(const ConfigValue&)> validator; // 自定义验证函数
};

// 配置变更事件
struct ConfigChangeEvent {
    std::string key;
    ConfigValue oldValue;
    ConfigValue newValue;
    ConfigLevel level;
    std::chrono::system_clock::time_point timestamp;
};

// 配置监听器
using ConfigChangeListener = std::function<void(const ConfigChangeEvent&)>;

// 配置管理器统计信息
struct ConfigManagerStats {
    size_t totalConfigs;
    size_t loadedConfigs;
    size_t modifiedConfigs;
    size_t listenersCount;
    std::chrono::milliseconds lastReloadTime;
    std::chrono::milliseconds averageLoadTime;
};

// 应用游戏配置结构
struct AppGameConfig {
    std::string version;
    std::string language;
    bool fullscreen;
    struct Resolution {
        int width;
        int height;
    } resolution;
    struct Volume {
        int master;
        int music;
        int sfx;
        int voice;
    } volume;
    struct Graphics {
        std::string quality;
        bool shadows;
        bool antialiasing;
        bool vsync;
    } graphics;
};

// 系统配置结构
struct SystemConfig {
    bool autoSave;
    int autoSaveInterval;
    int maxSaveSlots;
    std::string logLevel;
    bool debugMode;
};

/**
 * @brief 配置管理器主类
 */
class ConfigManager {
public:
    // 单例模式
    static ConfigManager& getInstance();
    
    // 禁用拷贝和赋值
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    // 初始化和清理
    bool initialize(const std::filesystem::path& configDir = "config/");
    void shutdown();
    
    // 配置加载和保存
    bool loadConfig(const std::string& configName, ConfigLevel level = ConfigLevel::APPLICATION);
    bool saveConfig(const std::string& configName, ConfigLevel level = ConfigLevel::APPLICATION);
    bool reloadConfig(const std::string& configName);
    bool reloadAllConfigs();
    
    // 配置值访问
    template<typename T>
    T getValue(const std::string& key, const T& defaultValue = T{}) const;
    
    template<typename T>
    bool setValue(const std::string& key, const T& value, ConfigLevel level = ConfigLevel::APPLICATION);
    
    // 配置项管理
    bool hasKey(const std::string& key) const;
    std::vector<std::string> getAllKeys() const;
    bool removeKey(const std::string& key, ConfigLevel level = ConfigLevel::APPLICATION);
    
    // 配置验证
    bool validateConfig(const std::string& configName) const;
    bool validateKey(const std::string& key, const ConfigValue& value) const;
    
    // 配置监听
    void addChangeListener(const std::string& key, ConfigChangeListener listener);
    void removeChangeListener(const std::string& key, const ConfigChangeListener& listener);
    void removeAllListeners();
    
    // 配置定义
    void defineConfig(const std::string& configName, const std::vector<ConfigItem>& items);
    std::vector<ConfigItem> getConfigDefinition(const std::string& configName) const;
    
    // 配置备份和恢复
    bool backupConfig(const std::string& configName, const std::filesystem::path& backupPath);
    bool restoreConfig(const std::string& configName, const std::filesystem::path& backupPath);
    
    // 配置加密
    bool encryptConfig(const std::string& configName, const std::string& password);
    bool decryptConfig(const std::string& configName, const std::string& password);
    
    // 配置导入导出
    bool exportConfig(const std::string& configName, const std::filesystem::path& exportPath, ConfigFormat format);
    bool importConfig(const std::string& configName, const std::filesystem::path& importPath);
    
    // 统计信息
    ConfigManagerStats getStats() const;
    void resetStats();
    
    // 工具方法
    ConfigFormat detectFormat(const std::filesystem::path& path) const;
    std::string getConfigPath(const std::string& configName, ConfigLevel level) const;
    bool isConfigModified(const std::string& configName) const;
    
    // 热重载控制
    void enableHotReload(bool enable);
    bool isHotReloadEnabled() const;
    
    // 调试和诊断
    void dumpConfig(const std::string& configName, std::ostream& output) const;
    void validateAllConfigs() const;

    // === 游戏特定配置功能 ===
    
    // 游戏配置
    void setGameConfig(const std::string& key, const ConfigValue& value);
    ConfigValue getGameConfig(const std::string& key, const ConfigValue& defaultValue = ConfigValue{}) const;
    AppGameConfig getGameConfig() const;
    void setGameConfig(const AppGameConfig& config);
    
    // 用户配置
    void setUserConfig(const std::string& key, const ConfigValue& value);
    ConfigValue getUserConfig(const std::string& key, const ConfigValue& defaultValue = ConfigValue{}) const;
    
    // 系统配置
    void setSystemConfig(const std::string& key, const ConfigValue& value);
    ConfigValue getSystemConfig(const std::string& key, const ConfigValue& defaultValue = ConfigValue{}) const;
    SystemConfig getSystemConfig() const;
    void setSystemConfig(const SystemConfig& config);
    
    // 角色配置
    std::unordered_map<std::string, ConfigValue> loadCharacterConfig(const std::string& characterType, int level);
    std::unordered_map<std::string, ConfigValue> loadCharacterConfigFromFile(const std::filesystem::path& filePath);
    bool saveCharacterConfig(const std::string& characterType, int level, const std::unordered_map<std::string, ConfigValue>& config);
    
    // 关卡配置
    std::unordered_map<std::string, ConfigValue> loadLevelConfig(int levelId);
    std::unordered_map<std::string, ConfigValue> loadLevelConfigFromFile(const std::filesystem::path& filePath);
    bool saveLevelConfig(int levelId, const std::unordered_map<std::string, ConfigValue>& config);
    
    // 物品配置
    std::unordered_map<std::string, ConfigValue> loadItemConfig(const std::string& itemId);
    std::unordered_map<std::string, ConfigValue> loadItemConfigFromFile(const std::filesystem::path& filePath);
    bool saveItemConfig(const std::string& itemId, const std::unordered_map<std::string, ConfigValue>& config);
    
    // 技能配置
    std::unordered_map<std::string, ConfigValue> loadSkillConfig(const std::string& skillId);
    std::unordered_map<std::string, ConfigValue> loadSkillConfigFromFile(const std::filesystem::path& filePath);
    bool saveSkillConfig(const std::string& skillId, const std::unordered_map<std::string, ConfigValue>& config);
    
    // 任务配置
    std::unordered_map<std::string, ConfigValue> loadQuestConfig(const std::string& questId);
    std::unordered_map<std::string, ConfigValue> loadQuestConfigFromFile(const std::filesystem::path& filePath);
    bool saveQuestConfig(const std::string& questId, const std::unordered_map<std::string, ConfigValue>& config);
    
    // 动态配置生成
    std::unordered_map<std::string, ConfigValue> generateCharacterConfig(const std::string& characterType, int level);
    std::unordered_map<std::string, ConfigValue> generateLevelConfig(int levelId, float difficulty = 1.0f);
    std::unordered_map<std::string, ConfigValue> generateItemConfig(const std::string& itemType, int level);
    std::unordered_map<std::string, ConfigValue> generateSkillConfig(const std::string& skillType, int level);
    
    // 配置模板
    std::unordered_map<std::string, ConfigValue> getConfigTemplate(const std::string& configType);
    bool applyConfigTemplate(std::unordered_map<std::string, ConfigValue>& config, const std::string& templateType);

private:
    ConfigManager();
    ~ConfigManager();
    
    // 内部实现
    class Impl;
    std::unique_ptr<Impl> pImpl_;
    
    // 私有方法
    bool loadConfigFile(const std::filesystem::path& path, ConfigFormat format);
    bool saveConfigFile(const std::filesystem::path& path, ConfigFormat format);
    void notifyListeners(const std::string& key, const ConfigValue& oldValue, const ConfigValue& newValue);
    ConfigValue getValueInternal(const std::string& key) const;
    bool setValueInternal(const std::string& key, const ConfigValue& value, ConfigLevel level);
    std::string encryptValue(const std::string& value, const std::string& password) const;
    std::string decryptValue(const std::string& encryptedValue, const std::string& password) const;
    
    // 游戏配置私有方法
    void setupDefaultGameConfig();
    void setupDefaultSystemConfig();
    void setupConfigTemplates();
    std::filesystem::path getGameConfigPath(const std::string& configType, const std::string& id = "") const;
};

// 配置管理器工具类
class ConfigUtils {
public:
    // 配置值转换
    static std::string toString(const ConfigValue& value);
    static int toInt(const ConfigValue& value, int defaultValue = 0);
    static double toDouble(const ConfigValue& value, double defaultValue = 0.0);
    static bool toBool(const ConfigValue& value, bool defaultValue = false);
    static std::vector<std::string> toStringArray(const ConfigValue& value);
    
    // 配置值创建
    static ConfigValue createValue(const std::string& value);
    static ConfigValue createValue(int value);
    static ConfigValue createValue(double value);
    static ConfigValue createValue(bool value);
    static ConfigValue createValue(const std::vector<std::string>& value);
    
    // 配置验证
    static bool isValidInt(const std::string& value);
    static bool isValidDouble(const std::string& value);
    static bool isValidBool(const std::string& value);
    static bool isValidPath(const std::string& value);
    static bool isValidEmail(const std::string& value);
    static bool isValidIP(const std::string& value);
    
    // 配置格式化
    static std::string formatConfigValue(const ConfigValue& value);
    static std::string formatConfigKey(const std::string& key);
    static std::string formatConfigPath(const std::filesystem::path& path);
};

// 配置管理器异常类
class ConfigException : public std::runtime_error {
public:
    explicit ConfigException(const std::string& message) : std::runtime_error(message) {}
    explicit ConfigException(const char* message) : std::runtime_error(message) {}
};

// 配置管理器宏定义
#define CONFIG_GET_VALUE(key, defaultValue) \
    Fantasy::ConfigManager::getInstance().getValue<decltype(defaultValue)>(key, defaultValue)

#define CONFIG_SET_VALUE(key, value) \
    Fantasy::ConfigManager::getInstance().setValue(key, value)

#define CONFIG_HAS_KEY(key) \
    Fantasy::ConfigManager::getInstance().hasKey(key)

#define CONFIG_RELOAD() \
    Fantasy::ConfigManager::getInstance().reloadAllConfigs()

// 配置日志宏
#define CONFIG_LOG_TRACE(msg, ...) \
    FANTASY_LOG_TRACE("[ConfigManager] " msg, ##__VA_ARGS__)

#define CONFIG_LOG_DEBUG(msg, ...) \
    FANTASY_LOG_DEBUG("[ConfigManager] " msg, ##__VA_ARGS__)

#define CONFIG_LOG_INFO(msg, ...) \
    FANTASY_LOG_INFO("[ConfigManager] " msg, ##__VA_ARGS__)

#define CONFIG_LOG_WARN(msg, ...) \
    FANTASY_LOG_WARN("[ConfigManager] " msg, ##__VA_ARGS__)

#define CONFIG_LOG_ERROR(msg, ...) \
    FANTASY_LOG_ERROR("[ConfigManager] " msg, ##__VA_ARGS__)

} // namespace Fantasy 