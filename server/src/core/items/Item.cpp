/**
 * @file Item.cpp
 * @brief 物品系统实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "include/core/items/Item.h"
#include "include/core/characters/Character.h"
#include <algorithm>
#include <random>

namespace Fantasy {

Item::Item(const std::string& id, const std::string& name, ItemType type, 
           const ItemConfig& config)
    : id_(id), name_(name), type_(type), config_(config),
      rarity_(config.rarity), quality_(config.quality),
      stackable_(config.stackable), maxStack_(config.maxStack),
      quantity_(1), equipped_(false), bound_(false),
      level_(1), maxQuantity_(config.maxStack), durability_(100), maxDurability_(100),
      weight_(0.0f), value_(0), tradeable_(true), droppable_(true),
      requirements_(config.requirements) {
    
    // 初始化效果
    effects_ = config.effects;
    
    // 初始化统计信息
    stats_ = ItemStats{};
    stats_.creationTime = std::chrono::system_clock::now();
}

Item::~Item() = default;

bool Item::use(std::shared_ptr<Character> user) {
    ITEM_LOG_DEBUG("Using item: {} by {}", name_, user ? user->getName() : "Unknown");
    
    if (!user) {
        ITEM_LOG_ERROR("Cannot use item: no user provided");
        return false;
    }
    
    // 检查使用条件
    if (!canUse(user)) {
        ITEM_LOG_WARN("Cannot use item: requirements not met");
        return false;
    }
    
    // 检查物品类型是否可以使用
    if (!isUsable()) {
        ITEM_LOG_WARN("Item is not usable: {}", name_);
        return false;
    }
    
    try {
        // 应用物品效果
        bool success = applyEffects(user);
        
        if (success) {
            // 减少数量（如果是消耗品）
            if (isConsumable()) {
                quantity_--;
            }
            
            // 更新统计信息
            stats_.totalUses++;
            stats_.lastUsedTime = std::chrono::system_clock::now();
            
            // 发送使用事件
            emitEvent(ItemEventType::ITEM_USED, "ItemUsed", 
                     std::unordered_map<std::string, std::string>{
                         {"itemId", id_},
                         {"itemName", name_},
                         {"userId", user->getName()},
                         {"userName", user->getName()}
                     });
            
            ITEM_LOG_INFO("Item used successfully: {} by {}", name_, user->getName());
        }
        
        return success;
        
    } catch (const std::exception& e) {
        ITEM_LOG_ERROR("Exception during item use: {}", e.what());
        return false;
    }
}

bool Item::equip(std::shared_ptr<Character> character) {
    ITEM_LOG_DEBUG("Equipping item: {} by {}", name_, character ? character->getName() : "Unknown");
    
    if (!character) {
        ITEM_LOG_ERROR("Cannot equip item: no character provided");
        return false;
    }
    
    // 检查是否可以装备
    if (!isEquippable()) {
        ITEM_LOG_WARN("Item is not equippable: {}", name_);
        return false;
    }
    
    // 检查装备需求
    if (!canEquip(character)) {
        ITEM_LOG_WARN("Cannot equip item: requirements not met");
        return false;
    }
    
    try {
        // 应用装备效果
        bool success = applyEquipEffects(character);
        
        if (success) {
            equipped_ = true;
            
            // 更新统计信息
            stats_.totalEquips++;
            stats_.lastEquippedTime = std::chrono::system_clock::now();
            
            // 发送装备事件
            emitEvent(ItemEventType::ITEM_EQUIPPED, "ItemEquipped", 
                     std::unordered_map<std::string, std::string>{
                         {"itemId", id_},
                         {"itemName", name_},
                         {"characterId", character->getName()},
                         {"characterName", character->getName()}
                     });
            
            ITEM_LOG_INFO("Item equipped successfully: {} by {}", name_, character->getName());
        }
        
        return success;
        
    } catch (const std::exception& e) {
        ITEM_LOG_ERROR("Exception during item equip: {}", e.what());
        return false;
    }
}

bool Item::unequip(std::shared_ptr<Character> character) {
    ITEM_LOG_DEBUG("Unequipping item: {} by {}", name_, character ? character->getName() : "Unknown");
    
    if (!character) {
        ITEM_LOG_ERROR("Cannot unequip item: no character provided");
        return false;
    }
    
    if (!equipped_) {
        ITEM_LOG_WARN("Item is not equipped: {}", name_);
        return false;
    }
    
    try {
        // 移除装备效果
        bool success = removeEquipEffects(character);
        
        if (success) {
            equipped_ = false;
            
            // 更新统计信息
            stats_.totalUnequips++;
            stats_.lastUnequippedTime = std::chrono::system_clock::now();
            
            // 发送卸下事件
            emitEvent(ItemEventType::ITEM_UNEQUIPPED, "ItemUnequipped", 
                     std::unordered_map<std::string, std::string>{
                         {"itemId", id_},
                         {"itemName", name_},
                         {"characterId", character->getName()},
                         {"characterName", character->getName()}
                     });
            
            ITEM_LOG_INFO("Item unequipped successfully: {} by {}", name_, character->getName());
        }
        
        return success;
        
    } catch (const std::exception& e) {
        ITEM_LOG_ERROR("Exception during item unequip: {}", e.what());
        return false;
    }
}

bool Item::canUse(std::shared_ptr<Character> user) const {
    if (!user) {
        return false;
    }
    
    // 检查等级需求
    if (requirements_.level > 0) {
        // TODO: 获取用户实际等级
        int userLevel = 1; // 临时值
        if (userLevel < requirements_.level) {
            return false;
        }
    }
    
    // 检查职业需求
    if (!requirements_.classes.empty()) {
        // TODO: 获取用户实际职业
        std::string userClass = "Warrior"; // 临时值
        if (std::find(requirements_.classes.begin(), requirements_.classes.end(), 
                     userClass) == requirements_.classes.end()) {
            return false;
        }
    }
    
    // 检查技能需求
    for (const auto& skillReq : requirements_.skills) {
        // TODO: 检查用户是否拥有所需技能
        // if (!user->hasSkill(skillReq.name) || user->getSkillLevel(skillReq.name) < skillReq.level) {
        //     return false;
        // }
    }
    
    return true;
}

bool Item::canEquip(std::shared_ptr<Character> character) const {
    if (!character) {
        return false;
    }
    
    // 检查使用条件
    if (!canUse(character)) {
        return false;
    }
    
    // 检查装备槽位
    if (!isEquippable()) {
        return false;
    }
    
    // TODO: 检查装备槽位是否可用
    // 1. 检查是否已有装备在相同槽位
    // 2. 检查槽位是否解锁
    
    return true;
}

bool Item::isUsable() const {
    switch (type_) {
        case ItemType::CONSUMABLE:
        case ItemType::SCROLL:
        case ItemType::POTION:
        case ItemType::FOOD:
        case ItemType::DRINK:
            return true;
        default:
            return false;
    }
}

bool Item::isEquippable() const {
    switch (type_) {
        case ItemType::WEAPON:
        case ItemType::ARMOR:
        case ItemType::ACCESSORY:
        case ItemType::SHIELD:
            return true;
        default:
            return false;
    }
}

bool Item::isConsumable() const {
    switch (type_) {
        case ItemType::CONSUMABLE:
        case ItemType::POTION:
        case ItemType::FOOD:
        case ItemType::DRINK:
            return true;
        default:
            return false;
    }
}

bool Item::isStackable() const {
    return stackable_;
}

bool Item::canStackWith(const Item& other) const {
    if (!stackable_ || !other.stackable_) {
        return false;
    }
    
    return id_ == other.id_ && 
           rarity_ == other.rarity_ && 
           quality_ == other.quality_;
}

bool Item::addToStack(int amount) {
    if (!stackable_) {
        return false;
    }
    
    if (quantity_ + amount > maxStack_) {
        return false;
    }
    
    quantity_ += amount;
    return true;
}

bool Item::removeFromStack(int amount) {
    if (quantity_ < amount) {
        return false;
    }
    
    quantity_ -= amount;
    return true;
}

int Item::getMaxStack() const {
    return maxStack_;
}

int Item::getQuantity() const {
    return quantity_;
}

void Item::setQuantity(int quantity) {
    if (quantity >= 0 && quantity <= maxStack_) {
        quantity_ = quantity;
    }
}

bool Item::isEquipped() const {
    return equipped_;
}

bool Item::isBound() const {
    return bound_;
}

void Item::setBound(bool bound) {
    bound_ = bound;
}

std::vector<ItemEffect> Item::getEffects() const {
    return effects_;
}

bool Item::addEffect(const ItemEffect& effect) {
    effects_.push_back(effect);
    return true;
}

bool Item::removeEffect(const std::string& effectId) {
    auto it = std::remove_if(effects_.begin(), effects_.end(),
        [&effectId](const ItemEffect& effect) {
            return effect.name == effectId;
        });
    
    if (it != effects_.end()) {
        effects_.erase(it, effects_.end());
        return true;
    }
    return false;
}

void Item::emitEvent(const ItemEvent& event) {
    eventQueue_.push_back(event);
    stats_.totalEvents++;
}

void Item::emitEvent(ItemEventType type, const std::string& name, const ItemEventData& data) {
    emitEvent(ItemEvent(type, name, data));
}

void Item::subscribeToEvent(ItemEventType type, ItemEventCallback callback) {
    eventCallbacks_[type].push_back(callback);
}

void Item::unsubscribeFromEvent(ItemEventType type, ItemEventCallback callback) {
    auto it = eventCallbacks_.find(type);
    if (it != eventCallbacks_.end()) {
        auto& callbacks = it->second;
        callbacks.erase(
            std::remove_if(callbacks.begin(), callbacks.end(),
                [&callback](const ItemEventCallback& cb) {
                    return cb.target_type() == callback.target_type();
                }),
            callbacks.end()
        );
    }
}

// 私有方法实现
bool Item::applyEffects(std::shared_ptr<Character> user) {
    ITEM_LOG_DEBUG("Applying {} effects for item: {}", effects_.size(), name_);
    
    bool allSuccess = true;
    
    for (const auto& effect : effects_) {
        if (!applyEffect(effect, user)) {
            ITEM_LOG_ERROR("Failed to apply effect: {}", effect.name);
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool Item::applyEffect(const ItemEffect& effect, std::shared_ptr<Character> user) {
    ITEM_LOG_DEBUG("Applying effect: {} with value: {}", effect.name, effect.value);
    
    try {
        switch (effect.type) {
            case EffectType::HEAL:
                return applyHealEffect(effect, user);
            case EffectType::BUFF:
                return applyBuffEffect(effect, user);
            case EffectType::DEBUFF:
                return applyDebuffEffect(effect, user);
            case EffectType::TELEPORT:
                return applyTeleportEffect(effect, user);
            case EffectType::SUMMON:
                return applySummonEffect(effect, user);
            case EffectType::TRANSFORM:
                return applyTransformEffect(effect, user);
            default:
                ITEM_LOG_ERROR("Unknown effect type: {}", static_cast<int>(effect.type));
                return false;
        }
    } catch (const std::exception& e) {
        ITEM_LOG_ERROR("Exception during effect application: {}", e.what());
        return false;
    }
}

bool Item::applyHealEffect(const ItemEffect& effect, std::shared_ptr<Character> user) {
    // TODO: 实现治疗效果
    // 1. 计算治疗量
    // 2. 应用治疗
    // 3. 检查是否暴击
    
    int healAmount = calculateHealAmount(effect, user);
    
    // user->heal(healAmount);
    
    ITEM_LOG_DEBUG("Applied heal effect: {} HP to {}", healAmount, user->getName());
    return true;
}

bool Item::applyBuffEffect(const ItemEffect& effect, std::shared_ptr<Character> user) {
    // TODO: 实现增益效果
    // 1. 检查是否已有相同效果
    // 2. 应用增益
    // 3. 设置持续时间
    
    ITEM_LOG_DEBUG("Applied buff effect: {} to {}", effect.name, user->getName());
    return true;
}

bool Item::applyDebuffEffect(const ItemEffect& effect, std::shared_ptr<Character> user) {
    // TODO: 实现减益效果
    // 1. 检查抗性
    // 2. 应用减益
    // 3. 设置持续时间
    
    ITEM_LOG_DEBUG("Applied debuff effect: {} to {}", effect.name, user->getName());
    return true;
}

bool Item::applyTeleportEffect(const ItemEffect& effect, std::shared_ptr<Character> user) {
    // TODO: 实现传送效果
    // 1. 解析目标位置
    // 2. 检查传送条件
    // 3. 执行传送
    
    ITEM_LOG_DEBUG("Applied teleport effect to {}", user->getName());
    return true;
}

bool Item::applySummonEffect(const ItemEffect& effect, std::shared_ptr<Character> user) {
    // TODO: 实现召唤效果
    // 1. 解析召唤物信息
    // 2. 检查召唤条件
    // 3. 创建召唤物
    
    ITEM_LOG_DEBUG("Applied summon effect for {}", user->getName());
    return true;
}

bool Item::applyTransformEffect(const ItemEffect& effect, std::shared_ptr<Character> user) {
    // TODO: 实现变形效果
    // 1. 解析变形目标
    // 2. 应用变形
    // 3. 设置持续时间
    
    ITEM_LOG_DEBUG("Applied transform effect to {}", user->getName());
    return true;
}

bool Item::applyEquipEffects(std::shared_ptr<Character> character) {
    ITEM_LOG_DEBUG("Applying equip effects for item: {}", name_);
    
    // TODO: 实现装备效果
    // 1. 增加属性
    // 2. 激活特殊能力
    // 3. 更新装备状态
    
    return true;
}

bool Item::removeEquipEffects(std::shared_ptr<Character> character) {
    ITEM_LOG_DEBUG("Removing equip effects for item: {}", name_);
    
    // TODO: 实现移除装备效果
    // 1. 减少属性
    // 2. 停用特殊能力
    // 3. 更新装备状态
    
    return true;
}

int Item::calculateHealAmount(const ItemEffect& effect, std::shared_ptr<Character> user) {
    // TODO: 实现治疗量计算
    // 1. 基础治疗量
    // 2. 治疗效果加成
    // 3. 暴击治疗
    // 4. 随机波动
    
    int baseHeal = effect.value;
    
    // 简单的随机化
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(static_cast<int>(baseHeal * 0.9), 
                                       static_cast<int>(baseHeal * 1.1));
    
    return std::max(1, dis(gen)); // 至少治疗1点
}

void Item::processEvents() {
    for (const auto& event : eventQueue_) {
        auto it = eventCallbacks_.find(event.type);
        if (it != eventCallbacks_.end()) {
            for (const auto& callback : it->second) {
                try {
                    callback(event);
                } catch (const std::exception& e) {
                    ITEM_LOG_ERROR("Event callback error: {}", e.what());
                }
            }
        }
    }
    
    eventQueue_.clear();
}

} // namespace Fantasy 