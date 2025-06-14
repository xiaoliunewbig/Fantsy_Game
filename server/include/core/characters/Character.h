#pragma once

#include <QObject>
#include <QString>
#include <QVector2D>
#include <memory>

enum class CharacterClass {
    WARRIOR,
    MAGE,
    ASSASSIN
};

struct CharacterStats {
    int level = 1;
    int experience = 0;
    int health = 100;
    int maxHealth = 100;
    int mana = 50;
    int maxMana = 50;
    int attack = 15;
    int defense = 10;
    int speed = 5;
    float criticalRate = 0.05f;
    float criticalDamage = 1.5f;
};

class Character : public QObject {
    Q_OBJECT
    
public:
    Character(const QString& name, int level, CharacterClass classType = CharacterClass::WARRIOR);
    ~Character();
    
    // 基础操作
    void update(float deltaTime);
    void move(const QVector2D& direction);
    void attack(Character* target);
    void useSkill(int skillIndex, Character* target);
    
    // 属性操作
    void takeDamage(int damage);
    void heal(int amount);
    void gainExperience(int exp);
    void levelUp();
    
    // 获取器
    const QString& getName() const { return m_name; }
    CharacterClass getClass() const { return m_class; }
    const CharacterStats& getStats() const { return m_stats; }
    const QVector2D& getPosition() const { return m_position; }
    bool isAlive() const { return m_stats.health > 0; }
    
signals:
    void healthChanged(int current, int maximum);
    void manaChanged(int current, int maximum);
    void levelChanged(int newLevel);
    void experienceGained(int amount);
    void characterDied();
    
private:
    void calculateStats();
    int calculateRequiredExperience(int level) const;
    
    QString m_name;
    CharacterClass m_class;
    CharacterStats m_stats;
    QVector2D m_position;
    QVector2D m_velocity;
    
    // 技能相关
    QList<int> m_skillIds;
    QMap<int, float> m_skillCooldowns;
};