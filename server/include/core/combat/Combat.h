#pragma once

#include <QObject>
#include <QList>
#include "Character.h"

class Combat : public QObject {
    Q_OBJECT
    
public:
    enum class CombatState {
        PREPARING,
        IN_PROGRESS,
        FINISHED
    };
    
    Combat();
    ~Combat();
    
    // 战斗控制
    void startCombat(Character* player, const QList<Character*>& enemies);
    void endCombat();
    void update(float deltaTime);
    
    // 战斗操作
    void playerAttack(int skillIndex, int targetIndex);
    void processEnemyTurns();
    
    // 获取器
    CombatState getState() const { return m_state; }
    Character* getPlayer() const { return m_player; }
    const QList<Character*>& getEnemies() const { return m_enemies; }
    bool isPlayerTurn() const { return m_isPlayerTurn; }
    
signals:
    void combatStarted();
    void combatEnded(bool playerWon);
    void turnChanged(bool isPlayerTurn);
    void enemyDefeated(Character* enemy);
    
private:
    void checkCombatEnd();
    void executeAction(Character* attacker, Character* target, int skillIndex);
    float calculateDamage(Character* attacker, Character* target, int skillIndex);
    
    CombatState m_state;
    Character* m_player;
    QList<Character*> m_enemies;
    bool m_isPlayerTurn;
    float m_turnTimer;
};