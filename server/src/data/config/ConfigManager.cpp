#include "ConfigManager.h"
#include "Logger.h"
#include "FileUtils.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QDateTime>
#include <QStandardPaths>
#include <QApplication>

namespace py = pybind11;

ConfigManager* ConfigManager::s_instance = nullptr;

ConfigManager::ConfigManager() 
    : m_pythonInitialized(false)
    , m_hotReloadEnabled(false) {
    
    Logger::instance()->log("ConfigManager", "ConfigManager instance created");
    
    // 初始化配置路径
    m_configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config";
    m_defaultConfigPath = m_configDir + "/default_config.json";
    m_userConfigPath = m_configDir + "/user_config.json";
    m_systemConfigPath = m_configDir + "/system_config.json";
    
    // 创建配置目录
    QDir().mkpath(m_configDir);
    
    // 初始化Python
    initializePython();
    
    // 设置默认配置
    setupDefaultConfigs();
    
    // 设置热重载
    setupHotReload();
    
    // 加载配置
    loadDefaultConfig();
}

ConfigManager::~ConfigManager() {
    Logger::instance()->log("ConfigManager", "ConfigManager instance destroyed");
}

ConfigManager* ConfigManager::instance() {
    if (!s_instance) {
        s_instance = new ConfigManager();
    }
    return s_instance;
}

void ConfigManager::initializePython() {
    try {
        // 初始化Python解释器
        py::initialize_interpreter();
        
        // 导入配置生成模块
        m_configGenerator = py::module_::import("config_generator");
        
        m_pythonInitialized = true;
        Logger::instance()->log("ConfigManager", "Python initialized successfully");
    } catch (const std::exception& e) {
        Logger::instance()->log("ConfigManager", QString("Failed to initialize Python: %1").arg(e.what()));
        m_pythonInitialized = false;
    }
}

void ConfigManager::setupDefaultConfigs() {
    // 游戏默认配置
    m_gameConfig["version"] = "1.0.0";
    m_gameConfig["language"] = "zh_CN";
    m_gameConfig["fullscreen"] = false;
    m_gameConfig["resolution"] = QVariantMap{{"width", 1920}, {"height", 1080}};
    m_gameConfig["volume"] = QVariantMap{
        {"master", 100},
        {"music", 80},
        {"sfx", 90},
        {"voice", 85}
    };
    m_gameConfig["graphics"] = QVariantMap{
        {"quality", "high"},
        {"shadows", true},
        {"antialiasing", true},
        {"vsync", true}
    };
    
    // 系统默认配置
    m_systemConfig["auto_save"] = true;
    m_systemConfig["auto_save_interval"] = 300; // 5分钟
    m_systemConfig["max_save_slots"] = 10;
    m_systemConfig["log_level"] = "info";
    m_systemConfig["debug_mode"] = false;
}

void ConfigManager::setupHotReload() {
    if (m_hotReloadEnabled) {
        // 设置文件监视器
        auto watcher = new QFileSystemWatcher(this);
        watcher->addPath(m_configDir);
        
        connect(watcher, &QFileSystemWatcher::directoryChanged, 
                this, &ConfigManager::onConfigFileChanged);
    }
}

// 配置管理方法
void ConfigManager::setConfig(const QString& key, const QVariant& value) {
    QMutexLocker locker(&m_configMutex);
    m_dynamicConfig[key] = value;
    emit configChanged(key, value);
}

QVariant ConfigManager::getConfig(const QString& key, const QVariant& defaultValue) {
    QMutexLocker locker(&m_configMutex);
    
    // 按优先级查找配置
    if (m_dynamicConfig.contains(key)) {
        return m_dynamicConfig[key];
    }
    if (m_userConfig.contains(key)) {
        return m_userConfig[key];
    }
    if (m_gameConfig.contains(key)) {
        return m_gameConfig[key];
    }
    if (m_systemConfig.contains(key)) {
        return m_systemConfig[key];
    }
    
    return defaultValue;
}

bool ConfigManager::hasConfig(const QString& key) const {
    QMutexLocker locker(&m_configMutex);
    return m_dynamicConfig.contains(key) || 
           m_userConfig.contains(key) || 
           m_gameConfig.contains(key) || 
           m_systemConfig.contains(key);
}

void ConfigManager::removeConfig(const QString& key) {
    QMutexLocker locker(&m_configMutex);
    m_dynamicConfig.remove(key);
    m_userConfig.remove(key);
    m_gameConfig.remove(key);
    m_systemConfig.remove(key);
}

QStringList ConfigManager::getAllConfigKeys() const {
    QMutexLocker locker(&m_configMutex);
    QStringList keys;
    keys << m_dynamicConfig.keys();
    keys << m_userConfig.keys();
    keys << m_gameConfig.keys();
    keys << m_systemConfig.keys();
    return keys;
}

// 配置文件操作
bool ConfigManager::loadConfigFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        Logger::instance()->log("ConfigManager", QString("Failed to open config file: %1").arg(filePath));
        return false;
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (doc.isNull()) {
        Logger::instance()->log("ConfigManager", QString("Invalid JSON in config file: %1").arg(filePath));
        return false;
    }
    
    QVariantMap config = doc.object().toVariantMap();
    
    // 根据文件路径决定加载到哪个配置组
    if (filePath.contains("user")) {
        m_userConfig = config;
    } else if (filePath.contains("system")) {
        m_systemConfig = config;
    } else {
        m_gameConfig = config;
    }
    
    emit configLoaded(filePath);
    return true;
}

bool ConfigManager::saveConfigFile(const QString& filePath) {
    QVariantMap config;
    
    if (filePath.contains("user")) {
        config = m_userConfig;
    } else if (filePath.contains("system")) {
        config = m_systemConfig;
    } else {
        config = m_gameConfig;
    }
    
    QJsonDocument doc = QJsonDocument::fromVariant(config);
    QFile file(filePath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        Logger::instance()->log("ConfigManager", QString("Failed to save config file: %1").arg(filePath));
        return false;
    }
    
    file.write(doc.toJson());
    emit configSaved(filePath);
    return true;
}

bool ConfigManager::loadDefaultConfig() {
    bool success = true;
    success &= loadConfigFile(m_defaultConfigPath);
    success &= loadConfigFile(m_userConfigPath);
    success &= loadConfigFile(m_systemConfigPath);
    return success;
}

bool ConfigManager::saveDefaultConfig() {
    bool success = true;
    success &= saveConfigFile(m_defaultConfigPath);
    success &= saveConfigFile(m_userConfigPath);
    success &= saveConfigFile(m_systemConfigPath);
    return success;
}

// 游戏配置方法
void ConfigManager::setGameConfig(const QString& key, const QVariant& value) {
    QMutexLocker locker(&m_configMutex);
    m_gameConfig[key] = value;
    emit configChanged(key, value);
}

QVariant ConfigManager::getGameConfig(const QString& key, const QVariant& defaultValue) {
    QMutexLocker locker(&m_configMutex);
    return m_gameConfig.value(key, defaultValue);
}

// 用户配置方法
void ConfigManager::setUserConfig(const QString& key, const QVariant& value) {
    QMutexLocker locker(&m_configMutex);
    m_userConfig[key] = value;
    emit configChanged(key, value);
}

QVariant ConfigManager::getUserConfig(const QString& key, const QVariant& defaultValue) {
    QMutexLocker locker(&m_configMutex);
    return m_userConfig.value(key, defaultValue);
}

// 系统配置方法
void ConfigManager::setSystemConfig(const QString& key, const QVariant& value) {
    QMutexLocker locker(&m_configMutex);
    m_systemConfig[key] = value;
    emit configChanged(key, value);
}

QVariant ConfigManager::getSystemConfig(const QString& key, const QVariant& defaultValue) {
    QMutexLocker locker(&m_configMutex);
    return m_systemConfig.value(key, defaultValue);
}

// 角色配置方法
QVariantMap ConfigManager::loadCharacterConfig(const QString& characterType, int level) {
    QString configPath = QString("%1/characters/%2_%3.json").arg(m_configDir, characterType, QString::number(level));
    
    if (QFile::exists(configPath)) {
        return loadCharacterConfigFromFile(configPath);
    } else {
        return generateCharacterConfig(characterType, level);
    }
}

QVariantMap ConfigManager::loadCharacterConfigFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        Logger::instance()->log("ConfigManager", QString("Failed to load character config: %1").arg(filePath));
        return QVariantMap();
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    return doc.object().toVariantMap();
}

bool ConfigManager::saveCharacterConfig(const QString& characterType, int level, const QVariantMap& config) {
    QString configPath = QString("%1/characters/%2_%3.json").arg(m_configDir, characterType, QString::number(level));
    
    QDir().mkpath(QFileInfo(configPath).absolutePath());
    
    QJsonDocument doc = QJsonDocument::fromVariant(config);
    QFile file(configPath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        Logger::instance()->log("ConfigManager", QString("Failed to save character config: %1").arg(configPath));
        return false;
    }
    
    file.write(doc.toJson());
    return true;
}

// 关卡配置方法
QVariantMap ConfigManager::loadLevelConfig(int levelId) {
    QString configPath = QString("%1/levels/level_%2.json").arg(m_configDir, QString::number(levelId));
    
    if (QFile::exists(configPath)) {
        return loadLevelConfigFromFile(configPath);
    } else {
        return generateLevelConfig(levelId);
    }
}

QVariantMap ConfigManager::loadLevelConfigFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        Logger::instance()->log("ConfigManager", QString("Failed to load level config: %1").arg(filePath));
        return QVariantMap();
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    return doc.object().toVariantMap();
}

bool ConfigManager::saveLevelConfig(int levelId, const QVariantMap& config) {
    QString configPath = QString("%1/levels/level_%2.json").arg(m_configDir, QString::number(levelId));
    
    QDir().mkpath(QFileInfo(configPath).absolutePath());
    
    QJsonDocument doc = QJsonDocument::fromVariant(config);
    QFile file(configPath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        Logger::instance()->log("ConfigManager", QString("Failed to save level config: %1").arg(configPath));
        return false;
    }
    
    file.write(doc.toJson());
    return true;
}

// 物品配置方法
QVariantMap ConfigManager::loadItemConfig(const QString& itemId) {
    QString configPath = QString("%1/items/%2.json").arg(m_configDir, itemId);
    
    if (QFile::exists(configPath)) {
        return loadItemConfigFromFile(configPath);
    } else {
        return generateItemConfig(itemId, 1);
    }
}

QVariantMap ConfigManager::loadItemConfigFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        Logger::instance()->log("ConfigManager", QString("Failed to load item config: %1").arg(filePath));
        return QVariantMap();
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    return doc.object().toVariantMap();
}

bool ConfigManager::saveItemConfig(const QString& itemId, const QVariantMap& config) {
    QString configPath = QString("%1/items/%2.json").arg(m_configDir, itemId);
    
    QDir().mkpath(QFileInfo(configPath).absolutePath());
    
    QJsonDocument doc = QJsonDocument::fromVariant(config);
    QFile file(configPath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        Logger::instance()->log("ConfigManager", QString("Failed to save item config: %1").arg(configPath));
        return false;
    }
    
    file.write(doc.toJson());
    return true;
}

// 技能配置方法
QVariantMap ConfigManager::loadSkillConfig(const QString& skillId) {
    QString configPath = QString("%1/skills/%2.json").arg(m_configDir, skillId);
    
    if (QFile::exists(configPath)) {
        return loadSkillConfigFromFile(configPath);
    } else {
        return generateSkillConfig(skillId, 1);
    }
}

QVariantMap ConfigManager::loadSkillConfigFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        Logger::instance()->log("ConfigManager", QString("Failed to load skill config: %1").arg(filePath));
        return QVariantMap();
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    return doc.object().toVariantMap();
}

bool ConfigManager::saveSkillConfig(const QString& skillId, const QVariantMap& config) {
    QString configPath = QString("%1/skills/%2.json").arg(m_configDir, skillId);
    
    QDir().mkpath(QFileInfo(configPath).absolutePath());
    
    QJsonDocument doc = QJsonDocument::fromVariant(config);
    QFile file(configPath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        Logger::instance()->log("ConfigManager", QString("Failed to save skill config: %1").arg(configPath));
        return false;
    }
    
    file.write(doc.toJson());
    return true;
}

// 任务配置方法
QVariantMap ConfigManager::loadQuestConfig(const QString& questId) {
    QString configPath = QString("%1/quests/%2.json").arg(m_configDir, questId);
    
    if (QFile::exists(configPath)) {
        return loadQuestConfigFromFile(configPath);
    } else {
        return QVariantMap();
    }
}

QVariantMap ConfigManager::loadQuestConfigFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        Logger::instance()->log("ConfigManager", QString("Failed to load quest config: %1").arg(filePath));
        return QVariantMap();
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    return doc.object().toVariantMap();
}

bool ConfigManager::saveQuestConfig(const QString& questId, const QVariantMap& config) {
    QString configPath = QString("%1/quests/%2.json").arg(m_configDir, questId);
    
    QDir().mkpath(QFileInfo(configPath).absolutePath());
    
    QJsonDocument doc = QJsonDocument::fromVariant(config);
    QFile file(configPath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        Logger::instance()->log("ConfigManager", QString("Failed to save quest config: %1").arg(configPath));
        return false;
    }
    
    file.write(doc.toJson());
    return true;
}

// 动态配置生成方法
QVariantMap ConfigManager::generateCharacterConfig(const QString& characterType, int level) {
    if (!m_pythonInitialized) {
        // 使用C++生成默认配置
        QVariantMap config;
        config["type"] = characterType;
        config["level"] = level;
        config["health"] = 100 + level * 10;
        config["mana"] = 50 + level * 5;
        config["attack"] = 10 + level * 2;
        config["defense"] = 5 + level;
        config["speed"] = 10 + level;
        return config;
    }
    
    try {
        py::dict pyConfig = m_configGenerator.attr("generate_character_config")(characterType.toStdString(), level);
        return pyDictToQVariantMap(pyConfig);
    } catch (const std::exception& e) {
        Logger::instance()->log("ConfigManager", QString("Failed to generate character config: %1").arg(e.what()));
        return QVariantMap();
    }
}

QVariantMap ConfigManager::generateLevelConfig(int levelId, float difficulty) {
    if (!m_pythonInitialized) {
        // 使用C++生成默认配置
        QVariantMap config;
        config["id"] = levelId;
        config["difficulty"] = difficulty;
        config["enemy_count"] = 5 + levelId * 2;
        config["boss_count"] = levelId / 5;
        config["reward_exp"] = 100 + levelId * 50;
        config["reward_gold"] = 50 + levelId * 25;
        return config;
    }
    
    try {
        py::dict pyConfig = m_configGenerator.attr("generate_level_config")(levelId, difficulty);
        return pyDictToQVariantMap(pyConfig);
    } catch (const std::exception& e) {
        Logger::instance()->log("ConfigManager", QString("Failed to generate level config: %1").arg(e.what()));
        return QVariantMap();
    }
}

QVariantMap ConfigManager::generateItemConfig(const QString& itemType, int level) {
    if (!m_pythonInitialized) {
        // 使用C++生成默认配置
        QVariantMap config;
        config["type"] = itemType;
        config["level"] = level;
        config["value"] = 10 + level * 5;
        config["rarity"] = "common";
        return config;
    }
    
    try {
        py::dict pyConfig = m_configGenerator.attr("generate_item_config")(itemType.toStdString(), level);
        return pyDictToQVariantMap(pyConfig);
    } catch (const std::exception& e) {
        Logger::instance()->log("ConfigManager", QString("Failed to generate item config: %1").arg(e.what()));
        return QVariantMap();
    }
}

QVariantMap ConfigManager::generateSkillConfig(const QString& skillType, int level) {
    if (!m_pythonInitialized) {
        // 使用C++生成默认配置
        QVariantMap config;
        config["type"] = skillType;
        config["level"] = level;
        config["damage"] = 20 + level * 5;
        config["mana_cost"] = 10 + level * 2;
        config["cooldown"] = 5.0 - level * 0.1;
        return config;
    }
    
    try {
        py::dict pyConfig = m_configGenerator.attr("generate_skill_config")(skillType.toStdString(), level);
        return pyDictToQVariantMap(pyConfig);
    } catch (const std::exception& e) {
        Logger::instance()->log("ConfigManager", QString("Failed to generate skill config: %1").arg(e.what()));
        return QVariantMap();
    }
}

// 配置验证方法
bool ConfigManager::validateConfig(const QVariantMap& config, const QString& configType) {
    return getConfigErrors(config, configType).isEmpty();
}

QStringList ConfigManager::getConfigErrors(const QVariantMap& config, const QString& configType) {
    QStringList errors;
    
    if (configType == "character") {
        if (!config.contains("type")) errors << "Missing character type";
        if (!config.contains("level")) errors << "Missing character level";
        if (!config.contains("health")) errors << "Missing character health";
    } else if (configType == "level") {
        if (!config.contains("id")) errors << "Missing level id";
        if (!config.contains("difficulty")) errors << "Missing level difficulty";
    } else if (configType == "item") {
        if (!config.contains("type")) errors << "Missing item type";
        if (!config.contains("value")) errors << "Missing item value";
    } else if (configType == "skill") {
        if (!config.contains("type")) errors << "Missing skill type";
        if (!config.contains("damage")) errors << "Missing skill damage";
    }
    
    return errors;
}

// 配置模板方法
QVariantMap ConfigManager::getConfigTemplate(const QString& configType) {
    QVariantMap template_;
    
    if (configType == "character") {
        template_["type"] = "";
        template_["level"] = 1;
        template_["health"] = 100;
        template_["mana"] = 50;
        template_["attack"] = 10;
        template_["defense"] = 5;
        template_["speed"] = 10;
    } else if (configType == "level") {
        template_["id"] = 1;
        template_["difficulty"] = 1.0;
        template_["enemy_count"] = 5;
        template_["boss_count"] = 0;
        template_["reward_exp"] = 100;
        template_["reward_gold"] = 50;
    }
    
    return template_;
}

bool ConfigManager::applyConfigTemplate(QVariantMap& config, const QString& templateType) {
    QVariantMap template_ = getConfigTemplate(templateType);
    
    for (auto it = template_.begin(); it != template_.end(); ++it) {
        if (!config.contains(it.key())) {
            config[it.key()] = it.value();
        }
    }
    
    return true;
}

// 配置热重载方法
void ConfigManager::enableHotReload(bool enable) {
    m_hotReloadEnabled = enable;
    setupHotReload();
}

bool ConfigManager::isHotReloadEnabled() const {
    return m_hotReloadEnabled;
}

void ConfigManager::reloadConfigs() {
    loadDefaultConfig();
    emit configReloaded();
}

// 配置备份和恢复方法
bool ConfigManager::backupConfigs(const QString& backupPath) {
    QDir backupDir(backupPath);
    if (!backupDir.exists()) {
        backupDir.mkpath(".");
    }
    
    bool success = true;
    success &= FileUtils::copyFile(m_defaultConfigPath, backupPath + "/default_config.json");
    success &= FileUtils::copyFile(m_userConfigPath, backupPath + "/user_config.json");
    success &= FileUtils::copyFile(m_systemConfigPath, backupPath + "/system_config.json");
    
    return success;
}

bool ConfigManager::restoreConfigs(const QString& backupPath) {
    bool success = true;
    success &= FileUtils::copyFile(backupPath + "/default_config.json", m_defaultConfigPath);
    success &= FileUtils::copyFile(backupPath + "/user_config.json", m_userConfigPath);
    success &= FileUtils::copyFile(backupPath + "/system_config.json", m_systemConfigPath);
    
    if (success) {
        reloadConfigs();
    }
    
    return success;
}

// 配置导入导出方法
bool ConfigManager::exportConfigs(const QString& exportPath, const QStringList& configTypes) {
    QDir exportDir(exportPath);
    if (!exportDir.exists()) {
        exportDir.mkpath(".");
    }
    
    bool success = true;
    
    if (configTypes.isEmpty() || configTypes.contains("game")) {
        success &= saveConfigFile(exportPath + "/game_config.json");
    }
    if (configTypes.isEmpty() || configTypes.contains("user")) {
        success &= saveConfigFile(exportPath + "/user_config.json");
    }
    if (configTypes.isEmpty() || configTypes.contains("system")) {
        success &= saveConfigFile(exportPath + "/system_config.json");
    }
    
    return success;
}

bool ConfigManager::importConfigs(const QString& importPath) {
    bool success = true;
    success &= loadConfigFile(importPath + "/game_config.json");
    success &= loadConfigFile(importPath + "/user_config.json");
    success &= loadConfigFile(importPath + "/system_config.json");
    
    if (success) {
        emit configReloaded();
    }
    
    return success;
}

// 私有方法
void ConfigManager::onConfigFileChanged(const QString& filePath) {
    if (m_hotReloadEnabled) {
        Logger::instance()->log("ConfigManager", QString("Config file changed: %1").arg(filePath));
        loadConfigFile(filePath);
    }
}

QVariantMap ConfigManager::pyDictToQVariantMap(const py::dict& pyDict) {
    QVariantMap result;
    
    for (auto item : pyDict) {
        QString key = QString::fromStdString(py::str(item.first));
        
        if (py::isinstance<py::dict>(item.second)) {
            result[key] = pyDictToQVariantMap(py::dict(item.second));
        } else if (py::isinstance<py::list>(item.second)) {
            result[key] = pyListToQVariantList(py::list(item.second));
        } else {
            result[key] = QVariant::fromValue(QString::fromStdString(py::str(item.second)));
        }
    }
    
    return result;
}

QVariantList ConfigManager::pyListToQVariantList(const py::list& pyList) {
    QVariantList result;
    
    for (auto item : pyList) {
        if (py::isinstance<py::dict>(item)) {
            result.append(pyDictToQVariantMap(py::dict(item)));
        } else if (py::isinstance<py::list>(item)) {
            result.append(pyListToQVariantList(py::list(item)));
        } else {
            result.append(QVariant::fromValue(QString::fromStdString(py::str(item))));
        }
    }
    
    return result;
}