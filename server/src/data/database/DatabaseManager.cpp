#include "DatabaseManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QFile>
#include <QDir>
#include <QVariant>
#include <QDebug>
#include <QDateTime>
#include <QTextStream>
#include <QJsonDocument>
#include <QMutexLocker>
#include <QElapsedTimer>

DatabaseManager* DatabaseManager::s_instance = nullptr;

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent), m_isOpen(false), m_inTransaction(false), m_autoSaveTimer(nullptr), m_cleanupTimer(nullptr) {}

DatabaseManager::~DatabaseManager() {
    close();
}

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::initialize(const QString& dbPath) {
    QMutexLocker locker(&m_mutex);
    if (m_isOpen) close();
    m_dbPath = dbPath;
    m_db = QSqlDatabase::addDatabase("QSQLITE", "main_db");
    m_db.setDatabaseName(dbPath);
    if (!m_db.open()) {
        logError("open", m_db.lastError());
        m_isOpen = false;
        emit errorOccurred(m_db.lastError().text());
        return false;
    }
    m_isOpen = true;
    emit databaseOpened();
    return true;
}

void DatabaseManager::close() {
    QMutexLocker locker(&m_mutex);
    if (m_db.isOpen()) {
        m_db.close();
        QSqlDatabase::removeDatabase("main_db");
        m_isOpen = false;
        emit databaseClosed();
    }
}

bool DatabaseManager::isOpen() const {
    return m_isOpen;
}

QString DatabaseManager::getDatabasePath() const {
    return m_dbPath;
}

bool DatabaseManager::backupDatabase(const QString& backupPath) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QFile dbFile(m_dbPath);
    if (!dbFile.exists()) return false;
    return dbFile.copy(backupPath);
}

bool DatabaseManager::restoreDatabase(const QString& backupPath) {
    QMutexLocker locker(&m_mutex);
    if (m_isOpen) close();
    QFile backupFile(backupPath);
    if (!backupFile.exists()) return false;
    QFile dbFile(m_dbPath);
    if (dbFile.exists()) dbFile.remove();
    bool ok = backupFile.copy(m_dbPath);
    if (ok) {
        return initialize(m_dbPath);
    }
    return false;
}

bool DatabaseManager::vacuumDatabase() {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    return query.exec("VACUUM");
}

bool DatabaseManager::optimizeDatabase() {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    return query.exec("PRAGMA optimize");
}

qint64 DatabaseManager::getDatabaseSize() {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return 0;
    
    QFileInfo fileInfo(m_dbPath);
    return fileInfo.size();
}

void DatabaseManager::logError(const QString& operation, const QSqlError& error) {
    qDebug() << "Database error in" << operation << ":" << error.text();
    emit errorOccurred(QString("Database error in %1: %2").arg(operation, error.text()));
}

// ================= 角色管理 =================

bool DatabaseManager::createCharacter(const QString& characterId, const QString& name, const QString& classType) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(INSERT INTO characters (id, name, class, level, experience, health, max_health, mana, max_mana, attack, defense, speed, critical_rate, critical_damage, is_player, is_alive, created_at, updated_at)
                    VALUES (:id, :name, :class, 1, 0, 100, 100, 50, 50, 15, 10, 5, 0.05, 1.5, 1, 1, :created_at, :updated_at))");
    query.bindValue(":id", characterId);
    query.bindValue(":name", name);
    query.bindValue(":class", classType);
    query.bindValue(":created_at", QDateTime::currentDateTime());
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("createCharacter", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    emit characterCreated(characterId);
    return true;
}

bool DatabaseManager::updateCharacter(const QString& characterId, const QVariantMap& data) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    if (data.isEmpty()) return false;
    QStringList setClauses;
    QVariantMap params;
    for (auto it = data.begin(); it != data.end(); ++it) {
        setClauses << QString("%1 = :%1").arg(it.key());
        params[":" + it.key()] = it.value();
    }
    setClauses << "updated_at = :updated_at";
    params[":updated_at"] = QDateTime::currentDateTime();
    QString sql = QString("UPDATE characters SET %1 WHERE id = :id").arg(setClauses.join(", "));
    params[":id"] = characterId;
    QSqlQuery query(m_db);
    query.prepare(sql);
    for (auto it = params.begin(); it != params.end(); ++it) {
        query.bindValue(it.key(), it.value());
    }
    bool ok = query.exec();
    if (!ok) {
        logError("updateCharacter", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    emit characterUpdated(characterId);
    return true;
}

QVariantMap DatabaseManager::getCharacter(const QString& characterId) {
    QMutexLocker locker(&m_mutex);
    QVariantMap result;
    if (!m_isOpen) return result;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM characters WHERE id = :id");
    query.bindValue(":id", characterId);
    if (query.exec() && query.next()) {
        QSqlRecord rec = query.record();
        for (int i = 0; i < rec.count(); ++i) {
            result[rec.fieldName(i)] = query.value(i);
        }
    } else if (query.lastError().isValid()) {
        logError("getCharacter", query.lastError());
    }
    return result;
}

bool DatabaseManager::deleteCharacter(const QString& characterId) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM characters WHERE id = :id");
    query.bindValue(":id", characterId);
    bool ok = query.exec();
    if (!ok) {
        logError("deleteCharacter", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    emit characterDeleted(characterId);
    return true;
}

QStringList DatabaseManager::getAllCharacterIds() {
    QMutexLocker locker(&m_mutex);
    QStringList ids;
    if (!m_isOpen) return ids;
    QSqlQuery query(m_db);
    if (query.exec("SELECT id FROM characters")) {
        while (query.next()) {
            ids << query.value(0).toString();
        }
    }
    return ids;
}

QVariantList DatabaseManager::getCharactersByClass(const QString& classType) {
    QMutexLocker locker(&m_mutex);
    QVariantList list;
    if (!m_isOpen) return list;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM characters WHERE class = :class");
    query.bindValue(":class", classType);
    if (query.exec()) {
        while (query.next()) {
            QVariantMap row;
            QSqlRecord rec = query.record();
            for (int i = 0; i < rec.count(); ++i) {
                row[rec.fieldName(i)] = query.value(i);
            }
            list << row;
        }
    }
    return list;
}

// ================= 技能管理 =================

bool DatabaseManager::addSkillToCharacter(const QString& characterId, const QString& skillId) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(INSERT INTO character_skills (character_id, skill_id, level, is_equipped, created_at, updated_at)
                    VALUES (:character_id, :skill_id, 1, 0, :created_at, :updated_at))");
    query.bindValue(":character_id", characterId);
    query.bindValue(":skill_id", skillId);
    query.bindValue(":created_at", QDateTime::currentDateTime());
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("addSkillToCharacter", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    return true;
}

bool DatabaseManager::removeSkillFromCharacter(const QString& characterId, const QString& skillId) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM character_skills WHERE character_id = :character_id AND skill_id = :skill_id");
    query.bindValue(":character_id", characterId);
    query.bindValue(":skill_id", skillId);
    bool ok = query.exec();
    if (!ok) {
        logError("removeSkillFromCharacter", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    return true;
}

bool DatabaseManager::updateSkillLevel(const QString& characterId, const QString& skillId, int level) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(UPDATE character_skills SET level = :level, updated_at = :updated_at
                    WHERE character_id = :character_id AND skill_id = :skill_id)");
    query.bindValue(":character_id", characterId);
    query.bindValue(":skill_id", skillId);
    query.bindValue(":level", level);
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("updateSkillLevel", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    return true;
}

QVariantList DatabaseManager::getCharacterSkills(const QString& characterId) {
    QMutexLocker locker(&m_mutex);
    QVariantList list;
    if (!m_isOpen) return list;
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT cs.*, s.name, s.description, s.type, s.element, s.mana_cost, s.cooldown
                    FROM character_skills cs
                    JOIN skills s ON cs.skill_id = s.id
                    WHERE cs.character_id = :character_id)");
    query.bindValue(":character_id", characterId);
    if (query.exec()) {
        while (query.next()) {
            QVariantMap row;
            QSqlRecord rec = query.record();
            for (int i = 0; i < rec.count(); ++i) {
                row[rec.fieldName(i)] = query.value(i);
            }
            list << row;
        }
    }
    return list;
}

QVariantList DatabaseManager::getEquippedSkills(const QString& characterId) {
    QMutexLocker locker(&m_mutex);
    QVariantList list;
    if (!m_isOpen) return list;
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT cs.*, s.name, s.description, s.type, s.element, s.mana_cost, s.cooldown
                    FROM character_skills cs
                    JOIN skills s ON cs.skill_id = s.id
                    WHERE cs.character_id = :character_id AND cs.is_equipped = 1)");
    query.bindValue(":character_id", characterId);
    if (query.exec()) {
        while (query.next()) {
            QVariantMap row;
            QSqlRecord rec = query.record();
            for (int i = 0; i < rec.count(); ++i) {
                row[rec.fieldName(i)] = query.value(i);
            }
            list << row;
        }
    }
    return list;
}

// ================= 装备管理 =================

bool DatabaseManager::equipItem(const QString& characterId, const QString& equipmentId, const QString& slot) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(INSERT INTO character_equipment (character_id, equipment_id, slot, is_equipped, durability, enchant_level, created_at, updated_at)
                    VALUES (:character_id, :equipment_id, :slot, 1, 100, 0, :created_at, :updated_at))");
    query.bindValue(":character_id", characterId);
    query.bindValue(":equipment_id", equipmentId);
    query.bindValue(":slot", slot);
    query.bindValue(":created_at", QDateTime::currentDateTime());
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("equipItem", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    return true;
}

bool DatabaseManager::unequipItem(const QString& characterId, const QString& slot) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare("UPDATE character_equipment SET is_equipped = 0 WHERE character_id = :character_id AND slot = :slot");
    query.bindValue(":character_id", characterId);
    query.bindValue(":slot", slot);
    bool ok = query.exec();
    if (!ok) {
        logError("unequipItem", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    return true;
}

QVariantMap DatabaseManager::getEquippedItems(const QString& characterId) {
    QMutexLocker locker(&m_mutex);
    QVariantMap result;
    if (!m_isOpen) return result;
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT ce.*, e.name, e.type, e.slot, e.rarity, e.attributes
                    FROM character_equipment ce
                    JOIN equipment e ON ce.equipment_id = e.id
                    WHERE ce.character_id = :character_id AND ce.is_equipped = 1)");
    query.bindValue(":character_id", characterId);
    if (query.exec()) {
        while (query.next()) {
            QString slot = query.value("slot").toString();
            QVariantMap item;
            QSqlRecord rec = query.record();
            for (int i = 0; i < rec.count(); ++i) {
                item[rec.fieldName(i)] = query.value(i);
            }
            result[slot] = item;
        }
    }
    return result;
}

bool DatabaseManager::updateEquipmentDurability(const QString& characterId, const QString& slot, int durability) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(UPDATE character_equipment SET durability = :durability, updated_at = :updated_at
                    WHERE character_id = :character_id AND slot = :slot)");
    query.bindValue(":character_id", characterId);
    query.bindValue(":slot", slot);
    query.bindValue(":durability", durability);
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("updateEquipmentDurability", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    return true;
}

// ================= 背包管理 =================

bool DatabaseManager::addItemToInventory(const QString& characterId, const QString& itemId, int quantity) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(INSERT INTO character_inventory (character_id, item_id, quantity, created_at, updated_at)
                    VALUES (:character_id, :item_id, :quantity, :created_at, :updated_at)
                    ON CONFLICT(character_id, item_id) DO UPDATE SET
                    quantity = quantity + :quantity, updated_at = :updated_at)");
    query.bindValue(":character_id", characterId);
    query.bindValue(":item_id", itemId);
    query.bindValue(":quantity", quantity);
    query.bindValue(":created_at", QDateTime::currentDateTime());
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("addItemToInventory", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    return true;
}

bool DatabaseManager::removeItemFromInventory(const QString& characterId, const QString& itemId, int quantity) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(UPDATE character_inventory SET quantity = quantity - :quantity, updated_at = :updated_at
                    WHERE character_id = :character_id AND item_id = :item_id AND quantity >= :quantity)");
    query.bindValue(":character_id", characterId);
    query.bindValue(":item_id", itemId);
    query.bindValue(":quantity", quantity);
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("removeItemFromInventory", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    return query.numRowsAffected() > 0;
}

QVariantList DatabaseManager::getInventory(const QString& characterId) {
    QMutexLocker locker(&m_mutex);
    QVariantList list;
    if (!m_isOpen) return list;
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT ci.*, i.name, i.type, i.rarity, i.description, i.effects
                    FROM character_inventory ci
                    JOIN items i ON ci.item_id = i.id
                    WHERE ci.character_id = :character_id AND ci.quantity > 0)");
    query.bindValue(":character_id", characterId);
    if (query.exec()) {
        while (query.next()) {
            QVariantMap row;
            QSqlRecord rec = query.record();
            for (int i = 0; i < rec.count(); ++i) {
                row[rec.fieldName(i)] = query.value(i);
            }
            list << row;
        }
    }
    return list;
}

int DatabaseManager::getItemQuantity(const QString& characterId, const QString& itemId) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return 0;
    QSqlQuery query(m_db);
    query.prepare("SELECT quantity FROM character_inventory WHERE character_id = :character_id AND item_id = :item_id");
    query.bindValue(":character_id", characterId);
    query.bindValue(":item_id", itemId);
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

// ================= 任务管理 =================

bool DatabaseManager::startQuest(const QString& characterId, const QString& questId) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(INSERT INTO character_quests (character_id, quest_id, status, progress, started_at, updated_at)
                    VALUES (:character_id, :quest_id, 'in_progress', '{}', :started_at, :updated_at))");
    query.bindValue(":character_id", characterId);
    query.bindValue(":quest_id", questId);
    query.bindValue(":started_at", QDateTime::currentDateTime());
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("startQuest", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    emit questStarted(characterId, questId);
    return true;
}

bool DatabaseManager::updateQuestProgress(const QString& characterId, const QString& questId, const QVariantMap& progress) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(UPDATE character_quests SET progress = :progress, updated_at = :updated_at
                    WHERE character_id = :character_id AND quest_id = :quest_id)");
    query.bindValue(":character_id", characterId);
    query.bindValue(":quest_id", questId);
    query.bindValue(":progress", serializeToJson(progress));
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("updateQuestProgress", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    return true;
}

bool DatabaseManager::completeQuest(const QString& characterId, const QString& questId) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(UPDATE character_quests SET status = 'completed', completed_at = :completed_at, updated_at = :updated_at
                    WHERE character_id = :character_id AND quest_id = :quest_id)");
    query.bindValue(":character_id", characterId);
    query.bindValue(":quest_id", questId);
    query.bindValue(":completed_at", QDateTime::currentDateTime());
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("completeQuest", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    emit questCompleted(characterId, questId);
    return true;
}

QVariantList DatabaseManager::getActiveQuests(const QString& characterId) {
    QMutexLocker locker(&m_mutex);
    QVariantList list;
    if (!m_isOpen) return list;
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT cq.*, q.title, q.description, q.type, q.chapter, q.rewards
                    FROM character_quests cq
                    JOIN quests q ON cq.quest_id = q.id
                    WHERE cq.character_id = :character_id AND cq.status = 'in_progress')");
    query.bindValue(":character_id", characterId);
    if (query.exec()) {
        while (query.next()) {
            QVariantMap row;
            QSqlRecord rec = query.record();
            for (int i = 0; i < rec.count(); ++i) {
                row[rec.fieldName(i)] = query.value(i);
            }
            list << row;
        }
    }
    return list;
}

QVariantList DatabaseManager::getCompletedQuests(const QString& characterId) {
    QMutexLocker locker(&m_mutex);
    QVariantList list;
    if (!m_isOpen) return list;
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT cq.*, q.title, q.description, q.type, q.chapter, q.rewards
                    FROM character_quests cq
                    JOIN quests q ON cq.quest_id = q.id
                    WHERE cq.character_id = :character_id AND cq.status = 'completed')");
    query.bindValue(":character_id", characterId);
    if (query.exec()) {
        while (query.next()) {
            QVariantMap row;
            QSqlRecord rec = query.record();
            for (int i = 0; i < rec.count(); ++i) {
                row[rec.fieldName(i)] = query.value(i);
            }
            list << row;
        }
    }
    return list;
}

// ================= 关卡管理 =================

bool DatabaseManager::unlockLevel(const QString& characterId, const QString& levelId) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(INSERT INTO character_level_progress (character_id, level_id, status, score, stars, completed_at, created_at, updated_at)
                    VALUES (:character_id, :level_id, 'unlocked', 0, 0, NULL, :created_at, :updated_at))");
    query.bindValue(":character_id", characterId);
    query.bindValue(":level_id", levelId);
    query.bindValue(":created_at", QDateTime::currentDateTime());
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("unlockLevel", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    emit levelUnlocked(characterId, levelId);
    return true;
}

bool DatabaseManager::updateLevelProgress(const QString& characterId, const QString& levelId, const QVariantMap& progress) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(UPDATE character_level_progress SET status = :status, score = :score, stars = :stars, 
                    completed_at = :completed_at, updated_at = :updated_at
                    WHERE character_id = :character_id AND level_id = :level_id)");
    query.bindValue(":character_id", characterId);
    query.bindValue(":level_id", levelId);
    query.bindValue(":status", progress.value("status", "in_progress").toString());
    query.bindValue(":score", progress.value("score", 0).toInt());
    query.bindValue(":stars", progress.value("stars", 0).toInt());
    query.bindValue(":completed_at", progress.value("completed_at").toDateTime());
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("updateLevelProgress", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    return true;
}

QVariantList DatabaseManager::getUnlockedLevels(const QString& characterId) {
    QMutexLocker locker(&m_mutex);
    QVariantList list;
    if (!m_isOpen) return list;
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT clp.*, l.name, l.type, l.chapter, l.difficulty, l.enemies, l.rewards
                    FROM character_level_progress clp
                    JOIN levels l ON clp.level_id = l.id
                    WHERE clp.character_id = :character_id)");
    query.bindValue(":character_id", characterId);
    if (query.exec()) {
        while (query.next()) {
            QVariantMap row;
            QSqlRecord rec = query.record();
            for (int i = 0; i < rec.count(); ++i) {
                row[rec.fieldName(i)] = query.value(i);
            }
            list << row;
        }
    }
    return list;
}

QVariantMap DatabaseManager::getLevelProgress(const QString& characterId, const QString& levelId) {
    QMutexLocker locker(&m_mutex);
    QVariantMap result;
    if (!m_isOpen) return result;
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT clp.*, l.name, l.type, l.chapter, l.difficulty, l.enemies, l.rewards
                    FROM character_level_progress clp
                    JOIN levels l ON clp.level_id = l.id
                    WHERE clp.character_id = :character_id AND clp.level_id = :level_id)");
    query.bindValue(":character_id", characterId);
    query.bindValue(":level_id", levelId);
    if (query.exec() && query.next()) {
        QSqlRecord rec = query.record();
        for (int i = 0; i < rec.count(); ++i) {
            result[rec.fieldName(i)] = query.value(i);
        }
    }
    return result;
}

// ================= 成就管理 =================

bool DatabaseManager::unlockAchievement(const QString& characterId, const QString& achievementId) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(INSERT INTO character_achievements (character_id, achievement_id, unlocked, unlocked_at, created_at, updated_at)
                    VALUES (:character_id, :achievement_id, 1, :unlocked_at, :created_at, :updated_at))");
    query.bindValue(":character_id", characterId);
    query.bindValue(":achievement_id", achievementId);
    query.bindValue(":unlocked_at", QDateTime::currentDateTime());
    query.bindValue(":created_at", QDateTime::currentDateTime());
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("unlockAchievement", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    emit achievementUnlocked(characterId, achievementId);
    return true;
}

bool DatabaseManager::isAchievementUnlocked(const QString& characterId, const QString& achievementId) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare("SELECT unlocked FROM character_achievements WHERE character_id = :character_id AND achievement_id = :achievement_id");
    query.bindValue(":character_id", characterId);
    query.bindValue(":achievement_id", achievementId);
    if (query.exec() && query.next()) {
        return query.value(0).toBool();
    }
    return false;
}

QVariantList DatabaseManager::getUnlockedAchievements(const QString& characterId) {
    QMutexLocker locker(&m_mutex);
    QVariantList list;
    if (!m_isOpen) return list;
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT ca.*, a.name, a.description, a.type, a.category, a.condition, a.rewards
                    FROM character_achievements ca
                    JOIN achievements a ON ca.achievement_id = a.id
                    WHERE ca.character_id = :character_id AND ca.unlocked = 1)");
    query.bindValue(":character_id", characterId);
    if (query.exec()) {
        while (query.next()) {
            QVariantMap row;
            QSqlRecord rec = query.record();
            for (int i = 0; i < rec.count(); ++i) {
                row[rec.fieldName(i)] = query.value(i);
            }
            list << row;
        }
    }
    return list;
}

QVariantList DatabaseManager::getAvailableAchievements(const QString& characterId) {
    QMutexLocker locker(&m_mutex);
    QVariantList list;
    if (!m_isOpen) return list;
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT a.*, COALESCE(ca.unlocked, 0) as is_unlocked
                    FROM achievements a
                    LEFT JOIN character_achievements ca ON a.id = ca.achievement_id AND ca.character_id = :character_id)");
    query.bindValue(":character_id", characterId);
    if (query.exec()) {
        while (query.next()) {
            QVariantMap row;
            QSqlRecord rec = query.record();
            for (int i = 0; i < rec.count(); ++i) {
                row[rec.fieldName(i)] = query.value(i);
            }
            list << row;
        }
    }
    return list;
}

// ================= 战斗记录管理 =================

bool DatabaseManager::recordBattle(const QString& characterId, const QVariantMap& battleData) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(INSERT INTO battle_records (character_id, battle_type, enemy_type, enemy_count, result, 
                    damage_dealt, damage_taken, experience_gained, gold_earned, items_earned, battle_duration, 
                    created_at, updated_at)
                    VALUES (:character_id, :battle_type, :enemy_type, :enemy_count, :result, 
                    :damage_dealt, :damage_taken, :experience_gained, :gold_earned, :items_earned, :battle_duration,
                    :created_at, :updated_at))");
    query.bindValue(":character_id", characterId);
    query.bindValue(":battle_type", battleData.value("battle_type", "normal").toString());
    query.bindValue(":enemy_type", battleData.value("enemy_type", "").toString());
    query.bindValue(":enemy_count", battleData.value("enemy_count", 1).toInt());
    query.bindValue(":result", battleData.value("result", "victory").toString());
    query.bindValue(":damage_dealt", battleData.value("damage_dealt", 0).toInt());
    query.bindValue(":damage_taken", battleData.value("damage_taken", 0).toInt());
    query.bindValue(":experience_gained", battleData.value("experience_gained", 0).toInt());
    query.bindValue(":gold_earned", battleData.value("gold_earned", 0).toInt());
    query.bindValue(":items_earned", serializeToJson(battleData.value("items_earned", QVariantList()).toList()));
    query.bindValue(":battle_duration", battleData.value("battle_duration", 0).toInt());
    query.bindValue(":created_at", QDateTime::currentDateTime());
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("recordBattle", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    emit battleRecorded(characterId, battleData);
    return true;
}

QVariantList DatabaseManager::getBattleHistory(const QString& characterId, int limit) {
    QMutexLocker locker(&m_mutex);
    QVariantList list;
    if (!m_isOpen) return list;
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT * FROM battle_records 
                    WHERE character_id = :character_id 
                    ORDER BY created_at DESC 
                    LIMIT :limit)");
    query.bindValue(":character_id", characterId);
    query.bindValue(":limit", limit);
    if (query.exec()) {
        while (query.next()) {
            QVariantMap row;
            QSqlRecord rec = query.record();
            for (int i = 0; i < rec.count(); ++i) {
                row[rec.fieldName(i)] = query.value(i);
            }
            list << row;
        }
    }
    return list;
}

QVariantMap DatabaseManager::getBattleStatistics(const QString& characterId) {
    QMutexLocker locker(&m_mutex);
    QVariantMap result;
    if (!m_isOpen) return result;
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT 
                        COUNT(*) as total_battles,
                        SUM(CASE WHEN result = 'victory' THEN 1 ELSE 0 END) as victories,
                        SUM(CASE WHEN result = 'defeat' THEN 1 ELSE 0 END) as defeats,
                        SUM(damage_dealt) as total_damage_dealt,
                        SUM(damage_taken) as total_damage_taken,
                        SUM(experience_gained) as total_experience,
                        SUM(gold_earned) as total_gold,
                        AVG(battle_duration) as avg_battle_duration
                    FROM battle_records 
                    WHERE character_id = :character_id)");
    query.bindValue(":character_id", characterId);
    if (query.exec() && query.next()) {
        QSqlRecord rec = query.record();
        for (int i = 0; i < rec.count(); ++i) {
            result[rec.fieldName(i)] = query.value(i);
        }
    }
    return result;
}

// ================= 状态效果管理 =================

bool DatabaseManager::addStatusEffect(const QString& characterId, const QString& effectType, const QString& effectName, 
                                     float value, float duration, const QString& source) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(INSERT INTO status_effects (character_id, effect_type, effect_name, value, duration, 
                    remaining_time, source, created_at, updated_at)
                    VALUES (:character_id, :effect_type, :effect_name, :value, :duration, 
                    :remaining_time, :source, :created_at, :updated_at))");
    query.bindValue(":character_id", characterId);
    query.bindValue(":effect_type", effectType);
    query.bindValue(":effect_name", effectName);
    query.bindValue(":value", value);
    query.bindValue(":duration", duration);
    query.bindValue(":remaining_time", duration);
    query.bindValue(":source", source);
    query.bindValue(":created_at", QDateTime::currentDateTime());
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("addStatusEffect", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    return true;
}

bool DatabaseManager::removeStatusEffect(const QString& characterId, const QString& effectType) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM status_effects WHERE character_id = :character_id AND effect_type = :effect_type");
    query.bindValue(":character_id", characterId);
    query.bindValue(":effect_type", effectType);
    bool ok = query.exec();
    if (!ok) {
        logError("removeStatusEffect", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    return true;
}

QVariantList DatabaseManager::getActiveStatusEffects(const QString& characterId) {
    QMutexLocker locker(&m_mutex);
    QVariantList list;
    if (!m_isOpen) return list;
    QSqlQuery query(m_db);
    query.prepare(R"(SELECT * FROM status_effects 
                    WHERE character_id = :character_id AND remaining_time > 0
                    ORDER BY created_at DESC)");
    query.bindValue(":character_id", characterId);
    if (query.exec()) {
        while (query.next()) {
            QVariantMap row;
            QSqlRecord rec = query.record();
            for (int i = 0; i < rec.count(); ++i) {
                row[rec.fieldName(i)] = query.value(i);
            }
            list << row;
        }
    }
    return list;
}

bool DatabaseManager::updateStatusEffectDuration(const QString& characterId, const QString& effectType, float remainingTime) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(UPDATE status_effects SET remaining_time = :remaining_time, updated_at = :updated_at
                    WHERE character_id = :character_id AND effect_type = :effect_type)");
    query.bindValue(":character_id", characterId);
    query.bindValue(":effect_type", effectType);
    query.bindValue(":remaining_time", remainingTime);
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("updateStatusEffectDuration", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    return true;
}

// ================= 统计数据管理 =================

bool DatabaseManager::updateStatistic(const QString& characterId, const QString& statKey, const QVariant& value) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(INSERT INTO statistics (character_id, stat_key, stat_value, created_at, updated_at)
                    VALUES (:character_id, :stat_key, :stat_value, :created_at, :updated_at)
                    ON CONFLICT(character_id, stat_key) DO UPDATE SET
                    stat_value = :stat_value, updated_at = :updated_at)");
    query.bindValue(":character_id", characterId);
    query.bindValue(":stat_key", statKey);
    query.bindValue(":stat_value", value);
    query.bindValue(":created_at", QDateTime::currentDateTime());
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("updateStatistic", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    return true;
}

QVariant DatabaseManager::getStatistic(const QString& characterId, const QString& statKey, const QVariant& defaultValue) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return defaultValue;
    QSqlQuery query(m_db);
    query.prepare("SELECT stat_value FROM statistics WHERE character_id = :character_id AND stat_key = :stat_key");
    query.bindValue(":character_id", characterId);
    query.bindValue(":stat_key", statKey);
    if (query.exec() && query.next()) {
        return query.value(0);
    }
    return defaultValue;
}

QVariantMap DatabaseManager::getAllStatistics(const QString& characterId) {
    QMutexLocker locker(&m_mutex);
    QVariantMap result;
    if (!m_isOpen) return result;
    QSqlQuery query(m_db);
    query.prepare("SELECT stat_key, stat_value FROM statistics WHERE character_id = :character_id");
    query.bindValue(":character_id", characterId);
    if (query.exec()) {
        while (query.next()) {
            QString key = query.value("stat_key").toString();
            QVariant value = query.value("stat_value");
            result[key] = value;
        }
    }
    return result;
}

// ================= 存档管理 =================

bool DatabaseManager::saveGame(const QString& slotId, const QString& characterId, const QVariantMap& gameData) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(INSERT INTO save_data (slot_id, character_id, player_name, chapter, level, playtime, 
                    game_data, save_date, created_at, updated_at)
                    VALUES (:slot_id, :character_id, :player_name, :chapter, :level, :playtime,
                    :game_data, :save_date, :created_at, :updated_at)
                    ON CONFLICT(slot_id) DO UPDATE SET
                    character_id = :character_id, player_name = :player_name, chapter = :chapter,
                    level = :level, playtime = :playtime, game_data = :game_data,
                    save_date = :save_date, updated_at = :updated_at)");
    query.bindValue(":slot_id", slotId);
    query.bindValue(":character_id", characterId);
    query.bindValue(":player_name", gameData.value("player_name", "").toString());
    query.bindValue(":chapter", gameData.value("chapter", 1).toInt());
    query.bindValue(":level", gameData.value("level", 1).toInt());
    query.bindValue(":playtime", gameData.value("playtime", 0).toInt());
    query.bindValue(":game_data", serializeToJson(gameData));
    query.bindValue(":save_date", QDateTime::currentDateTime());
    query.bindValue(":created_at", QDateTime::currentDateTime());
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("saveGame", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    emit gameSaved(slotId);
    return true;
}

QVariantMap DatabaseManager::loadGame(const QString& slotId) {
    QMutexLocker locker(&m_mutex);
    QVariantMap result;
    if (!m_isOpen) return result;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM save_data WHERE slot_id = :slot_id");
    query.bindValue(":slot_id", slotId);
    if (query.exec() && query.next()) {
        QSqlRecord rec = query.record();
        for (int i = 0; i < rec.count(); ++i) {
            result[rec.fieldName(i)] = query.value(i);
        }
        // 解析游戏数据
        QString gameDataStr = query.value("game_data").toString();
        if (!gameDataStr.isEmpty()) {
            QVariantMap gameData = parseJsonField(gameDataStr);
            result["game_data"] = gameData;
        }
    }
    if (!result.isEmpty()) {
        emit gameLoaded(slotId);
    }
    return result;
}

bool DatabaseManager::deleteSave(const QString& slotId) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM save_data WHERE slot_id = :slot_id");
    query.bindValue(":slot_id", slotId);
    bool ok = query.exec();
    if (!ok) {
        logError("deleteSave", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    return true;
}

QStringList DatabaseManager::getAllSaveSlots() {
    QMutexLocker locker(&m_mutex);
    QStringList slots;
    if (!m_isOpen) return slots;
    QSqlQuery query(m_db);
    if (query.exec("SELECT slot_id FROM save_data ORDER BY save_date DESC")) {
        while (query.next()) {
            slots << query.value(0).toString();
        }
    }
    return slots;
}

// ================= 配置管理 =================

bool DatabaseManager::setConfig(const QString& key, const QVariant& value, const QString& description) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(INSERT INTO config (config_key, config_value, description, created_at, updated_at)
                    VALUES (:config_key, :config_value, :description, :created_at, :updated_at)
                    ON CONFLICT(config_key) DO UPDATE SET
                    config_value = :config_value, description = :description, updated_at = :updated_at)");
    query.bindValue(":config_key", key);
    query.bindValue(":config_value", value);
    query.bindValue(":description", description);
    query.bindValue(":created_at", QDateTime::currentDateTime());
    query.bindValue(":updated_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("setConfig", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    return true;
}

QVariant DatabaseManager::getConfig(const QString& key, const QVariant& defaultValue) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return defaultValue;
    QSqlQuery query(m_db);
    query.prepare("SELECT config_value FROM config WHERE config_key = :config_key");
    query.bindValue(":config_key", key);
    if (query.exec() && query.next()) {
        return query.value(0);
    }
    return defaultValue;
}

bool DatabaseManager::deleteConfig(const QString& key) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM config WHERE config_key = :config_key");
    query.bindValue(":config_key", key);
    bool ok = query.exec();
    if (!ok) {
        logError("deleteConfig", query.lastError());
        emit errorOccurred(query.lastError().text());
        return false;
    }
    return true;
}

QVariantMap DatabaseManager::getConfigsByCategory(const QString& category) {
    QMutexLocker locker(&m_mutex);
    QVariantMap result;
    if (!m_isOpen) return result;
    QSqlQuery query(m_db);
    query.prepare("SELECT config_key, config_value FROM config WHERE config_key LIKE :category_pattern");
    query.bindValue(":category_pattern", category + "%");
    if (query.exec()) {
        while (query.next()) {
            QString key = query.value("config_key").toString();
            QVariant value = query.value("config_value");
            result[key] = value;
        }
    }
    return result;
}

// ================= 日志管理 =================

bool DatabaseManager::logEvent(const QString& characterId, const QString& level, const QString& category, 
                              const QString& message, const QVariantMap& data) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare(R"(INSERT INTO game_logs (character_id, log_level, log_category, message, log_data, created_at)
                    VALUES (:character_id, :log_level, :log_category, :message, :log_data, :created_at))");
    query.bindValue(":character_id", characterId);
    query.bindValue(":log_level", level);
    query.bindValue(":log_category", category);
    query.bindValue(":message", message);
    query.bindValue(":log_data", serializeToJson(data));
    query.bindValue(":created_at", QDateTime::currentDateTime());
    bool ok = query.exec();
    if (!ok) {
        logError("logEvent", query.lastError());
        return false;
    }
    return true;
}

QVariantList DatabaseManager::getLogs(const QString& characterId, const QString& level, int limit) {
    QMutexLocker locker(&m_mutex);
    QVariantList list;
    if (!m_isOpen) return list;
    QString sql = "SELECT * FROM game_logs WHERE 1=1";
    QVariantMap params;
    if (!characterId.isEmpty()) {
        sql += " AND character_id = :character_id";
        params[":character_id"] = characterId;
    }
    if (!level.isEmpty()) {
        sql += " AND log_level = :log_level";
        params[":log_level"] = level;
    }
    sql += " ORDER BY created_at DESC LIMIT :limit";
    params[":limit"] = limit;
    
    QSqlQuery query(m_db);
    query.prepare(sql);
    for (auto it = params.begin(); it != params.end(); ++it) {
        query.bindValue(it.key(), it.value());
    }
    if (query.exec()) {
        while (query.next()) {
            QVariantMap row;
            QSqlRecord rec = query.record();
            for (int i = 0; i < rec.count(); ++i) {
                row[rec.fieldName(i)] = query.value(i);
            }
            list << row;
        }
    }
    return list;
}

// ================= 事务管理 =================

bool DatabaseManager::beginTransaction() {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen || m_inTransaction) return false;
    
    if (executeQuery("BEGIN TRANSACTION")) {
        m_inTransaction = true;
        emit transactionStarted();
        return true;
    }
    return false;
}

bool DatabaseManager::commitTransaction() {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen || !m_inTransaction) return false;
    
    if (executeQuery("COMMIT")) {
        m_inTransaction = false;
        emit transactionCommitted();
        return true;
    }
    return false;
}

bool DatabaseManager::rollbackTransaction() {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen || !m_inTransaction) return false;
    
    if (executeQuery("ROLLBACK")) {
        m_inTransaction = false;
        emit transactionRolledBack();
        return true;
    }
    return false;
}

bool DatabaseManager::isInTransaction() const {
    return m_inTransaction;
}

// ================= 批量操作 =================

bool DatabaseManager::batchInsert(const QString& tableName, const QVariantList& data) {
    if (data.isEmpty()) return true;
    
    if (!beginTransaction()) return false;
    
    for (const QVariant& item : data) {
        QVariantMap row = item.toMap();
        QStringList columns = row.keys();
        QStringList placeholders;
        
        for (const QString& column : columns) {
            placeholders << QString(":%1").arg(column);
        }
        
        QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)")
                     .arg(tableName, columns.join(", "), placeholders.join(", "));
        
        if (!executeQuery(sql, row)) {
            rollbackTransaction();
            return false;
        }
    }
    
    return commitTransaction();
}

bool DatabaseManager::batchUpdate(const QString& tableName, const QString& whereClause, const QVariantMap& data) {
    QStringList setClauses;
    for (auto it = data.begin(); it != data.end(); ++it) {
        setClauses << QString("%1 = :%1").arg(it.key());
    }
    
    QString sql = QString("UPDATE %1 SET %2 WHERE %3")
                 .arg(tableName, setClauses.join(", "), whereClause);
    
    return executeQuery(sql, data);
}

bool DatabaseManager::batchDelete(const QString& tableName, const QString& whereClause) {
    QString sql = QString("DELETE FROM %1 WHERE %2").arg(tableName, whereClause);
    return executeQuery(sql);
}

// ================= 查询构建器 =================

DatabaseManager::QueryBuilder DatabaseManager::query() {
    return QueryBuilder(this);
}

DatabaseManager::QueryBuilder::QueryBuilder(DatabaseManager* manager)
    : m_manager(manager), m_tableName(""), m_selectColumns("*"), m_whereConditions(), m_orderBy(""), m_limit(-1) {}

DatabaseManager::QueryBuilder& DatabaseManager::QueryBuilder::select(const QString& columns) {
    m_selectColumns = columns;
    return *this;
}

DatabaseManager::QueryBuilder& DatabaseManager::QueryBuilder::from(const QString& tableName) {
    m_tableName = tableName;
    return *this;
}

DatabaseManager::QueryBuilder& DatabaseManager::QueryBuilder::where(const QString& column, const QString& operator_, const QVariant& value) {
    m_whereConditions.append({column, operator_, value});
    return *this;
}

DatabaseManager::QueryBuilder& DatabaseManager::QueryBuilder::orderBy(const QString& column, const QString& direction) {
    m_orderBy = QString("%1 %2").arg(column, direction);
    return *this;
}

DatabaseManager::QueryBuilder& DatabaseManager::QueryBuilder::limit(int count) {
    m_limit = count;
    return *this;
}

QVariantList DatabaseManager::QueryBuilder::execute() {
    if (!m_manager || m_tableName.isEmpty()) return QVariantList();
    
    QString sql = QString("SELECT %1 FROM %2").arg(m_selectColumns, m_tableName);
    
    QVariantMap params;
    if (!m_whereConditions.isEmpty()) {
        QStringList whereClauses;
        for (int i = 0; i < m_whereConditions.size(); ++i) {
            const auto& condition = m_whereConditions[i];
            QString placeholder = QString(":where_%1").arg(i);
            whereClauses << QString("%1 %2 %3").arg(condition.column, condition.operator_, placeholder);
            params[placeholder] = condition.value;
        }
        sql += " WHERE " + whereClauses.join(" AND ");
    }
    
    if (!m_orderBy.isEmpty()) {
        sql += " ORDER BY " + m_orderBy;
    }
    
    if (m_limit > 0) {
        sql += QString(" LIMIT %1").arg(m_limit);
    }
    
    return m_manager->executeSelectQuery(sql, params);
}

QVariantMap DatabaseManager::QueryBuilder::executeOne() {
    QVariantList results = execute();
    if (!results.isEmpty()) {
        return results.first().toMap();
    }
    return QVariantMap();
}

bool DatabaseManager::QueryBuilder::executeNonQuery() {
    if (!m_manager || m_tableName.isEmpty()) return false;
    
    QString sql = QString("DELETE FROM %1").arg(m_tableName);
    
    QVariantMap params;
    if (!m_whereConditions.isEmpty()) {
        QStringList whereClauses;
        for (int i = 0; i < m_whereConditions.size(); ++i) {
            const auto& condition = m_whereConditions[i];
            QString placeholder = QString(":where_%1").arg(i);
            whereClauses << QString("%1 %2 %3").arg(condition.column, condition.operator_, placeholder);
            params[placeholder] = condition.value;
        }
        sql += " WHERE " + whereClauses.join(" AND ");
    }
    
    return m_manager->executeQuery(sql, params);
}

// ================= 工具函数 =================

bool DatabaseManager::executeQuery(const QString& sql, const QVariantMap& parameters) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    
    QElapsedTimer timer;
    timer.start();
    
    QSqlQuery query(m_db);
    query.prepare(sql);
    
    for (auto it = parameters.begin(); it != parameters.end(); ++it) {
        query.bindValue(it.key(), it.value());
    }
    
    bool ok = query.exec();
    
    qint64 duration = timer.elapsed();
    logQuery(sql, parameters, duration);
    
    if (!ok) {
        logError("executeQuery", query.lastError());
        emit errorOccurred(query.lastError().text());
    }
    
    return ok;
}

QVariantList DatabaseManager::executeSelectQuery(const QString& sql, const QVariantMap& parameters) {
    QMutexLocker locker(&m_mutex);
    QVariantList results;
    if (!m_isOpen) return results;
    
    QElapsedTimer timer;
    timer.start();
    
    QSqlQuery query(m_db);
    query.prepare(sql);
    
    for (auto it = parameters.begin(); it != parameters.end(); ++it) {
        query.bindValue(it.key(), it.value());
    }
    
    if (query.exec()) {
        while (query.next()) {
            QVariantMap row;
            QSqlRecord rec = query.record();
            for (int i = 0; i < rec.count(); ++i) {
                row[rec.fieldName(i)] = query.value(i);
            }
            results << row;
        }
    } else {
        logError("executeSelectQuery", query.lastError());
        emit errorOccurred(query.lastError().text());
    }
    
    qint64 duration = timer.elapsed();
    logQuery(sql, parameters, duration);
    
    return results;
}

QVariantMap DatabaseManager::executeSelectOneQuery(const QString& sql, const QVariantMap& parameters) {
    QVariantList results = executeSelectQuery(sql, parameters);
    if (!results.isEmpty()) {
        return results.first().toMap();
    }
    return QVariantMap();
}

void DatabaseManager::logQuery(const QString& sql, const QVariantMap& parameters, qint64 duration) {
    if (m_queryLoggingEnabled) {
        QVariantMap logEntry;
        logEntry["sql"] = sql;
        logEntry["parameters"] = parameters;
        logEntry["duration"] = duration;
        logEntry["timestamp"] = QDateTime::currentDateTime();
        m_queryLog.append(logEntry);
        
        // 限制日志大小
        if (m_queryLog.size() > 1000) {
            m_queryLog.removeFirst();
        }
    }
}

QString DatabaseManager::buildWhereClause(const QVariantMap& conditions) {
    if (conditions.isEmpty()) return "";
    
    QStringList clauses;
    for (auto it = conditions.begin(); it != conditions.end(); ++it) {
        clauses << QString("%1 = :%1").arg(it.key());
    }
    return clauses.join(" AND ");
}

QVariantMap DatabaseManager::parseJsonField(const QString& jsonString) {
    QVariantMap result;
    if (jsonString.isEmpty()) return result;
    
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    if (doc.isObject()) {
        result = doc.object().toVariantMap();
    }
    return result;
}

QString DatabaseManager::serializeToJson(const QVariantMap& data) {
    QJsonDocument doc = QJsonDocument::fromVariantMap(data);
    return QString::fromUtf8(doc.toJson());
}

QString DatabaseManager::serializeToJson(const QVariantList& data) {
    QJsonDocument doc = QJsonDocument::fromVariantList(data);
    return QString::fromUtf8(doc.toJson());
}

// ================= 存在性检查 =================

bool DatabaseManager::checkCharacterExists(const QString& characterId) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare("SELECT 1 FROM characters WHERE id = :id");
    query.bindValue(":id", characterId);
    return query.exec() && query.next();
}

bool DatabaseManager::checkSkillExists(const QString& skillId) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare("SELECT 1 FROM skills WHERE id = :id");
    query.bindValue(":id", skillId);
    return query.exec() && query.next();
}

bool DatabaseManager::checkEquipmentExists(const QString& equipmentId) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare("SELECT 1 FROM equipment WHERE id = :id");
    query.bindValue(":id", equipmentId);
    return query.exec() && query.next();
}

bool DatabaseManager::checkItemExists(const QString& itemId) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare("SELECT 1 FROM items WHERE id = :id");
    query.bindValue(":id", itemId);
    return query.exec() && query.next();
}

bool DatabaseManager::checkQuestExists(const QString& questId) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare("SELECT 1 FROM quests WHERE id = :id");
    query.bindValue(":id", questId);
    return query.exec() && query.next();
}

bool DatabaseManager::checkLevelExists(const QString& levelId) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare("SELECT 1 FROM levels WHERE id = :id");
    query.bindValue(":id", levelId);
    return query.exec() && query.next();
}

bool DatabaseManager::checkAchievementExists(const QString& achievementId) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    QSqlQuery query(m_db);
    query.prepare("SELECT 1 FROM achievements WHERE id = :id");
    query.bindValue(":id", achievementId);
    return query.exec() && query.next();
}

// ================= 数据验证 =================

bool DatabaseManager::validateCharacterData(const QVariantMap& data) {
    // 基本验证
    if (!data.contains("name") || data["name"].toString().isEmpty()) {
        return false;
    }
    if (!data.contains("class") || data["class"].toString().isEmpty()) {
        return false;
    }
    if (data.contains("level") && data["level"].toInt() < 1) {
        return false;
    }
    return true;
}

bool DatabaseManager::validateQuestData(const QVariantMap& data) {
    // 基本验证
    if (!data.contains("title") || data["title"].toString().isEmpty()) {
        return false;
    }
    if (!data.contains("type") || data["type"].toString().isEmpty()) {
        return false;
    }
    return true;
}

bool DatabaseManager::validateLevelData(const QVariantMap& data) {
    // 基本验证
    if (!data.contains("name") || data["name"].toString().isEmpty()) {
        return false;
    }
    if (!data.contains("type") || data["type"].toString().isEmpty()) {
        return false;
    }
    return true;
}

// ================= 数据导入导出 =================

bool DatabaseManager::exportData(const QString& exportPath, const QStringList& tables) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    
    QFile file(exportPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out.setCodec("UTF-8");
    
    QStringList tableList = tables.isEmpty() ? QStringList{"characters", "skills", "equipment", "items", "quests", "levels", "achievements"} : tables;
    
    for (const QString& tableName : tableList) {
        QVariantList data = executeSelectQuery(QString("SELECT * FROM %1").arg(tableName));
        if (!data.isEmpty()) {
            out << QString("-- Table: %1\n").arg(tableName);
            out << QString("INSERT INTO %1 VALUES\n").arg(tableName);
            
            QStringList values;
            for (const QVariant& row : data) {
                QVariantMap rowMap = row.toMap();
                QStringList rowValues;
                for (const QVariant& value : rowMap.values()) {
                    if (value.isNull()) {
                        rowValues << "NULL";
                    } else if (value.type() == QVariant::String) {
                        rowValues << QString("'%1'").arg(value.toString().replace("'", "''"));
                    } else {
                        rowValues << value.toString();
                    }
                }
                values << QString("(%1)").arg(rowValues.join(", "));
            }
            out << values.join(",\n") << ";\n\n";
        }
    }
    
    file.close();
    return true;
}

bool DatabaseManager::importData(const QString& importPath) {
    QMutexLocker locker(&m_mutex);
    if (!m_isOpen) return false;
    
    QFile file(importPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream in(&file);
    in.setCodec("UTF-8");
    
    if (!beginTransaction()) {
        file.close();
        return false;
    }
    
    QString sql;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("--") || line.isEmpty()) {
            continue;
        }
        sql += line + " ";
        if (line.endsWith(";")) {
            if (!executeQuery(sql)) {
                rollbackTransaction();
                file.close();
                return false;
            }
            sql.clear();
        }
    }
    
    file.close();
    return commitTransaction();
}

// ================= 性能统计 =================

QVariantMap DatabaseManager::getPerformanceStats() {
    QMutexLocker locker(&m_mutex);
    QVariantMap stats;
    if (!m_isOpen) return stats;
    
    // 数据库大小
    stats["database_size"] = getDatabaseSize();
    
    // 表记录数
    QStringList tables = {"characters", "skills", "equipment", "items", "quests", "levels", "achievements"};
    for (const QString& table : tables) {
        QVariantList result = executeSelectQuery(QString("SELECT COUNT(*) as count FROM %1").arg(table));
        if (!result.isEmpty()) {
            stats[QString("%1_count").arg(table)] = result.first().toMap()["count"];
        }
    }
    
    // 查询日志统计
    if (m_queryLoggingEnabled) {
        stats["query_count"] = m_queryLog.size();
        if (!m_queryLog.isEmpty()) {
            qint64 totalDuration = 0;
            for (const QVariant& logEntry : m_queryLog) {
                totalDuration += logEntry.toMap()["duration"].toLongLong();
            }
            stats["avg_query_duration"] = totalDuration / m_queryLog.size();
        }
    }
    
    return stats;
}

bool DatabaseManager::enableQueryLogging(bool enable) {
    m_queryLoggingEnabled = enable;
    if (!enable) {
        m_queryLog.clear();
    }
    return true;
}

QVariantList DatabaseManager::getQueryLog() {
    return m_queryLog;
}

// ================= 定时器槽函数 =================

void DatabaseManager::onAutoSaveTimer() {
    // 自动保存逻辑
    if (m_isOpen && !m_inTransaction) {
        // 可以在这里实现自动保存逻辑
        // 例如：保存当前游戏状态
    }
}

void DatabaseManager::onCleanupTimer() {
    // 清理逻辑
    if (m_isOpen) {
        // 清理过期的状态效果
        executeQuery("DELETE FROM status_effects WHERE remaining_time <= 0");
        
        // 清理过期的日志（保留最近7天）
        executeQuery("DELETE FROM game_logs WHERE created_at < datetime('now', '-7 days')");
        
        // 优化数据库
        optimizeDatabase();
    }
}
