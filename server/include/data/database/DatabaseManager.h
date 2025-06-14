#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVariantList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMutex>
#include <QTimer>
#include <memory>

class DatabaseManager : public QObject {
    Q_OBJECT
    
public:
    static DatabaseManager* instance();
    
    // 数据库连接管理
    bool initialize(const QString& databasePath = "");
    void close();
    bool isOpen() const;
    QString getDatabasePath() const;
    
    // 数据库维护
    bool backupDatabase(const QString& backupPath);
    bool restoreDatabase(const QString& backupPath);
    bool vacuumDatabase();
    bool optimizeDatabase();
    qint64 getDatabaseSize();
    
    // 角色管理
    bool createCharacter(const QString& characterId, const QString& name, const QString& classType);
    bool updateCharacter(const QString& characterId, const QVariantMap& data);
    QVariantMap getCharacter(const QString& characterId);
    bool deleteCharacter(const QString& characterId);
    QStringList getAllCharacterIds();
    QVariantList getCharactersByClass(const QString& classType);
    
    // 技能管理
    bool addSkillToCharacter(const QString& characterId, const QString& skillId);
    bool removeSkillFromCharacter(const QString& characterId, const QString& skillId);
    bool updateSkillLevel(const QString& characterId, const QString& skillId, int level);
    QVariantList getCharacterSkills(const QString& characterId);
    QVariantList getEquippedSkills(const QString& characterId);
    
    // 装备管理
    bool equipItem(const QString& characterId, const QString& equipmentId, const QString& slot);
    bool unequipItem(const QString& characterId, const QString& slot);
    QVariantMap getEquippedItems(const QString& characterId);
    bool updateEquipmentDurability(const QString& characterId, const QString& slot, int durability);
    
    // 背包管理
    bool addItemToInventory(const QString& characterId, const QString& itemId, int quantity = 1);
    bool removeItemFromInventory(const QString& characterId, const QString& itemId, int quantity = 1);
    QVariantList getInventory(const QString& characterId);
    int getItemQuantity(const QString& characterId, const QString& itemId);
    
    // 任务管理
    bool startQuest(const QString& characterId, const QString& questId);
    bool updateQuestProgress(const QString& characterId, const QString& questId, const QVariantMap& progress);
    bool completeQuest(const QString& characterId, const QString& questId);
    QVariantList getActiveQuests(const QString& characterId);
    QVariantList getCompletedQuests(const QString& characterId);
    
    // 关卡管理
    bool unlockLevel(const QString& characterId, const QString& levelId);
    bool updateLevelProgress(const QString& characterId, const QString& levelId, const QVariantMap& progress);
    QVariantList getUnlockedLevels(const QString& characterId);
    QVariantMap getLevelProgress(const QString& characterId, const QString& levelId);
    
    // 战斗记录
    bool recordBattle(const QString& characterId, const QVariantMap& battleData);
    QVariantList getBattleHistory(const QString& characterId, int limit = 10);
    QVariantMap getBattleStatistics(const QString& characterId);
    
    // 状态效果管理
    bool addStatusEffect(const QString& characterId, const QString& effectType, const QString& effectName, 
                        float value, float duration, const QString& source);
    bool removeStatusEffect(const QString& characterId, const QString& effectType);
    QVariantList getActiveStatusEffects(const QString& characterId);
    bool updateStatusEffectDuration(const QString& characterId, const QString& effectType, float remainingTime);
    
    // 成就管理
    bool unlockAchievement(const QString& characterId, const QString& achievementId);
    bool isAchievementUnlocked(const QString& characterId, const QString& achievementId);
    QVariantList getUnlockedAchievements(const QString& characterId);
    QVariantList getAvailableAchievements(const QString& characterId);
    
    // 存档管理
    bool saveGame(const QString& slotId, const QString& characterId, const QVariantMap& gameData);
    QVariantMap loadGame(const QString& slotId);
    bool deleteSave(const QString& slotId);
    QStringList getAllSaveSlots();
    
    // 统计数据管理
    bool updateStatistic(const QString& characterId, const QString& statKey, const QVariant& value);
    QVariant getStatistic(const QString& characterId, const QString& statKey, const QVariant& defaultValue = QVariant());
    QVariantMap getAllStatistics(const QString& characterId);
    
    // 配置管理
    bool setConfig(const QString& key, const QVariant& value, const QString& description = "");
    QVariant getConfig(const QString& key, const QVariant& defaultValue = QVariant());
    bool deleteConfig(const QString& key);
    QVariantMap getConfigsByCategory(const QString& category);
    
    // 日志管理
    bool logEvent(const QString& characterId, const QString& level, const QString& category, 
                  const QString& message, const QVariantMap& data = QVariantMap());
    QVariantList getLogs(const QString& characterId = "", const QString& level = "", int limit = 100);
    
    // 事务管理
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    bool isInTransaction() const;
    
    // 批量操作
    bool batchInsert(const QString& tableName, const QVariantList& data);
    bool batchUpdate(const QString& tableName, const QString& whereClause, const QVariantMap& data);
    bool batchDelete(const QString& tableName, const QString& whereClause);
    
    // 查询构建器
    class QueryBuilder {
    public:
        QueryBuilder(DatabaseManager* manager);
        
        QueryBuilder& select(const QString& columns = "*");
        QueryBuilder& from(const QString& tableName);
        QueryBuilder& where(const QString& column, const QString& operator_, const QVariant& value);
        QueryBuilder& orderBy(const QString& column, const QString& direction = "ASC");
        QueryBuilder& limit(int count);
        
        QVariantList execute();
        QVariantMap executeOne();
        bool executeNonQuery();
        
    private:
        struct WhereCondition {
            QString column;
            QString operator_;
            QVariant value;
        };
        
        DatabaseManager* m_manager;
        QString m_tableName;
        QString m_selectColumns;
        QList<WhereCondition> m_whereConditions;
        QString m_orderBy;
        int m_limit;
    };
    
    QueryBuilder query();
    
    // 数据验证
    bool validateCharacterData(const QVariantMap& data);
    bool validateQuestData(const QVariantMap& data);
    bool validateLevelData(const QVariantMap& data);
    
    // 数据导入导出
    bool exportData(const QString& exportPath, const QStringList& tables = QStringList());
    bool importData(const QString& importPath);
    
    // 性能监控
    QVariantMap getPerformanceStats();
    bool enableQueryLogging(bool enable);
    QVariantList getQueryLog();
    
signals:
    void databaseOpened();
    void databaseClosed();
    void characterCreated(const QString& characterId);
    void characterUpdated(const QString& characterId);
    void characterDeleted(const QString& characterId);
    void questStarted(const QString& characterId, const QString& questId);
    void questCompleted(const QString& characterId, const QString& questId);
    void levelUnlocked(const QString& characterId, const QString& levelId);
    void achievementUnlocked(const QString& characterId, const QString& achievementId);
    void battleRecorded(const QString& characterId, const QVariantMap& battleData);
    void gameSaved(const QString& slotId);
    void gameLoaded(const QString& slotId);
    void errorOccurred(const QString& error);
    void transactionStarted();
    void transactionCommitted();
    void transactionRolledBack();
    
private slots:
    void onAutoSaveTimer();
    void onCleanupTimer();
    
private:
    DatabaseManager();
    ~DatabaseManager();
    
    // 私有辅助方法
    bool executeQuery(const QString& sql, const QVariantMap& parameters = QVariantMap());
    QVariantList executeSelectQuery(const QString& sql, const QVariantMap& parameters = QVariantMap());
    QVariantMap executeSelectOneQuery(const QString& sql, const QVariantMap& parameters = QVariantMap());
    
    void logError(const QString& operation, const QSqlError& error);
    void logQuery(const QString& sql, const QVariantMap& parameters, qint64 duration);
    
    QString buildWhereClause(const QVariantMap& conditions);
    QVariantMap parseJsonField(const QString& jsonString);
    QString serializeToJson(const QVariantMap& data);
    
    bool checkCharacterExists(const QString& characterId);
    bool checkSkillExists(const QString& skillId);
    bool checkEquipmentExists(const QString& equipmentId);
    bool checkItemExists(const QString& itemId);
    bool checkQuestExists(const QString& questId);
    bool checkLevelExists(const QString& levelId);
    bool checkAchievementExists(const QString& achievementId);
    
    // 成员变量
    static DatabaseManager* s_instance;
    static QMutex s_mutex;
    
    QSqlDatabase m_database;
    QString m_databasePath;
    bool m_isInitialized;
    bool m_inTransaction;
    bool m_queryLoggingEnabled;
    
    // 自动保存
    QTimer* m_autoSaveTimer;
    bool m_autoSaveEnabled;
    int m_autoSaveInterval;
    
    // 清理定时器
    QTimer* m_cleanupTimer;
    
    // 查询日志
    QVariantList m_queryLog;
    int m_maxQueryLogSize;
    
    // 性能统计
    QVariantMap m_performanceStats;
    qint64 m_totalQueries;
    qint64 m_totalQueryTime;
}; 