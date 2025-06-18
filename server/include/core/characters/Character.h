/**
 * @file Character.h
 * @brief 角色系统 - 游戏中的角色基类
 * @author [pengchengkang]
 * @date 2025.06.17
 * 
 * 功能特性:
 * - 角色基础属性
 * - 等级和经验系统
 * - 技能系统
 * - 装备系统
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
#include <algorithm>
#include <cmath>
#include "include/core/levels/Level.h"  // 包含Vector2D定义

namespace Fantasy {

// 角色职业枚举
enum class CharacterClass {
    WARRIOR,    // 战士
    MAGE,       // 法师
    ASSASSIN,   // 刺客
    ARCHER,     // 弓箭手
    PRIEST,     // 牧师
    PALADIN,    // 圣骑士
    NECROMANCER, // 死灵法师
    DRUID,      // 德鲁伊
    MONK,       // 武僧
    BERSERKER   // 狂战士
};

// 角色状态枚举
enum class CharacterState {
    IDLE,       // 空闲
    MOVING,     // 移动中
    ATTACKING,  // 攻击中
    CASTING,    // 施法中
    STUNNED,    // 眩晕
    FROZEN,     // 冰冻
    POISONED,   // 中毒
    BURNING,    // 燃烧
    DEAD        // 死亡
};

// 角色属性结构
struct CharacterStats {
    int level = 1;
    int experience = 0;
    int experienceToNext = 100;
    
    // 基础属性
    int health = 100;
    int maxHealth = 100;
    int mana = 50;
    int maxMana = 50;
    int stamina = 100;
    int maxStamina = 100;
    
    // 战斗属性
    int attack = 15;
    int defense = 10;
    int magicAttack = 10;
    int magicDefense = 8;
    int speed = 5;
    int criticalRate = 5;      // 百分比
    int criticalDamage = 150;  // 百分比
    int dodgeRate = 3;         // 百分比
    int blockRate = 2;         // 百分比
    
    // 元素抗性
    int fireResistance = 0;
    int iceResistance = 0;
    int lightningResistance = 0;
    int poisonResistance = 0;
    int holyResistance = 0;
    int darkResistance = 0;
    
    CharacterStats() = default;
    CharacterStats(int lvl, int hp, int mp, int atk, int def, int spd)
        : level(lvl), health(hp), maxHealth(hp), mana(mp), maxMana(mp),
          attack(atk), defense(def), speed(spd) {}
};

// 装备槽位枚举
enum class EquipmentSlot {
    WEAPON,
    SHIELD,
    HELMET,
    ARMOR,
    GLOVES,
    BOOTS,
    RING_1,
    RING_2,
    NECKLACE,
    CAPE
};

// 状态效果结构
struct StatusEffect {
    std::string id;
    std::string name;
    std::string description;
    CharacterState state;
    int duration;  // 持续时间（帧数）
    int remaining; // 剩余时间
    float value;   // 效果值
    bool isPositive; // 是否为正面效果
    
    StatusEffect() : duration(0), remaining(0), value(0.0f), isPositive(false) {}
    StatusEffect(const std::string& id, const std::string& name, CharacterState state, 
                int duration, float value, bool positive = false)
        : id(id), name(name), state(state), duration(duration), remaining(duration),
          value(value), isPositive(positive) {}
};

// 技能结构
struct CharacterSkill {
    std::string id;
    std::string name;
    std::string description;
    int level = 1;
    int maxLevel = 10;
    int manaCost = 0;
    int staminaCost = 0;
    float cooldown = 0.0f;
    float currentCooldown = 0.0f;
    float range = 1.0f;
    float damage = 0.0f;
    bool isActive = true;
    
    CharacterSkill() = default;
    CharacterSkill(const std::string& id, const std::string& name, int manaCost, float cooldown)
        : id(id), name(name), manaCost(manaCost), cooldown(cooldown), currentCooldown(0.0f) {}
};

// 角色事件类型
enum class CharacterEventType {
    CREATED,
    DESTROYED,
    MOVED,
    ATTACKED,
    DAMAGED,
    HEALED,
    LEVELED_UP,
    EXPERIENCE_GAINED,
    SKILL_USED,
    STATUS_EFFECT_ADDED,
    STATUS_EFFECT_REMOVED,
    EQUIPMENT_CHANGED,
    DIED,
    REVIVED
};

// 角色事件数据
using CharacterEventData = std::variant<
    std::string,                    // 字符串数据
    int,                           // 整数数据
    float,                         // 浮点数数据
    Vector2D,                      // 位置数据
    std::vector<std::string>       // 字符串数组
>;

// 角色事件结构
struct CharacterEvent {
    CharacterEventType type;
    std::string name;
    CharacterEventData data;
    std::chrono::system_clock::time_point timestamp;
    
    CharacterEvent(CharacterEventType t, const std::string& n, const CharacterEventData& d = std::string{})
        : type(t), name(n), data(d), timestamp(std::chrono::system_clock::now()) {}
};

// 角色事件回调函数类型
using CharacterEventCallback = std::function<void(const CharacterEvent&)>;

/**
 * @brief 角色基类
 * 
 * 提供角色的基础功能：
 * - 属性管理
 * - 移动系统
 * - 战斗系统
 * - 技能系统
 * - 装备系统
 * - 状态效果
 */
class Character {
public:
    // 构造和析构
    Character(const std::string& name, CharacterClass classType = CharacterClass::WARRIOR);
    virtual ~Character();
    
    // 禁用拷贝
    Character(const Character&) = delete;
    Character& operator=(const Character&) = delete;
    
    // 基础操作
    virtual void update(double deltaTime);
    virtual void move(const Vector2D& direction);
    virtual void moveTo(const Vector2D& target);
    virtual void stop();
    
    // 战斗操作
    virtual void attack(std::shared_ptr<Character> target);
    virtual void takeDamage(int damage, const std::string& damageType = "physical");
    virtual void heal(int amount);
    virtual void useSkill(const std::string& skillId, std::shared_ptr<Character> target = nullptr);
    virtual void useSkill(int skillIndex, std::shared_ptr<Character> target = nullptr);
    
    // 属性操作
    virtual void gainExperience(int exp);
    virtual void levelUp();
    virtual void calculateStats();
    virtual int calculateRequiredExperience(int level) const;
    
    // 技能管理
    virtual bool addSkill(const CharacterSkill& skill);
    virtual bool removeSkill(const std::string& skillId);
    virtual bool upgradeSkill(const std::string& skillId);
    virtual void updateSkillCooldowns(double deltaTime);
    virtual bool canUseSkill(const std::string& skillId) const;
    
    // 状态效果管理
    virtual void addStatusEffect(const StatusEffect& effect);
    virtual void removeStatusEffect(const std::string& effectId);
    virtual void updateStatusEffects(double deltaTime);
    virtual bool hasStatusEffect(const std::string& effectId) const;
    virtual bool hasStatusEffect(CharacterState state) const;
    
    // 装备管理
    virtual bool equipItem(const std::string& itemId, EquipmentSlot slot);
    virtual bool unequipItem(EquipmentSlot slot);
    virtual std::string getEquippedItem(EquipmentSlot slot) const;
    virtual void updateEquipmentStats();
    
    // 事件系统
    virtual void emitEvent(const CharacterEvent& event);
    virtual void emitEvent(CharacterEventType type, const std::string& name, const CharacterEventData& data = std::string{});
    virtual void subscribeToEvent(CharacterEventType type, CharacterEventCallback callback);
    virtual void unsubscribeFromEvent(CharacterEventType type, CharacterEventCallback callback);
    
    // 获取器
    const std::string& getName() const { return name_; }
    CharacterClass getClass() const { return class_; }
    CharacterState getState() const { return state_; }
    const CharacterStats& getStats() const { return stats_; }
    const Vector2D& getPosition() const { return position_; }
    const Vector2D& getVelocity() const { return velocity_; }
    bool isAlive() const { return stats_.health > 0; }
    bool isMoving() const { return state_ == CharacterState::MOVING; }
    bool isAttacking() const { return state_ == CharacterState::ATTACKING; }
    bool isCasting() const { return state_ == CharacterState::CASTING; }
    
    // 设置器
    virtual void setName(const std::string& name) { name_ = name; }
    virtual void setPosition(const Vector2D& position) { position_ = position; }
    virtual void setVelocity(const Vector2D& velocity) { velocity_ = velocity; }
    virtual void setState(CharacterState state) { state_ = state; }
    virtual void setStats(const CharacterStats& stats) { stats_ = stats; }
    
    // 工具方法
    virtual float getDistanceTo(const std::shared_ptr<Character>& other) const;
    virtual bool isInRange(const std::shared_ptr<Character>& other, float range) const;
    virtual Vector2D getDirectionTo(const std::shared_ptr<Character>& other) const;
    virtual std::string getClassString() const;
    virtual std::string getStateString() const;

protected:
    // 内部方法
    virtual void onCreated();
    virtual void onDestroyed();
    virtual void onMoved();
    virtual void onAttacked(std::shared_ptr<Character> target);
    virtual void onDamaged(int damage, const std::string& damageType);
    virtual void onHealed(int amount);
    virtual void onLeveledUp();
    virtual void onExperienceGained(int amount);
    virtual void onSkillUsed(const std::string& skillId);
    virtual void onStatusEffectAdded(const StatusEffect& effect);
    virtual void onStatusEffectRemoved(const std::string& effectId);
    virtual void onEquipmentChanged(EquipmentSlot slot, const std::string& itemId);
    virtual void onDied();
    virtual void onRevived();

private:
    // 基础属性
    std::string name_;
    CharacterClass class_;
    CharacterState state_;
    CharacterStats stats_;
    
    // 位置和移动
    Vector2D position_;
    Vector2D velocity_;
    Vector2D targetPosition_;
    float moveSpeed_;
    bool isMovingToTarget_;
    
    // 技能系统
    std::vector<CharacterSkill> skills_;
    std::unordered_map<std::string, int> skillIndices_;
    
    // 状态效果
    std::vector<StatusEffect> statusEffects_;
    std::unordered_map<std::string, int> statusEffectIndices_;
    
    // 装备系统
    std::unordered_map<EquipmentSlot, std::string> equippedItems_;
    
    // 事件系统
    std::unordered_map<CharacterEventType, std::vector<CharacterEventCallback>> eventCallbacks_;
    std::vector<CharacterEvent> eventQueue_;
    
    // 内部状态
    bool initialized_;
    std::chrono::system_clock::time_point lastUpdateTime_;
    
    // 私有方法
    void initializeStats();
    void updateMovement(double deltaTime);
    void updateAnimation(double deltaTime);
    void processEvents();
    void applyStatusEffects();
    void removeExpiredStatusEffects();
};

// 角色日志宏
#define CHAR_LOG_TRACE(msg, ...) \
    FANTASY_LOG_TRACE("[Character] " msg, ##__VA_ARGS__)

#define CHAR_LOG_DEBUG(msg, ...) \
    FANTASY_LOG_DEBUG("[Character] " msg, ##__VA_ARGS__)

#define CHAR_LOG_INFO(msg, ...) \
    FANTASY_LOG_INFO("[Character] " msg, ##__VA_ARGS__)

#define CHAR_LOG_WARN(msg, ...) \
    FANTASY_LOG_WARN("[Character] " msg, ##__VA_ARGS__)

#define CHAR_LOG_ERROR(msg, ...) \
    FANTASY_LOG_ERROR("[Character] " msg, ##__VA_ARGS__)

} // namespace Fantasy