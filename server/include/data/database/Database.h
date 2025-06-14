#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVariantList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QMutex>
#include <QList>

class Database : public QObject {
    Q_OBJECT
    
public:
    static Database* instance();
    
    // 数据库连接管理
    bool initialize(const QString& databasePath = "");
    void close();
    bool isOpen() const;
    
    // 表管理
    bool createTables();
    bool tableExists(const QString& tableName);
    bool dropTable(const QString& tableName);
    
    // 角色数据
    bool saveCharacter(const QString& characterId, const QVariantMap& data);
    QVariantMap loadCharacter(const QString& characterId);
    bool deleteCharacter(const QString& characterId);
    QStringList getAllCharacterIds();
    
    // 存档数据
    bool saveGameData(const QString& saveSlot, const QVariantMap& data);
    QVariantMap loadGameData(const QString& saveSlot);
    bool deleteGameData(const QString& saveSlot);
    QStringList getAllSaveSlots();
    
    // 配置数据
    bool saveConfig(const QString& configKey, const QVariant& value);
    QVariant loadConfig(const QString& configKey, const QVariant& defaultValue = QVariant());
    bool deleteConfig(const QString& configKey);
    QStringList getAllConfigKeys();
    
    // 任务数据
    bool saveQuest(const QString& questId, const QVariantMap& data);
    QVariantMap loadQuest(const QString& questId);
    bool deleteQuest(const QString& questId);
    QStringList getAllQuestIds();
    
    // 物品数据
    bool saveItem(const QString& itemId, const QVariantMap& data);
    QVariantMap loadItem(const QString& itemId);
    bool deleteItem(const QString& itemId);
    QStringList getAllItemIds();
    
    // 关卡数据
    bool saveLevel(const QString& levelId, const QVariantMap& data);
    QVariantMap loadLevel(const QString& levelId);
    bool deleteLevel(const QString& levelId);
    QStringList getAllLevelIds();
    
    // 统计数据
    bool saveStatistic(const QString& statKey, const QVariant& value);
    QVariant loadStatistic(const QString& statKey, const QVariant& defaultValue = QVariant());
    bool deleteStatistic(const QString& statKey);
    QStringList getAllStatKeys();
    
    // 成就数据
    bool saveAchievement(const QString& achievementId, const QVariantMap& data);
    QVariantMap loadAchievement(const QString& achievementId);
    bool deleteAchievement(const QString& achievementId);
    QStringList getAllAchievementIds();
    
    // 通用查询方法
    bool executeQuery(const QString& sql, const QVariantMap& parameters = QVariantMap());
    QVariantList executeSelectQuery(const QString& sql, const QVariantMap& parameters = QVariantMap());
    QVariantMap executeSelectOneQuery(const QString& sql, const QVariantMap& parameters = QVariantMap());
    
    // 事务管理
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    
    // 备份和恢复
    bool backupDatabase(const QString& backupPath);
    bool restoreDatabase(const QString& backupPath);
    
    // 数据库维护
    bool vacuumDatabase();
    bool optimizeDatabase();
    qint64 getDatabaseSize();
    
signals:
    void databaseOpened();
    void databaseClosed();
    void tableCreated(const QString& tableName);
    void tableDropped(const QString& tableName);
    void dataSaved(const QString& tableName, const QString& id);
    void dataDeleted(const QString& tableName, const QString& id);
    void errorOccurred(const QString& error);
    
private slots:
    void onDatabaseError(const QSqlError& error);
    
private:
    Database();
    ~Database();
    
    bool createCharacterTable();
    bool createSaveDataTable();
    bool createConfigTable();
    bool createQuestTable();
    bool createItemTable();
    bool createLevelTable();
    bool createStatisticTable();
    bool createAchievementTable();
    
    QString getDatabasePath() const;
    void logError(const QString& operation, const QSqlError& error);
    
    static Database* s_instance;
    static QMutex s_mutex;
    
    QSqlDatabase m_database;
    QString m_databasePath;
    bool m_isInitialized;
    bool m_inTransaction;
};
