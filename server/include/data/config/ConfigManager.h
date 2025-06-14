#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVariantList>
#include <QMap>
#include <QMutex>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

class ConfigManager : public QObject {
    Q_OBJECT
    
public:
    static ConfigManager* instance();
    
    // 配置管理
    void setConfig(const QString& key, const QVariant& value);
    QVariant getConfig(const QString& key, const QVariant& defaultValue = QVariant());
    bool hasConfig(const QString& key) const;
    void removeConfig(const QString& key);
    QStringList getAllConfigKeys() const;
    
    // 配置文件操作
    bool loadConfigFile(const QString& filePath);
    bool saveConfigFile(const QString& filePath);
    bool loadDefaultConfig();
    bool saveDefaultConfig();
    
    // 游戏配置
    void setGameConfig(const QString& key, const QVariant& value);
    QVariant getGameConfig(const QString& key, const QVariant& defaultValue = QVariant());
    
    // 用户配置
    void setUserConfig(const QString& key, const QVariant& value);
    QVariant getUserConfig(const QString& key, const QVariant& defaultValue = QVariant());
    
    // 系统配置
    void setSystemConfig(const QString& key, const QVariant& value);
    QVariant getSystemConfig(const QString& key, const QVariant& defaultValue = QVariant());
    
    // 角色配置
    QVariantMap loadCharacterConfig(const QString& characterType, int level);
    QVariantMap loadCharacterConfigFromFile(const QString& filePath);
    bool saveCharacterConfig(const QString& characterType, int level, const QVariantMap& config);
    
    // 关卡配置
    QVariantMap loadLevelConfig(int levelId);
    QVariantMap loadLevelConfigFromFile(const QString& filePath);
    bool saveLevelConfig(int levelId, const QVariantMap& config);
    
    // 物品配置
    QVariantMap loadItemConfig(const QString& itemId);
    QVariantMap loadItemConfigFromFile(const QString& filePath);
    bool saveItemConfig(const QString& itemId, const QVariantMap& config);
    
    // 技能配置
    QVariantMap loadSkillConfig(const QString& skillId);
    QVariantMap loadSkillConfigFromFile(const QString& filePath);
    bool saveSkillConfig(const QString& skillId, const QVariantMap& config);
    
    // 任务配置
    QVariantMap loadQuestConfig(const QString& questId);
    QVariantMap loadQuestConfigFromFile(const QString& filePath);
    bool saveQuestConfig(const QString& questId, const QVariantMap& config);
    
    // 动态配置生成
    QVariantMap generateCharacterConfig(const QString& characterType, int level);
    QVariantMap generateLevelConfig(int levelId, float difficulty = 1.0f);
    QVariantMap generateItemConfig(const QString& itemType, int level);
    QVariantMap generateSkillConfig(const QString& skillType, int level);
    
    // 配置验证
    bool validateConfig(const QVariantMap& config, const QString& configType);
    QStringList getConfigErrors(const QVariantMap& config, const QString& configType);
    
    // 配置模板
    QVariantMap getConfigTemplate(const QString& configType);
    bool applyConfigTemplate(QVariantMap& config, const QString& templateType);
    
    // 配置热重载
    void enableHotReload(bool enable);
    bool isHotReloadEnabled() const;
    void reloadConfigs();
    
    // 配置备份和恢复
    bool backupConfigs(const QString& backupPath);
    bool restoreConfigs(const QString& backupPath);
    
    // 配置导入导出
    bool exportConfigs(const QString& exportPath, const QStringList& configTypes = QStringList());
    bool importConfigs(const QString& importPath);
    
signals:
    void configChanged(const QString& key, const QVariant& value);
    void configLoaded(const QString& filePath);
    void configSaved(const QString& filePath);
    void configReloaded();
    void configError(const QString& error);
    
private slots:
    void onConfigFileChanged(const QString& filePath);
    
private:
    ConfigManager();
    ~ConfigManager();
    
    void initializePython();
    void setupDefaultConfigs();
    void setupHotReload();
    QVariantMap pyDictToQVariantMap(const py::dict& pyDict);
    QVariantList pyListToQVariantList(const py::list& pyList);
    
    // 配置存储
    QVariantMap m_gameConfig;
    QVariantMap m_userConfig;
    QVariantMap m_systemConfig;
    QVariantMap m_dynamicConfig;
    
    // Python集成
    py::module_ m_configGenerator;
    bool m_pythonInitialized;
    
    // 热重载
    bool m_hotReloadEnabled;
    QMap<QString, QDateTime> m_fileModificationTimes;
    
    // 配置路径
    QString m_configDir;
    QString m_defaultConfigPath;
    QString m_userConfigPath;
    QString m_systemConfigPath;
    
    // 线程安全
    mutable QMutex m_configMutex;
    
    static ConfigManager* s_instance;
};
