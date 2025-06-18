/**
 * @file Combat.h
 * @brief 战斗系统 - 游戏战斗管理
 * @author [pengchengkang]
 * @date 2025.06.17
 * 
 * 功能特性:
 * - 回合制战斗
 * - 实时战斗
 * - 技能系统
 * - 伤害计算
 * - 状态效果
 * - AI行为
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <functional>
#include <variant>
#include <queue>

namespace Fantasy {

// 前向声明
class Character;

// 战斗类型枚举
enum class CombatType {
    TURN_BASED,     // 回合制
    REAL_TIME,      // 实时
    AUTO_BATTLE,    // 自动战斗
    PVP,            // 玩家对战
    PVE             // 玩家对环境
};

// 战斗状态枚举
enum class CombatState {
    PREPARING,      // 准备中
    ACTIVE,         // 进行中
    PAUSED,         // 暂停
    FINISHED,       // 结束
    VICTORY,        // 胜利
    DEFEAT          // 失败
};

// 行动类型枚举
enum class ActionType {
    ATTACK,         // 攻击
    SKILL,          // 技能
    ITEM,           // 物品
    DEFEND,         // 防御
    FLEE,           // 逃跑
    WAIT            // 等待
};

// 伤害类型枚举
enum class DamageType {
    PHYSICAL,       // 物理
    MAGICAL,        // 魔法
    FIRE,           // 火
    ICE,            // 冰
    LIGHTNING,      // 雷
    POISON,         // 毒
    HOLY,           // 神圣
    DARK,           // 暗
    TRUE            // 真实伤害
};

// 战斗行动结构
struct CombatAction {
    std::string id;
    ActionType type;
    std::shared_ptr<Character> actor;
    std::shared_ptr<Character> target;
    std::string skillId;
    std::string itemId;
    int priority;
    double executionTime;
    bool executed;
    
    CombatAction() : type(ActionType::WAIT), priority(0), executionTime(0.0), executed(false) {}
    CombatAction(ActionType t, std::shared_ptr<Character> a, std::shared_ptr<Character> target = nullptr)
        : type(t), actor(a), target(target), priority(0), executionTime(0.0), executed(false) {}
};

// 战斗结果结构
struct CombatResult {
    bool hit;
    bool critical;
    int damage;
    DamageType damageType;
    std::vector<std::string> effects;
    bool targetDefeated;
    
    CombatResult() : hit(false), critical(false), damage(0), damageType(DamageType::PHYSICAL), targetDefeated(false) {}
};

// 战斗统计结构
struct CombatStats {
    int totalDamageDealt;
    int totalDamageTaken;
    int criticalHits;
    int skillsUsed;
    int itemsUsed;
    int totalActions;
    int totalKills;
    double combatDuration;
    int turnsTaken;
    int totalEvents;
    
    CombatStats() : totalDamageDealt(0), totalDamageTaken(0), criticalHits(0),
                   skillsUsed(0), itemsUsed(0), totalActions(0), totalKills(0),
                   combatDuration(0.0), turnsTaken(0), totalEvents(0) {}
};

// 战斗事件类型
enum class CombatEventType {
    COMBAT_STARTED,
    COMBAT_ENDED,
    TURN_STARTED,
    TURN_ENDED,
    ACTION_EXECUTED,
    DAMAGE_DEALT,
    DAMAGE_TAKEN,
    CHARACTER_DEFEATED,
    SKILL_USED,
    ITEM_USED,
    STATUS_EFFECT_APPLIED,
    STATUS_EFFECT_REMOVED,
    VICTORY,
    DEFEAT
};

// 战斗事件数据
using CombatEventData = std::unordered_map<std::string, std::string>;

// 战斗事件结构
struct CombatEvent {
    CombatEventType type;
    std::string name;
    CombatEventData data;
    std::chrono::system_clock::time_point timestamp;
    
    CombatEvent(CombatEventType t, const std::string& n, const CombatEventData& d = CombatEventData{})
        : type(t), name(n), data(d), timestamp(std::chrono::system_clock::now()) {}
};

// 事件回调函数类型
using CombatEventCallback = std::function<void(const CombatEvent&)>;

// 战斗配置结构
struct CombatConfig {
    CombatType type;
    int maxTurns;
    double timeLimit;
    bool allowFlee;
    bool allowItems;
    bool allowSkills;
    int maxParticipants;
    bool autoResolve;
    
    CombatConfig() : type(CombatType::TURN_BASED), maxTurns(50), timeLimit(300.0),
                    allowFlee(true), allowItems(true), allowSkills(true), 
                    maxParticipants(10), autoResolve(false) {}
};

// 日志宏定义
#define COMBAT_LOG_DEBUG(fmt, ...) 
#define COMBAT_LOG_INFO(fmt, ...) 
#define COMBAT_LOG_WARN(fmt, ...) 
#define COMBAT_LOG_ERROR(fmt, ...) 

/**
 * @brief 战斗系统类
 * 
 * 提供战斗的基础功能：
 * - 战斗初始化和管理
 * - 行动队列处理
 * - 伤害计算
 * - 状态效果管理
 * - AI决策
 */
class Combat {
public:
    // 构造和析构
    Combat(const CombatConfig& config = CombatConfig{});
    virtual ~Combat();
    
    // 禁用拷贝
    Combat(const Combat&) = delete;
    Combat& operator=(const Combat&) = delete;
    
    // 战斗控制
    virtual bool start(const std::vector<std::shared_ptr<Character>>& participants);
    virtual bool end();
    virtual void update(double deltaTime);
    virtual void pause();
    virtual void resume();
    virtual void reset();
    
    // 战斗操作
    virtual bool executeAction(const CombatAction& action);
    virtual bool queueAction(const CombatAction& action);
    virtual void clearActionQueue();
    virtual void processTurn();
    virtual void processActions(double deltaTime);
    
    // 玩家操作
    virtual bool playerAttack(int targetIndex);
    virtual bool playerUseSkill(const std::string& skillId, int targetIndex);
    virtual bool playerUseItem(const std::string& itemId, int targetIndex);
    virtual bool playerDefend();
    virtual bool playerFlee();
    
    // AI操作
    virtual void processEnemyTurns();
    virtual CombatAction generateAIAction(std::shared_ptr<Character> enemy);
    virtual std::shared_ptr<Character> selectTarget(std::shared_ptr<Character> attacker, 
                           const std::vector<std::shared_ptr<Character>>& targets);
    
    // 伤害计算
    virtual CombatResult calculateDamage(std::shared_ptr<Character> attacker, 
                                       std::shared_ptr<Character> target,
                                       const std::string& skillId = "");
    virtual int calculateBaseDamage(std::shared_ptr<Character> attacker, 
                                  std::shared_ptr<Character> target);
    virtual float calculateCriticalChance(std::shared_ptr<Character> attacker);
    virtual float calculateCriticalMultiplier(std::shared_ptr<Character> attacker);
    virtual float calculateHitChance(std::shared_ptr<Character> attacker, 
                                   std::shared_ptr<Character> target);
    virtual float calculateDodgeChance(std::shared_ptr<Character> target);
    
    // 状态检查
    virtual bool checkCombatEnd();
    virtual bool isPlayerVictory() const;
    virtual bool isPlayerDefeat() const;
    virtual bool canFlee() const;
    virtual bool isPlayerTurn() const { return isPlayerTurn_; }
    virtual bool isActive() const;
    virtual bool isPaused() const;
    
    // 事件系统
    virtual void emitEvent(const CombatEvent& event);
    virtual void emitEvent(CombatEventType type, const std::string& name, 
                          const CombatEventData& data = CombatEventData{});
    virtual void subscribeToEvent(CombatEventType type, CombatEventCallback callback);
    virtual void unsubscribeFromEvent(CombatEventType type, CombatEventCallback callback);
    
    // 获取器
    CombatState getState() const { return state_; }
    const CombatConfig& getConfig() const { return config_; }
    const CombatStats& getStats() const { return stats_; }
    std::shared_ptr<Character> getPlayer() const { return player_; }
    const std::vector<std::shared_ptr<Character>>& getEnemies() const { return enemies_; }
    const std::vector<std::shared_ptr<Character>>& getAllParticipants() const { return participants_; }
    const std::queue<CombatAction>& getActionQueue() const { return actionQueue_; }
    int getCurrentTurn() const { return currentTurn_; }
    double getCombatTime() const { return combatTime_; }
    std::vector<std::shared_ptr<Character>> getParticipants() const;
    std::vector<std::shared_ptr<Character>> getAliveParticipants() const;
    std::shared_ptr<Character> getCurrentActor() const;
    std::shared_ptr<Character> getWinner() const;
    
    // 设置器
    virtual void setConfig(const CombatConfig& config) { config_ = config; }
    virtual void setState(CombatState state) { state_ = state; }

protected:
    // 内部方法
    virtual void onCombatStarted();
    virtual void onCombatEnded();
    virtual void onTurnStarted();
    virtual void onTurnEnded();
    virtual void onActionExecuted(const CombatAction& action, const CombatResult& result);
    virtual void onDamageDealt(std::shared_ptr<Character> target, int damage, DamageType type);
    virtual void onDamageTaken(std::shared_ptr<Character> target, int damage, DamageType type);
    virtual void onCharacterDefeated(std::shared_ptr<Character> character);
    virtual void onVictory();
    virtual void onDefeat();

private:
    // 配置和状态
    CombatConfig config_;
    CombatState state_;
    CombatStats stats_;
    
    // 参与者
    std::shared_ptr<Character> player_;
    std::vector<std::shared_ptr<Character>> enemies_;
    std::vector<std::shared_ptr<Character>> participants_;
    
    // 战斗状态
    bool isPlayerTurn_;
    int currentTurn_;
    double combatTime_;
    double turnTimer_;
    double actionTimer_;
    
    // 行动系统
    std::queue<CombatAction> actionQueue_;
    std::vector<CombatAction> pendingActions_;
    
    // 事件系统
    std::unordered_map<CombatEventType, std::vector<CombatEventCallback>> eventCallbacks_;
    std::vector<CombatEvent> eventQueue_;
    
    // 内部状态
    bool initialized_;
    std::chrono::system_clock::time_point startTime_;
    std::chrono::system_clock::time_point lastUpdateTime_;
    
    // 私有方法
    void initializeCombat();
    void updateTimers(double deltaTime);
    void processEvents();
    void updateStats(double deltaTime);
    void executeQueuedActions();
    void resolveAction(const CombatAction& action);
    void applyDamage(std::shared_ptr<Character> target, const CombatResult& result);
    void checkCharacterStatus(std::shared_ptr<Character> character);
    void removeDefeatedCharacters();
    void switchTurn();
    void generateEnemyActions();
    std::vector<std::shared_ptr<Character>> getAliveEnemies() const;
    bool isValidTarget(std::shared_ptr<Character> target) const;
    int getParticipantIndex(std::shared_ptr<Character> participant) const;
    
    // 新增方法
    bool initParticipants();
    void determineTurnOrder();
    bool validateAction(const CombatAction& action, std::shared_ptr<Character> actor);
    bool validateAttackAction(const CombatAction& action, std::shared_ptr<Character> actor);
    bool validateSkillAction(const CombatAction& action, std::shared_ptr<Character> actor);
    bool validateItemAction(const CombatAction& action, std::shared_ptr<Character> actor);
    bool validateDefendAction(const CombatAction& action, std::shared_ptr<Character> actor);
    bool validateFleeAction(const CombatAction& action, std::shared_ptr<Character> actor);
    CombatResult performAction(const CombatAction& action, std::shared_ptr<Character> actor);
    CombatResult performAttackAction(const CombatAction& action, std::shared_ptr<Character> actor);
    CombatResult performSkillAction(const CombatAction& action, std::shared_ptr<Character> actor);
    CombatResult performItemAction(const CombatAction& action, std::shared_ptr<Character> actor);
    CombatResult performDefendAction(const CombatAction& action, std::shared_ptr<Character> actor);
    CombatResult performFleeAction(const CombatAction& action, std::shared_ptr<Character> actor);
    int calculateDamage(std::shared_ptr<Character> attacker, std::shared_ptr<Character> target, const CombatAction& action);
    void applyDamage(std::shared_ptr<Character> target, int damage);
    double calculateFleeChance(std::shared_ptr<Character> actor);
    void processActionResult(const CombatResult& result);
    void nextTurn();
    void endTurn();
    void determineWinner();
    void cleanupCombat();
    std::shared_ptr<Character> findParticipant(const std::string& id) const;
};

} // namespace Fantasy