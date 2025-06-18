/**
 * @file Skill.cpp
 * @brief 技能系统实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "include/core/skills/Skill.h"
#include "include/core/characters/Character.h"
#include <iostream>
#include <algorithm>
#include <chrono>

namespace Fantasy {

// 构造函数
Skill::Skill(const std::string& id, const std::string& name, SkillType type, const SkillConfig& config)
    : id_(id), name_(name), description_(config.description), type_(type), element_(config.element),
      targetType_(config.targetType), level_(config.level), maxLevel_(config.maxLevel),
      learned_(false), active_(false), cooldownRemaining_(0.0f), castTimeRemaining_(0.0f),
      useCount_(0), config_(config), cooldown_(config.cooldown), castTime_(config.castTime),
      character_(nullptr) {
    
    SKILL_LOG_INFO("创建技能: %s (%s)", name_.c_str(), id_.c_str());
    
    // 初始化效果
    effects_ = config.effects;
    
    // 初始化需求
    requirements_ = config.requirements;
    
    // 初始化升级
    upgrades_ = config.upgrades;
    
    // 初始化统计
    stats_.creationTime = std::chrono::system_clock::now();
    
    initializeSkill();
}

// 析构函数
Skill::~Skill() {
    SKILL_LOG_INFO("销毁技能: %s", name_.c_str());
}

// 学习技能
bool Skill::learn(std::shared_ptr<Character> character) {
    if (learned_) {
        SKILL_LOG_WARN("技能 %s 已经学习过了", name_.c_str());
        return false;
    }
    
    if (!canLearn(character)) {
        SKILL_LOG_WARN("角色不满足学习技能 %s 的条件", name_.c_str());
        return false;
    }
    
    character_ = character;
    learned_ = true;
    stats_.lastLearnTime = std::chrono::system_clock::now();
    stats_.totalLearns++;
    
    SKILL_LOG_INFO("技能 %s 学习成功", name_.c_str());
    
    // 发送事件
    emitEvent(SkillEventType::SKILL_LEARNED, "SkillLearned");
    onLearned();
    
    return true;
}

// 遗忘技能
bool Skill::forget() {
    if (!learned_) {
        SKILL_LOG_WARN("技能 %s 未学习，无法遗忘", name_.c_str());
        return false;
    }
    
    if (!config_.forgettable) {
        SKILL_LOG_WARN("技能 %s 不可遗忘", name_.c_str());
        return false;
    }
    
    learned_ = false;
    active_ = false;
    stats_.lastForgetTime = std::chrono::system_clock::now();
    stats_.totalForgets++;
    
    SKILL_LOG_INFO("技能 %s 遗忘成功", name_.c_str());
    
    // 发送事件
    emitEvent(SkillEventType::SKILL_FORGOTTEN, "SkillForgotten");
    onForgotten();
    
    return true;
}

// 升级技能
bool Skill::upgrade() {
    if (!learned_) {
        SKILL_LOG_WARN("技能 %s 未学习，无法升级", name_.c_str());
        return false;
    }
    
    if (level_ >= maxLevel_) {
        SKILL_LOG_WARN("技能 %s 已达到最高等级", name_.c_str());
        return false;
    }
    
    if (!config_.upgradable) {
        SKILL_LOG_WARN("技能 %s 不可升级", name_.c_str());
        return false;
    }
    
    level_++;
    stats_.lastUpgradeTime = std::chrono::system_clock::now();
    stats_.totalUpgrades++;
    
    SKILL_LOG_INFO("技能 %s 升级到 %d 级", name_.c_str(), level_);
    emitEvent(SkillEventType::SKILL_UPGRADED, "SkillUpgraded");
    onUpgraded();
    
    return true;
}

// 使用技能
bool Skill::use(std::shared_ptr<Character> caster, const std::vector<std::shared_ptr<Character>>& targets) {
    if (!learned_) {
        SKILL_LOG_WARN("技能 %s 未学习，无法使用", name_.c_str());
        return false;
    }
    
    if (!canUse(caster)) {
        SKILL_LOG_WARN("技能 %s 无法使用", name_.c_str());
        return false;
    }
    
    if (isOnCooldown()) {
        SKILL_LOG_WARN("技能 %s 正在冷却中", name_.c_str());
        return false;
    }
    
    if (isCasting()) {
        SKILL_LOG_WARN("技能 %s 正在施法中", name_.c_str());
        return false;
    }
    
    // 消耗资源
    if (!consumeResources(caster)) {
        SKILL_LOG_WARN("技能 %s 资源不足", name_.c_str());
        return false;
    }
    
    // 应用效果
    bool success = applyEffects(caster, targets);
    
    if (success) {
        useCount_++;
        stats_.lastUsedTime = std::chrono::system_clock::now();
        stats_.totalUses++;
        
        // 开始冷却
        startCooldown();
        
        // 发送事件
        emitEvent(SkillEventType::SKILL_USED, "SkillUsed");
        onUsed();
    }
    
    return success;
}

// 更新技能
void Skill::update(double deltaTime) {
    if (!learned_) {
        return;
    }
    
    // 更新计时器
    updateTimers(deltaTime);
    
    // 处理事件
    processEvents();
    
    // 更新统计
    stats_.totalEvents++;
}

// 获取效果
std::vector<SkillEffect> Skill::getEffects() const {
    return effects_;
}

// 获取指定等级的效果
std::vector<SkillEffect> Skill::getEffectsForLevel(int level) const {
    std::vector<SkillEffect> levelEffects;
    
    for (const auto& effect : effects_) {
        SkillEffect levelEffect = effect;
        levelEffect.baseValue = calculateEffectValue(effect, level);
        levelEffects.push_back(levelEffect);
    }
    
    return levelEffects;
}

// 计算效果值
float Skill::calculateEffectValue(const SkillEffect& effect, int level) const {
    return effect.baseValue + (effect.scalingValue * (level - 1));
}

// 检查是否可以学习
bool Skill::canLearn(std::shared_ptr<Character> character) const {
    if (!config_.learnable) {
        return false;
    }
    
    if (learned_) {
        return false;
    }
    
    // 检查等级需求
    if (!checkLevelRequirement(character->getStats().level)) {
        return false;
    }
    
    // 检查其他需求
    for (const auto& req : requirements_) {
        if (req.type == "skill") {
            if (!checkSkillRequirement(req.targetId, req.minValue)) {
                return false;
            }
        } else if (req.type == "attribute") {
            if (!checkAttributeRequirement(req.targetId, req.minValue)) {
                return false;
            }
        }
    }
    
    return true;
}

// 检查是否可以使用
bool Skill::canUse(std::shared_ptr<Character> caster) const {
    if (!learned_) {
        return false;
    }
    
    if (isOnCooldown()) {
        return false;
    }
    
    if (isCasting()) {
        return false;
    }
    
    // 检查资源
    if (caster->getStats().mana < getManaCost()) {
        return false;
    }
    
    if (caster->getStats().stamina < getStaminaCost()) {
        return false;
    }
    
    return true;
}

// 检查等级需求
bool Skill::checkLevelRequirement(int level) const {
    for (const auto& req : requirements_) {
        if (req.type == "level") {
            if (req.operator_ == ">=") {
                return level >= req.minValue;
            } else if (req.operator_ == "==") {
                return level == req.minValue;
            } else if (req.operator_ == "<=") {
                return level <= req.minValue;
            }
        }
    }
    return true;
}

// 检查技能需求
bool Skill::checkSkillRequirement(const std::string& skillId, int level) const {
    // 这里需要检查角色是否拥有指定技能和等级
    // 暂时返回true，实际实现需要访问角色技能列表
    return true;
}

// 检查属性需求
bool Skill::checkAttributeRequirement(const std::string& attribute, int value) const {
    // 这里需要检查角色属性
    // 暂时返回true，实际实现需要访问角色属性
    return true;
}

// 开始冷却
void Skill::startCooldown() {
    cooldownRemaining_ = cooldown_;
    emitEvent(SkillEventType::SKILL_COOLDOWN_STARTED, "CooldownStarted");
    onCooldownStarted();
}

// 重置冷却
void Skill::resetCooldown() {
    cooldownRemaining_ = 0.0f;
    emitEvent(SkillEventType::SKILL_COOLDOWN_ENDED, "CooldownEnded");
    onCooldownEnded();
}

// 获取冷却进度
float Skill::getCooldownProgress() const {
    if (cooldown_ <= 0) return 1.0f;
    return 1.0f - (cooldownRemaining_ / cooldown_);
}

// 开始施法
void Skill::startCast() {
    castTimeRemaining_ = castTime_;
}

// 中断施法
void Skill::interruptCast() {
    castTimeRemaining_ = 0.0f;
}

// 获取施法进度
float Skill::getCastProgress() const {
    if (castTime_ <= 0) return 1.0f;
    return 1.0f - (castTimeRemaining_ / castTime_);
}

// 发送事件
void Skill::emitEvent(const SkillEvent& event) {
    eventQueue_.push_back(event);
    stats_.totalEvents++;
}

// 发送事件
void Skill::emitEvent(SkillEventType type, const std::string& name, const SkillEventData& data) {
    SkillEvent event(type, name, data);
    emitEvent(event);
}

// 订阅事件
void Skill::subscribeToEvent(SkillEventType type, EventCallback callback) {
    eventCallbacks_[type].push_back(callback);
}

// 取消订阅事件
void Skill::unsubscribeFromEvent(SkillEventType type, EventCallback callback) {
    auto& callbacks = eventCallbacks_[type];
    callbacks.erase(
        std::remove_if(callbacks.begin(), callbacks.end(),
            [&callback](const EventCallback& cb) {
                // 由于std::function没有operator==，我们使用target_type()和target()来比较
                return cb.target_type() == callback.target_type() && 
                       cb.target<void(*)(const SkillEvent&)>() == callback.target<void(*)(const SkillEvent&)>();
            }),
        callbacks.end()
    );
}

// 检查是否可以学习
bool Skill::canLearn() const {
    return config_.learnable && !learned_;
}

// 检查是否可以遗忘
bool Skill::canForget() const {
    return config_.forgettable && learned_;
}

// 检查是否可以升级
bool Skill::canUpgrade() const {
    return config_.upgradable && learned_ && level_ < maxLevel_;
}

// 检查是否可以使用
bool Skill::canUse() const {
    return learned_ && !isOnCooldown() && !isCasting();
}

// 获取魔法消耗
int Skill::getManaCost() const {
    const SkillUpgrade* upgrade = findUpgrade(level_);
    return upgrade ? upgrade->manaCost : config_.manaCost;
}

// 获取体力消耗
int Skill::getStaminaCost() const {
    const SkillUpgrade* upgrade = findUpgrade(level_);
    return upgrade ? upgrade->staminaCost : config_.staminaCost;
}

// 获取冷却时间
float Skill::getCooldown() const {
    const SkillUpgrade* upgrade = findUpgrade(level_);
    return upgrade ? upgrade->cooldown : config_.cooldown;
}

// 获取施法时间
float Skill::getCastTime() const {
    return config_.castTime;
}

// 获取范围
float Skill::getRange() const {
    const SkillUpgrade* upgrade = findUpgrade(level_);
    return upgrade ? upgrade->range : config_.range;
}

// 获取区域
float Skill::getArea() const {
    const SkillUpgrade* upgrade = findUpgrade(level_);
    return upgrade ? upgrade->area : config_.area;
}

// 设置等级
void Skill::setLevel(int level) {
    if (level >= 1 && level <= maxLevel_) {
        level_ = level;
        calculateSkillValues();
    }
}

// 事件回调方法
void Skill::onLearned() {
    SKILL_LOG_INFO("技能 %s 学习回调", name_.c_str());
}

void Skill::onForgotten() {
    SKILL_LOG_INFO("技能 %s 遗忘回调", name_.c_str());
}

void Skill::onUpgraded() {
    SKILL_LOG_INFO("技能 %s 升级回调", name_.c_str());
}

void Skill::onUsed() {
    SKILL_LOG_INFO("技能 %s 使用回调", name_.c_str());
}

void Skill::onActivated() {
    SKILL_LOG_INFO("技能 %s 激活回调", name_.c_str());
}

void Skill::onDeactivated() {
    SKILL_LOG_INFO("技能 %s 停用回调", name_.c_str());
}

void Skill::onCooldownStarted() {
    SKILL_LOG_INFO("技能 %s 冷却开始回调", name_.c_str());
}

void Skill::onCooldownEnded() {
    SKILL_LOG_INFO("技能 %s 冷却结束回调", name_.c_str());
}

void Skill::onEffectApplied(const std::string& effectId) {
    SKILL_LOG_INFO("技能 %s 效果应用回调: %s", name_.c_str(), effectId.c_str());
}

void Skill::onEffectRemoved(const std::string& effectId) {
    SKILL_LOG_INFO("技能 %s 效果移除回调: %s", name_.c_str(), effectId.c_str());
}

// 私有方法实现
void Skill::initializeSkill() {
    SKILL_LOG_INFO("初始化技能: %s", name_.c_str());
    calculateSkillValues();
}

void Skill::processEvents() {
    while (!eventQueue_.empty()) {
        const SkillEvent& event = eventQueue_.front();
        
        auto it = eventCallbacks_.find(event.type);
        if (it != eventCallbacks_.end()) {
            for (const auto& callback : it->second) {
                callback(event);
            }
        }
        
        eventQueue_.erase(eventQueue_.begin());
    }
}

void Skill::updateTimers(double deltaTime) {
    updateCooldown(deltaTime);
    updateCastTime(deltaTime);
}

void Skill::updateCooldown(double deltaTime) {
    if (cooldownRemaining_ > 0) {
        cooldownRemaining_ -= deltaTime;
        if (cooldownRemaining_ <= 0) {
            cooldownRemaining_ = 0;
            emitEvent(SkillEventType::SKILL_COOLDOWN_ENDED, "CooldownEnded");
            onCooldownEnded();
        }
    }
}

void Skill::updateCastTime(double deltaTime) {
    if (castTimeRemaining_ > 0) {
        castTimeRemaining_ -= deltaTime;
        if (castTimeRemaining_ <= 0) {
            castTimeRemaining_ = 0;
        }
    }
}

const SkillUpgrade* Skill::findUpgrade(int level) const {
    for (const auto& upgrade : upgrades_) {
        if (upgrade.level == level) {
            return &upgrade;
        }
    }
    return nullptr;
}

void Skill::calculateSkillValues() {
    // 根据等级重新计算技能值
    SKILL_LOG_INFO("重新计算技能 %s 的值", name_.c_str());
}

bool Skill::validateRequirements() const {
    for (const auto& req : requirements_) {
        if (req.type == "level") {
            if (!checkLevelRequirement(level_)) {
                return false;
            }
        }
    }
    return true;
}

void Skill::applyLevelEffects() {
    SKILL_LOG_INFO("应用技能 %s 的等级效果", name_.c_str());
}

void Skill::removeLevelEffects() {
    SKILL_LOG_INFO("移除技能 %s 的等级效果", name_.c_str());
}

bool Skill::consumeResources(std::shared_ptr<Character> caster) {
    int manaCost = getManaCost();
    int staminaCost = getStaminaCost();
    
    if (caster->getStats().mana < manaCost) {
        return false;
    }
    
    if (caster->getStats().stamina < staminaCost) {
        return false;
    }
    
    // 这里应该调用角色的资源消耗方法
    // caster->consumeMana(manaCost);
    // caster->consumeStamina(staminaCost);
    
    return true;
}

bool Skill::applyEffects(std::shared_ptr<Character> caster, const std::vector<std::shared_ptr<Character>>& targets) {
    bool success = true;
    
    for (const auto& effect : effects_) {
        if (!applyEffect(effect, caster, targets)) {
            success = false;
        }
    }
    
    return success;
}

bool Skill::applyEffect(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                       const std::vector<std::shared_ptr<Character>>& targets) {
    switch (effect.type) {
        case SkillEffectType::DAMAGE:
            return applyDamageEffect(effect, caster, targets);
        case SkillEffectType::HEAL:
            return applyHealEffect(effect, caster, targets);
        case SkillEffectType::BUFF:
            return applyBuffEffect(effect, caster, targets);
        case SkillEffectType::DEBUFF:
            return applyDebuffEffect(effect, caster, targets);
        case SkillEffectType::TELEPORT:
            return applyTeleportEffect(effect, caster, targets);
        case SkillEffectType::SUMMON:
            return applySummonEffect(effect, caster, targets);
        case SkillEffectType::TRANSFORM:
            return applyTransformEffect(effect, caster, targets);
        default:
            SKILL_LOG_WARN("未知的效果类型: %d", static_cast<int>(effect.type));
            return false;
    }
}

int Skill::calculateDamage(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                          std::shared_ptr<Character> target) {
    float baseDamage = calculateEffectValue(effect, level_);
    // 这里应该包含更复杂的伤害计算逻辑
    return static_cast<int>(baseDamage);
}

int Skill::calculateHealAmount(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                              std::shared_ptr<Character> target) {
    float baseHeal = calculateEffectValue(effect, level_);
    // 这里应该包含更复杂的治疗计算逻辑
    return static_cast<int>(baseHeal);
}

bool Skill::applyDamageEffect(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                             const std::vector<std::shared_ptr<Character>>& targets) {
    for (const auto& target : targets) {
        int damage = calculateDamage(effect, caster, target);
        // 这里应该调用目标的受伤方法
        // target->takeDamage(damage);
        SKILL_LOG_INFO("对目标造成 %d 点伤害", damage);
    }
    return true;
}

bool Skill::applyHealEffect(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                           const std::vector<std::shared_ptr<Character>>& targets) {
    for (const auto& target : targets) {
        int heal = calculateHealAmount(effect, caster, target);
        // 这里应该调用目标的治疗方法
        // target->heal(heal);
        SKILL_LOG_INFO("治疗目标 %d 点生命值", heal);
    }
    return true;
}

bool Skill::applyBuffEffect(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                           const std::vector<std::shared_ptr<Character>>& targets) {
    for (const auto& target : targets) {
        // 这里应该应用增益效果
        SKILL_LOG_INFO("对目标应用增益效果: %s", effect.targetStat.c_str());
    }
    return true;
}

bool Skill::applyDebuffEffect(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                             const std::vector<std::shared_ptr<Character>>& targets) {
    for (const auto& target : targets) {
        // 这里应该应用减益效果
        SKILL_LOG_INFO("对目标应用减益效果: %s", effect.targetStat.c_str());
    }
    return true;
}

bool Skill::applyTeleportEffect(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                               const std::vector<std::shared_ptr<Character>>& targets) {
    // 这里应该实现传送效果
    SKILL_LOG_INFO("应用传送效果");
    return true;
}

bool Skill::applySummonEffect(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                             const std::vector<std::shared_ptr<Character>>& targets) {
    // 这里应该实现召唤效果
    SKILL_LOG_INFO("应用召唤效果");
    return true;
}

bool Skill::applyTransformEffect(const SkillEffect& effect, std::shared_ptr<Character> caster, 
                                const std::vector<std::shared_ptr<Character>>& targets) {
    // 这里应该实现变形效果
    SKILL_LOG_INFO("应用变形效果");
    return true;
}

void Skill::initSkillEffects() {
    SKILL_LOG_INFO("初始化技能 %s 的效果", name_.c_str());
}

void Skill::removeSkillEffects() {
    SKILL_LOG_INFO("移除技能 %s 的效果", name_.c_str());
}

void Skill::applySkillEffects() {
    SKILL_LOG_INFO("应用技能 %s 的效果", name_.c_str());
}

} // namespace Fantasy 