/**
 * @file Skill.h
 * @brief 技能系统 - 游戏技能管理
 * @author [pengchengkang]
 * @date 2025.06.17
 * 
 * 功能特性:
 * - 技能类型管理
 * - 技能效果系统
 * - 技能升级系统
 * - 技能冷却管理
 * - 技能组合系统
 * - 技能学习系统
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <functional>
#include <variant>

namespace Fantasy {

// 前向声明
class Character;

// 技能类型枚举
enum class SkillType {
    ACTIVE,         // 主动技能
    PASSIVE,        // 被动技能
    ULTIMATE,       // 终极技能
    BUFF,           // 增益技能
    DEBUFF,         // 减益技能
    HEAL,           // 治疗技能
    DAMAGE,         // 伤害技能
    UTILITY,        // 功能技能
    SUMMON,         // 召唤技能
    TRANSFORM       // 变形技能
};

// 技能元素类型枚举
enum class SkillElement {
    NONE,           // 无属性
    FIRE,           // 火
    ICE,            // 冰
    LIGHTNING,      // 雷
    EARTH,          // 土
    WIND,           // 风
    WATER,          // 水
    LIGHT,          // 光
    DARK,           // 暗
    PHYSICAL        // 物理
};

// 技能目标类型枚举
enum class SkillTargetType {
    SELF,           // 自身
    SINGLE_ENEMY,   // 单个敌人
    SINGLE_ALLY,    // 单个盟友
    ALL_ENEMIES,    // 所有敌人
    ALL_ALLIES,     // 所有盟友
    AREA,           // 区域
    LINE,           // 直线
    CONE,           // 扇形
    RANDOM,         // 随机
    NONE            // 无目标
};

// 技能效果类型枚举
enum class SkillEffectType {
    DAMAGE,         // 伤害
    HEAL,           // 治疗
    BUFF,           // 增益
    DEBUFF,         // 减益
    STATUS,         // 状态
    TELEPORT,       // 传送
    SUMMON,         // 召唤
    TRANSFORM,      // 变形
    DISPEL,         // 驱散
    REFLECT         // 反射
};

// 技能效果结构
struct SkillEffect {
    std::string id;
    SkillEffectType type;
    std::string targetStat;
    float baseValue;
    float scalingValue;  // 基于技能等级的缩放值
    int duration;        // 持续时间（秒），0表示瞬间
    bool isPercentage;
    std::string description;
    std::vector<std::string> conditions;
    
    SkillEffect() : type(SkillEffectType::DAMAGE), baseValue(0.0f), scalingValue(0.0f), 
                   duration(0), isPercentage(false) {}
    SkillEffect(SkillEffectType t, const std::string& stat, float base, float scaling = 0.0f, int dur = 0)
        : type(t), targetStat(stat), baseValue(base), scalingValue(scaling), duration(dur), isPercentage(false) {}
};

// 技能需求结构
struct SkillRequirement {
    std::string type;  // "level", "skill", "item", "attribute", "class"
    std::string targetId;
    int minValue;
    std::string operator_;  // ">=", "==", "<=", "!="
    
    SkillRequirement() : minValue(0), operator_(">=") {}
    SkillRequirement(const std::string& type, const std::string& target, int value, const std::string& op = ">=")
        : type(type), targetId(target), minValue(value), operator_(op) {}
};

// 技能升级结构
struct SkillUpgrade {
    int level;
    std::vector<SkillEffect> effects;
    int manaCost;
    int staminaCost;
    float cooldown;
    float range;
    float area;
    std::string description;
    
    SkillUpgrade() : level(1), manaCost(0), staminaCost(0), cooldown(0.0f), range(1.0f), area(0.0f) {}
};

// 技能事件类型
enum class SkillEventType {
    SKILL_LEARNED,
    SKILL_FORGOTTEN,
    SKILL_UPGRADED,
    SKILL_USED,
    SKILL_ACTIVATED,
    SKILL_DEACTIVATED,
    SKILL_COOLDOWN_STARTED,
    SKILL_COOLDOWN_ENDED,
    SKILL_EFFECT_APPLIED,
    SKILL_EFFECT_REMOVED
};

// 技能事件数据
using SkillEventData = std::unordered_map<std::string, std::string>;

// 技能事件结构
struct SkillEvent {
    SkillEventType type;
    std::string name;
    SkillEventData data;
    std::chrono::system_clock::time_point timestamp;
    
    SkillEvent(SkillEventType t, const std::string& n, const SkillEventData& d = SkillEventData{})
        : type(t), name(n), data(d), timestamp(std::chrono::system_clock::now()) {}
};

// 事件回调函数类型
using SkillEventCallback = std::function<void(const SkillEvent&)>;

// 技能配置结构
struct SkillConfig {
    std::string id;
    std::string name;
    std::string description;
    SkillType type;
    SkillElement element;
    SkillTargetType targetType;
    int level;
    int maxLevel;
    std::vector<SkillRequirement> requirements;
    std::vector<SkillEffect> effects;
    std::vector<SkillUpgrade> upgrades;
    std::vector<std::string> prerequisites;
    std::vector<std::string> combinations;
    int manaCost;
    int staminaCost;
    float cooldown;
    float castTime;
    float range;
    float area;
    std::string icon;
    std::string animation;
    bool learnable;
    bool forgettable;
    bool upgradable;
    
    SkillConfig() : type(SkillType::ACTIVE), element(SkillElement::NONE), targetType(SkillTargetType::SELF),
                   level(1), maxLevel(10), manaCost(0), staminaCost(0), cooldown(0.0f), castTime(0.0f),
                   range(1.0f), area(0.0f), learnable(true), forgettable(true), upgradable(true) {}
};

// 技能统计结构
struct SkillStats {
    std::chrono::system_clock::time_point creationTime;
    std::chrono::system_clock::time_point lastLearnTime;
    std::chrono::system_clock::time_point lastForgetTime;
    std::chrono::system_clock::time_point lastUpgradeTime;
    std::chrono::system_clock::time_point lastUsedTime;
    int totalLearns;
    int totalForgets;
    int totalUpgrades;
    int totalUses;
    int totalEvents;
    
    SkillStats() : totalLearns(0), totalForgets(0), totalUpgrades(0), totalUses(0), totalEvents(0) {}
};

// 日志宏定义
#define SKILL_LOG_DEBUG(fmt, ...) 
#define SKILL_LOG_INFO(fmt, ...) 
#define SKILL_LOG_WARN(fmt, ...) 
#define SKILL_LOG_ERROR(fmt, ...) 

/**
 * @brief 技能类
 * 
 * 提供技能的基础功能：
 * - 技能配置管理
 * - 效果计算
 * - 冷却管理
 * - 升级系统
 * - 事件系统
 */
class Skill {
public:
    // 构造和析构
    Skill(const std::string& id, const std::string& name, SkillType type, const SkillConfig& config);
    virtual ~Skill();
    
    // 禁用拷贝
    Skill(const Skill&) = delete;
    Skill& operator=(const Skill&) = delete;
    
    // 技能生命周期
    virtual bool learn(std::shared_ptr<Character> character);
    virtual bool forget();
    virtual bool upgrade();
    virtual bool use(std::shared_ptr<Character> caster, const std::vector<std::shared_ptr<Character>>& targets);
    virtual void update(double deltaTime);
    
    // 效果管理
    virtual std::vector<SkillEffect> getEffects() const;
    virtual std::vector<SkillEffect> getEffectsForLevel(int level) const;
    virtual float calculateEffectValue(const SkillEffect& effect, int level) const;
    virtual bool hasEffect(const std::string& effectId) const;
    
    // 需求检查
    virtual bool canLearn(std::shared_ptr<Character> character) const;
    virtual bool canUse(std::shared_ptr<Character> caster) const;
    virtual bool meetsRequirements() const;
    virtual bool checkLevelRequirement(int level) const;
    virtual bool checkSkillRequirement(const std::string& skillId, int level) const;
    virtual bool checkAttributeRequirement(const std::string& attribute, int value) const;
    
    // 冷却管理
    virtual void startCooldown();
    virtual void resetCooldown();
    virtual bool isOnCooldown() const { return cooldownRemaining_ > 0; }
    virtual float getCooldownRemaining() const { return cooldownRemaining_; }
    virtual float getCooldownProgress() const;
    
    // 施法管理
    virtual void startCast();
    virtual void interruptCast();
    virtual bool isCasting() const { return castTimeRemaining_ > 0; }
    virtual float getCastProgress() const;
    
    // 事件系统
    virtual void emitEvent(const SkillEvent& event);
    virtual void emitEvent(SkillEventType type, const std::string& name, 
                          const SkillEventData& data = SkillEventData{});
    virtual void subscribeToEvent(SkillEventType type, SkillEventCallback callback);
    virtual void unsubscribeFromEvent(SkillEventType type, SkillEventCallback callback);
    
    // 状态检查
    virtual bool isLearned() const { return learned_; }
    virtual bool isActive() const { return active_; }
    virtual bool canLearn() const;
    virtual bool canForget() const;
    virtual bool canUpgrade() const;
    virtual bool canUse() const;
    virtual bool isMaxLevel() const { return level_ >= maxLevel_; }
    
    // 获取器
    const std::string& getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getDescription() const { return description_; }
    SkillType getType() const { return type_; }
    SkillElement getElement() const { return element_; }
    SkillTargetType getTargetType() const { return targetType_; }
    int getLevel() const { return level_; }
    int getMaxLevel() const { return maxLevel_; }
    int getManaCost() const;
    int getStaminaCost() const;
    float getCooldown() const;
    float getCastTime() const;
    float getRange() const;
    float getArea() const;
    int getUseCount() const { return useCount_; }
    
    // 设置器
    virtual void setLevel(int level);
    virtual void setLearned(bool learned) { learned_ = learned; }
    virtual void setActive(bool active) { active_ = active; }
    virtual void setState(const SkillStats& stats) { stats_ = stats; }

    // 生命周期事件
    virtual void onLearned();
    virtual void onForgotten();
    virtual void onUpgraded();
    virtual void onUsed();
    virtual void onActivated();
    virtual void onDeactivated();
    virtual void onCooldownStarted();
    virtual void onCooldownEnded();
    virtual void onEffectApplied(const std::string& effectId);
    virtual void onEffectRemoved(const std::string& effectId);

protected:
    // 私有方法
    void initializeSkill();
    void processEvents();
    void updateTimers(double deltaTime);
    void updateCooldown(double deltaTime);
    void updateCastTime(double deltaTime);
    const SkillUpgrade* findUpgrade(int level) const;
    void calculateSkillValues();
    bool validateRequirements() const;
    void applyLevelEffects();
    void removeLevelEffects();
    bool consumeResources(std::shared_ptr<Character> caster);
    bool applyEffects(std::shared_ptr<Character> caster, const std::vector<std::shared_ptr<Character>>& targets);
    bool applyEffect(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                    const std::vector<std::shared_ptr<Character>>& targets);
    int calculateDamage(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                       std::shared_ptr<Character> target);
    int calculateHealAmount(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                           std::shared_ptr<Character> target);
    bool applyDamageEffect(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                          const std::vector<std::shared_ptr<Character>>& targets);
    bool applyHealEffect(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                        const std::vector<std::shared_ptr<Character>>& targets);
    bool applyBuffEffect(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                        const std::vector<std::shared_ptr<Character>>& targets);
    bool applyDebuffEffect(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                          const std::vector<std::shared_ptr<Character>>& targets);
    bool applyTeleportEffect(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                            const std::vector<std::shared_ptr<Character>>& targets);
    bool applySummonEffect(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                          const std::vector<std::shared_ptr<Character>>& targets);
    bool applyTransformEffect(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                             const std::vector<std::shared_ptr<Character>>& targets);
    void initSkillEffects();
    void removeSkillEffects();
    void applySkillEffects();

private:
    // 基础属性
    std::string id_;
    std::string name_;
    std::string description_;
    SkillType type_;
    SkillElement element_;
    SkillTargetType targetType_;
    int level_;
    int maxLevel_;
    bool learned_;
    bool active_;
    float cooldownRemaining_;
    float castTimeRemaining_;
    int useCount_;
    
    // 配置
    SkillConfig config_;
    std::vector<SkillEffect> effects_;
    std::vector<SkillRequirement> requirements_;
    std::vector<SkillUpgrade> upgrades_;
    SkillStats stats_;
    
    // 时间管理
    std::chrono::system_clock::time_point lastUsedTime_;
    float cooldown_;
    float castTime_;
    
    // 关联对象
    std::shared_ptr<Character> character_;
    
    // 事件系统
    std::vector<SkillEvent> eventQueue_;
    std::unordered_map<SkillEventType, std::vector<SkillEventCallback>> eventCallbacks_;
};

} // namespace Fantasy 