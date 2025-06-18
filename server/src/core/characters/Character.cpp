/**
 * @file Character.cpp
 * @brief 角色系统实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "include/core/characters/Character.h"
#include "include/utils/resources/ResourceLogger.h"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace Fantasy {

Character::Character(const std::string& name, CharacterClass classType)
    : name_(name)
    , class_(classType)
    , state_(CharacterState::IDLE)
    , position_(0.0f, 0.0f)
    , velocity_(0.0f, 0.0f)
    , targetPosition_(0.0f, 0.0f)
    , moveSpeed_(5.0f)
    , isMovingToTarget_(false)
    , initialized_(false)
    , lastUpdateTime_(std::chrono::system_clock::now())
{
    CHAR_LOG_INFO("Creating character: %s (Class: %s)", name.c_str(), getClassString().c_str());
    
    initializeStats();
    onCreated();
    initialized_ = true;
}

Character::~Character() {
    CHAR_LOG_INFO("Destroying character: %s", name_.c_str());
    onDestroyed();
}

void Character::update(double deltaTime) {
    if (!initialized_ || !isAlive()) {
        return;
    }
    
    // 更新移动
    updateMovement(deltaTime);
    
    // 更新技能冷却
    updateSkillCooldowns(deltaTime);
    
    // 更新状态效果
    updateStatusEffects(deltaTime);
    
    // 更新动画
    updateAnimation(deltaTime);
    
    // 处理事件
    processEvents();
    
    lastUpdateTime_ = std::chrono::system_clock::now();
}

void Character::move(const Vector2D& direction) {
    if (!isAlive() || state_ == CharacterState::STUNNED || state_ == CharacterState::FROZEN) {
        return;
    }
    
    Vector2D normalizedDir = direction.normalized();
    velocity_ = normalizedDir * moveSpeed_;
    state_ = CharacterState::MOVING;
    isMovingToTarget_ = false;
    
    CHAR_LOG_DEBUG("Character %s moving in direction (%.2f, %.2f)", 
                   name_.c_str(), normalizedDir.x, normalizedDir.y);
}

void Character::moveTo(const Vector2D& target) {
    if (!isAlive() || state_ == CharacterState::STUNNED || state_ == CharacterState::FROZEN) {
        return;
    }
    
    targetPosition_ = target;
    isMovingToTarget_ = true;
    state_ = CharacterState::MOVING;
    
    CHAR_LOG_DEBUG("Character %s moving to target (%.2f, %.2f)", 
                   name_.c_str(), target.x, target.y);
}

void Character::stop() {
    velocity_ = Vector2D(0.0f, 0.0f);
    state_ = CharacterState::IDLE;
    isMovingToTarget_ = false;
    
    CHAR_LOG_DEBUG("Character %s stopped", name_.c_str());
}

void Character::attack(std::shared_ptr<Character> target) {
    if (!isAlive() || !target || !target->isAlive()) {
        return;
    }
    
    if (state_ == CharacterState::STUNNED || state_ == CharacterState::FROZEN) {
        return;
    }
    
    // 检查攻击范围
    if (!isInRange(target, 2.0f)) {
        CHAR_LOG_DEBUG("Character %s cannot attack %s - out of range", 
                       name_.c_str(), target->getName().c_str());
        return;
    }
    
    state_ = CharacterState::ATTACKING;
    
    // 计算伤害
    int damage = stats_.attack;
    
    // 暴击检查
    if (rand() % 100 < stats_.criticalRate) {
        damage = static_cast<int>(damage * (stats_.criticalDamage / 100.0f));
        CHAR_LOG_DEBUG("Character %s critical hit on %s!", name_.c_str(), target->getName().c_str());
    }
    
    // 应用伤害
    target->takeDamage(damage, "physical");
    
    onAttacked(target);
    
    CHAR_LOG_INFO("Character %s attacked %s for %d damage", 
                  name_.c_str(), target->getName().c_str(), damage);
}

void Character::takeDamage(int damage, const std::string& damageType) {
    if (!isAlive()) {
        return;
    }
    
    // 计算实际伤害（考虑防御和抗性）
    int actualDamage = damage;
    
    if (damageType == "physical") {
        actualDamage = std::max(1, damage - stats_.defense);
    } else if (damageType == "magical") {
        actualDamage = std::max(1, damage - stats_.magicDefense);
    }
    // TODO: 实现元素抗性系统
    
    stats_.health = std::max(0, stats_.health - actualDamage);
    
    onDamaged(actualDamage, damageType);
    
    CHAR_LOG_INFO("Character %s took %d %s damage (health: %d/%d)", 
                  name_.c_str(), actualDamage, damageType.c_str(), 
                  stats_.health, stats_.maxHealth);
    
    if (!isAlive()) {
        onDied();
    }
}

void Character::heal(int amount) {
    if (!isAlive()) {
        return;
    }
    
    int oldHealth = stats_.health;
    stats_.health = std::min(stats_.maxHealth, stats_.health + amount);
    int actualHeal = stats_.health - oldHealth;
    
    onHealed(actualHeal);
    
    CHAR_LOG_INFO("Character %s healed for %d (health: %d/%d)", 
                  name_.c_str(), actualHeal, stats_.health, stats_.maxHealth);
}

void Character::useSkill(const std::string& skillId, std::shared_ptr<Character> target) {
    if (!canUseSkill(skillId)) {
        CHAR_LOG_WARN("Character %s cannot use skill %s", name_.c_str(), skillId.c_str());
        return;
    }
    
    auto it = skillIndices_.find(skillId);
    if (it == skillIndices_.end()) {
        CHAR_LOG_ERROR("Skill %s not found for character %s", skillId.c_str(), name_.c_str());
        return;
    }
    
    useSkill(it->second, target);
}

void Character::useSkill(int skillIndex, std::shared_ptr<Character> target) {
    if (skillIndex < 0 || skillIndex >= static_cast<int>(skills_.size())) {
        CHAR_LOG_ERROR("Invalid skill index %d for character %s", skillIndex, name_.c_str());
        return;
    }
    
    CharacterSkill& skill = skills_[skillIndex];
    
    if (!skill.isActive || skill.currentCooldown > 0) {
        CHAR_LOG_WARN("Skill %s is not available for character %s", skill.name.c_str(), name_.c_str());
        return;
    }
    
    if (stats_.mana < skill.manaCost) {
        CHAR_LOG_WARN("Character %s has insufficient mana for skill %s", name_.c_str(), skill.name.c_str());
        return;
    }
    
    if (stats_.stamina < skill.staminaCost) {
        CHAR_LOG_WARN("Character %s has insufficient stamina for skill %s", name_.c_str(), skill.name.c_str());
        return;
    }
    
    // 消耗资源
    stats_.mana -= skill.manaCost;
    stats_.stamina -= skill.staminaCost;
    
    // 设置冷却
    skill.currentCooldown = skill.cooldown;
    
    // 设置状态
    state_ = CharacterState::CASTING;
    
    // 应用技能效果
    if (target && target->isAlive()) {
        if (skill.damage > 0) {
            target->takeDamage(static_cast<int>(skill.damage), "magical");
        }
    }
    
    onSkillUsed(skill.id);
    
    CHAR_LOG_INFO("Character %s used skill %s", name_.c_str(), skill.name.c_str());
}

void Character::gainExperience(int exp) {
    if (!isAlive()) {
        return;
    }
    
    stats_.experience += exp;
    onExperienceGained(exp);
    
    CHAR_LOG_DEBUG("Character %s gained %d experience (total: %d)", 
                   name_.c_str(), exp, stats_.experience);
    
    // 检查是否可以升级
    while (stats_.experience >= stats_.experienceToNext) {
        levelUp();
    }
}

void Character::levelUp() {
    stats_.level++;
    stats_.experience -= stats_.experienceToNext;
    stats_.experienceToNext = calculateRequiredExperience(stats_.level);
    
    // 提升属性
    calculateStats();
    
    onLeveledUp();
    
    CHAR_LOG_INFO("Character %s leveled up to level %d!", name_.c_str(), stats_.level);
}

void Character::calculateStats() {
    // 基础属性提升
    int levelBonus = (stats_.level - 1) * 5;
    
    stats_.maxHealth = 100 + levelBonus * 10;
    stats_.maxMana = 50 + levelBonus * 5;
    stats_.maxStamina = 100 + levelBonus * 3;
    
    stats_.attack = 15 + levelBonus * 2;
    stats_.defense = 10 + levelBonus;
    stats_.magicAttack = 10 + levelBonus * 2;
    stats_.magicDefense = 8 + levelBonus;
    stats_.speed = 5 + levelBonus / 2;
    
    // 确保当前值不超过最大值
    stats_.health = std::min(stats_.health, stats_.maxHealth);
    stats_.mana = std::min(stats_.mana, stats_.maxMana);
    stats_.stamina = std::min(stats_.stamina, stats_.maxStamina);
}

int Character::calculateRequiredExperience(int level) const {
    // 简单的经验公式：每级需要100 * level的经验
    return level * 100;
}

bool Character::addSkill(const CharacterSkill& skill) {
    if (skillIndices_.find(skill.id) != skillIndices_.end()) {
        CHAR_LOG_WARN("Skill %s already exists for character %s", skill.id.c_str(), name_.c_str());
        return false;
    }
    
    skills_.push_back(skill);
    skillIndices_[skill.id] = static_cast<int>(skills_.size()) - 1;
    
    CHAR_LOG_INFO("Added skill %s to character %s", skill.name.c_str(), name_.c_str());
    return true;
}

bool Character::removeSkill(const std::string& skillId) {
    auto it = skillIndices_.find(skillId);
    if (it == skillIndices_.end()) {
        CHAR_LOG_WARN("Skill %s not found for character %s", skillId.c_str(), name_.c_str());
        return false;
    }
    
    int index = it->second;
    skills_.erase(skills_.begin() + index);
    skillIndices_.erase(it);
    
    // 更新索引
    for (auto& pair : skillIndices_) {
        if (pair.second > index) {
            pair.second--;
        }
    }
    
    CHAR_LOG_INFO("Removed skill %s from character %s", skillId.c_str(), name_.c_str());
    return true;
}

bool Character::upgradeSkill(const std::string& skillId) {
    auto it = skillIndices_.find(skillId);
    if (it == skillIndices_.end()) {
        CHAR_LOG_WARN("Skill %s not found for character %s", skillId.c_str(), name_.c_str());
        return false;
    }
    
    CharacterSkill& skill = skills_[it->second];
    if (skill.level >= skill.maxLevel) {
        CHAR_LOG_WARN("Skill %s is already at max level for character %s", skillId.c_str(), name_.c_str());
        return false;
    }
    
    skill.level++;
    // TODO: 根据技能等级调整技能效果
    
    CHAR_LOG_INFO("Upgraded skill %s to level %d for character %s", 
                  skillId.c_str(), skill.level, name_.c_str());
    return true;
}

void Character::updateSkillCooldowns(double deltaTime) {
    for (auto& skill : skills_) {
        if (skill.currentCooldown > 0) {
            skill.currentCooldown -= static_cast<float>(deltaTime);
            if (skill.currentCooldown < 0) {
                skill.currentCooldown = 0;
            }
        }
    }
}

bool Character::canUseSkill(const std::string& skillId) const {
    auto it = skillIndices_.find(skillId);
    if (it == skillIndices_.end()) {
        return false;
    }
    
    const CharacterSkill& skill = skills_[it->second];
    return skill.isActive && skill.currentCooldown <= 0 && 
           stats_.mana >= skill.manaCost && stats_.stamina >= skill.staminaCost;
}

void Character::addStatusEffect(const StatusEffect& effect) {
    auto it = statusEffectIndices_.find(effect.id);
    if (it != statusEffectIndices_.end()) {
        // 更新现有效果
        statusEffects_[it->second] = effect;
        CHAR_LOG_DEBUG("Updated status effect %s for character %s", effect.name.c_str(), name_.c_str());
    } else {
        // 添加新效果
        statusEffects_.push_back(effect);
        statusEffectIndices_[effect.id] = static_cast<int>(statusEffects_.size()) - 1;
        CHAR_LOG_INFO("Added status effect %s to character %s", effect.name.c_str(), name_.c_str());
    }
    
    onStatusEffectAdded(effect);
}

void Character::removeStatusEffect(const std::string& effectId) {
    auto it = statusEffectIndices_.find(effectId);
    if (it == statusEffectIndices_.end()) {
        return;
    }
    
    int index = it->second;
    statusEffects_.erase(statusEffects_.begin() + index);
    statusEffectIndices_.erase(it);
    
    // 更新索引
    for (auto& pair : statusEffectIndices_) {
        if (pair.second > index) {
            pair.second--;
        }
    }
    
    CHAR_LOG_INFO("Removed status effect %s from character %s", effectId.c_str(), name_.c_str());
    onStatusEffectRemoved(effectId);
}

void Character::updateStatusEffects(double deltaTime) {
    removeExpiredStatusEffects();
    applyStatusEffects();
}

bool Character::hasStatusEffect(const std::string& effectId) const {
    return statusEffectIndices_.find(effectId) != statusEffectIndices_.end();
}

bool Character::hasStatusEffect(CharacterState state) const {
    for (const auto& effect : statusEffects_) {
        if (effect.state == state) {
            return true;
        }
    }
    return false;
}

bool Character::equipItem(const std::string& itemId, EquipmentSlot slot) {
    // TODO: 实现装备系统
    equippedItems_[slot] = itemId;
    updateEquipmentStats();
    
    CHAR_LOG_INFO("Character %s equipped %s in slot %d", name_.c_str(), itemId.c_str(), static_cast<int>(slot));
    onEquipmentChanged(slot, itemId);
    return true;
}

bool Character::unequipItem(EquipmentSlot slot) {
    auto it = equippedItems_.find(slot);
    if (it == equippedItems_.end()) {
        return false;
    }
    
    std::string itemId = it->second;
    equippedItems_.erase(it);
    updateEquipmentStats();
    
    CHAR_LOG_INFO("Character %s unequipped %s from slot %d", name_.c_str(), itemId.c_str(), static_cast<int>(slot));
    onEquipmentChanged(slot, "");
    return true;
}

std::string Character::getEquippedItem(EquipmentSlot slot) const {
    auto it = equippedItems_.find(slot);
    return it != equippedItems_.end() ? it->second : "";
}

void Character::updateEquipmentStats() {
    // TODO: 实现装备属性计算
    CHAR_LOG_DEBUG("Updated equipment stats for character %s", name_.c_str());
}

void Character::emitEvent(const CharacterEvent& event) {
    eventQueue_.push_back(event);
}

void Character::emitEvent(CharacterEventType type, const std::string& name, const CharacterEventData& data) {
    emitEvent(CharacterEvent(type, name, data));
}

void Character::subscribeToEvent(CharacterEventType type, CharacterEventCallback callback) {
    eventCallbacks_[type].push_back(callback);
}

void Character::unsubscribeFromEvent(CharacterEventType type, CharacterEventCallback callback) {
    // TODO: 实现事件取消订阅
    CHAR_LOG_WARN("Event unsubscription not implemented yet");
}

float Character::getDistanceTo(const std::shared_ptr<Character>& other) const {
    if (!other) {
        return std::numeric_limits<float>::max();
    }
    return position_.distance(other->getPosition());
}

bool Character::isInRange(const std::shared_ptr<Character>& other, float range) const {
    return getDistanceTo(other) <= range;
}

Vector2D Character::getDirectionTo(const std::shared_ptr<Character>& other) const {
    if (!other) {
        return Vector2D();
    }
    return (other->getPosition() - position_).normalized();
}

std::string Character::getClassString() const {
    switch (class_) {
        case CharacterClass::WARRIOR: return "Warrior";
        case CharacterClass::MAGE: return "Mage";
        case CharacterClass::ASSASSIN: return "Assassin";
        case CharacterClass::ARCHER: return "Archer";
        case CharacterClass::PRIEST: return "Priest";
        case CharacterClass::PALADIN: return "Paladin";
        case CharacterClass::NECROMANCER: return "Necromancer";
        case CharacterClass::DRUID: return "Druid";
        case CharacterClass::MONK: return "Monk";
        case CharacterClass::BERSERKER: return "Berserker";
        default: return "Unknown";
    }
}

std::string Character::getStateString() const {
    switch (state_) {
        case CharacterState::IDLE: return "Idle";
        case CharacterState::MOVING: return "Moving";
        case CharacterState::ATTACKING: return "Attacking";
        case CharacterState::CASTING: return "Casting";
        case CharacterState::STUNNED: return "Stunned";
        case CharacterState::FROZEN: return "Frozen";
        case CharacterState::POISONED: return "Poisoned";
        case CharacterState::BURNING: return "Burning";
        case CharacterState::DEAD: return "Dead";
        default: return "Unknown";
    }
}

// 内部方法实现
void Character::initializeStats() {
    // 根据职业设置初始属性
    switch (class_) {
        case CharacterClass::WARRIOR:
            stats_ = CharacterStats(1, 120, 30, 18, 12, 4);
            break;
        case CharacterClass::MAGE:
            stats_ = CharacterStats(1, 80, 80, 12, 6, 6);
            break;
        case CharacterClass::ASSASSIN:
            stats_ = CharacterStats(1, 90, 40, 20, 8, 8);
            break;
        case CharacterClass::ARCHER:
            stats_ = CharacterStats(1, 85, 45, 16, 7, 7);
            break;
        case CharacterClass::PRIEST:
            stats_ = CharacterStats(1, 95, 70, 10, 9, 5);
            break;
        case CharacterClass::PALADIN:
            stats_ = CharacterStats(1, 110, 50, 14, 14, 4);
            break;
        case CharacterClass::NECROMANCER:
            stats_ = CharacterStats(1, 85, 90, 14, 5, 5);
            break;
        case CharacterClass::DRUID:
            stats_ = CharacterStats(1, 100, 60, 13, 10, 6);
            break;
        case CharacterClass::MONK:
            stats_ = CharacterStats(1, 95, 40, 17, 11, 9);
            break;
        case CharacterClass::BERSERKER:
            stats_ = CharacterStats(1, 130, 20, 22, 6, 3);
            break;
    }
    
    stats_.experienceToNext = calculateRequiredExperience(stats_.level);
}

void Character::updateMovement(double deltaTime) {
    if (state_ != CharacterState::MOVING) {
        return;
    }
    
    if (isMovingToTarget_) {
        Vector2D direction = (targetPosition_ - position_).normalized();
        float distance = position_.distance(targetPosition_);
        
        if (distance < 0.1f) {
            // 到达目标
            stop();
            return;
        }
        
        velocity_ = direction * moveSpeed_;
    }
    
    // 更新位置
    position_ = position_ + velocity_ * static_cast<float>(deltaTime);
    
    // 应用状态效果影响
    if (hasStatusEffect(CharacterState::STUNNED) || hasStatusEffect(CharacterState::FROZEN)) {
        velocity_ = Vector2D(0.0f, 0.0f);
        state_ = CharacterState::IDLE;
    }
}

void Character::updateAnimation(double deltaTime) {
    // TODO: 实现动画系统
}

void Character::processEvents() {
    for (const auto& event : eventQueue_) {
        auto it = eventCallbacks_.find(event.type);
        if (it != eventCallbacks_.end()) {
            for (const auto& callback : it->second) {
                callback(event);
            }
        }
    }
    eventQueue_.clear();
}

void Character::applyStatusEffects() {
    for (const auto& effect : statusEffects_) {
        if (effect.isPositive) {
            // 正面效果
            if (effect.state == CharacterState::IDLE) {
                // 治疗效果
                heal(static_cast<int>(effect.value));
            }
        } else {
            // 负面效果
            if (effect.state == CharacterState::POISONED) {
                takeDamage(static_cast<int>(effect.value), "poison");
            } else if (effect.state == CharacterState::BURNING) {
                takeDamage(static_cast<int>(effect.value), "fire");
            }
        }
    }
}

void Character::removeExpiredStatusEffects() {
    auto it = statusEffects_.begin();
    while (it != statusEffects_.end()) {
        if (it->remaining <= 0) {
            std::string effectId = it->id;
            it = statusEffects_.erase(it);
            
            // 更新索引
            statusEffectIndices_.clear();
            for (size_t i = 0; i < statusEffects_.size(); ++i) {
                statusEffectIndices_[statusEffects_[i].id] = static_cast<int>(i);
            }
            
            onStatusEffectRemoved(effectId);
        } else {
            it->remaining--;
            ++it;
        }
    }
}

// 事件回调方法
void Character::onCreated() {
    emitEvent(CharacterEventType::CREATED, "Character created");
}

void Character::onDestroyed() {
    emitEvent(CharacterEventType::DESTROYED, "Character destroyed");
}

void Character::onMoved() {
    emitEvent(CharacterEventType::MOVED, "Character moved", position_);
}

void Character::onAttacked(std::shared_ptr<Character> target) {
    if (target) {
        emitEvent(CharacterEventType::ATTACKED, "Character attacked", target->getName());
    }
}

void Character::onDamaged(int damage, const std::string& damageType) {
    emitEvent(CharacterEventType::DAMAGED, "Character damaged", damage);
}

void Character::onHealed(int amount) {
    emitEvent(CharacterEventType::HEALED, "Character healed", amount);
}

void Character::onLeveledUp() {
    emitEvent(CharacterEventType::LEVELED_UP, "Character leveled up", stats_.level);
}

void Character::onExperienceGained(int amount) {
    emitEvent(CharacterEventType::EXPERIENCE_GAINED, "Character gained experience", amount);
}

void Character::onSkillUsed(const std::string& skillId) {
    emitEvent(CharacterEventType::SKILL_USED, "Character used skill", skillId);
}

void Character::onStatusEffectAdded(const StatusEffect& effect) {
    emitEvent(CharacterEventType::STATUS_EFFECT_ADDED, "Status effect added", effect.id);
}

void Character::onStatusEffectRemoved(const std::string& effectId) {
    emitEvent(CharacterEventType::STATUS_EFFECT_REMOVED, "Status effect removed", effectId);
}

void Character::onEquipmentChanged(EquipmentSlot slot, const std::string& itemId) {
    emitEvent(CharacterEventType::EQUIPMENT_CHANGED, "Equipment changed", itemId);
}

void Character::onDied() {
    state_ = CharacterState::DEAD;
    emitEvent(CharacterEventType::DIED, "Character died");
}

void Character::onRevived() {
    if (state_ == CharacterState::DEAD) {
        state_ = CharacterState::IDLE;
        stats_.health = stats_.maxHealth / 2; // 复活时恢复一半血量
        emitEvent(CharacterEventType::REVIVED, "Character revived");
    }
}

} // namespace Fantasy
