/**
 * @file Item.h
 * @brief 物品系统 - 游戏物品管理
 * @author [pengchengkang]
 * @date 2025.06.17
 * 
 * 功能特性:
 * - 物品类型管理
 * - 装备系统
 * - 消耗品系统
 * - 物品效果
 * - 物品合成
 * - 物品交易
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

// 物品类型枚举
enum class ItemType {
    WEAPON,         // 武器
    ARMOR,          // 护甲
    SHIELD,         // 盾牌
    HELMET,         // 头盔
    GLOVES,         // 手套
    BOOTS,          // 靴子
    RING,           // 戒指
    NECKLACE,       // 项链
    CAPE,           // 披风
    CONSUMABLE,     // 消耗品
    MATERIAL,       // 材料
    QUEST_ITEM,     // 任务物品
    CURRENCY,       // 货币
    CONTAINER,      // 容器
    DECORATION,     // 装饰品
    SCROLL,         // 卷轴
    POTION,         // 药水
    FOOD,           // 食物
    DRINK,          // 饮料
    ACCESSORY       // 饰品
};

// 物品稀有度枚举
enum class ItemRarity {
    COMMON,         // 普通
    UNCOMMON,       // 罕见
    RARE,           // 稀有
    EPIC,           // 史诗
    LEGENDARY,      // 传说
    MYTHIC          // 神话
};

// 物品品质枚举
enum class ItemQuality {
    BROKEN,         // 破损
    POOR,           // 劣质
    NORMAL,         // 普通
    GOOD,           // 良好
    EXCELLENT,      // 优秀
    PERFECT         // 完美
};

// 物品效果类型枚举
enum class EffectType {
    HEAL,           // 治疗
    BUFF,           // 增益
    DEBUFF,         // 减益
    TELEPORT,       // 传送
    SUMMON,         // 召唤
    TRANSFORM       // 变形
};

// 物品效果结构
struct ItemEffect {
    std::string name;
    EffectType type;
    float value;
    int duration;  // 持续时间（秒），0表示永久
    std::string description;
    
    ItemEffect() : type(EffectType::HEAL), value(0.0f), duration(0) {}
    ItemEffect(const std::string& n, EffectType t, float v, int dur = 0)
        : name(n), type(t), value(v), duration(dur) {}
};

// 物品需求结构
struct ItemRequirements {
    int level;
    std::vector<std::string> classes;
    std::vector<std::string> skills;
    
    ItemRequirements() : level(1) {}
};

// 物品配置结构
struct ItemConfig {
    ItemRarity rarity;
    ItemQuality quality;
    bool stackable;
    int maxStack;
    std::vector<ItemEffect> effects;
    ItemRequirements requirements;
    
    ItemConfig() : rarity(ItemRarity::COMMON), quality(ItemQuality::NORMAL), 
                   stackable(false), maxStack(1) {}
};

// 物品统计结构
struct ItemStats {
    std::chrono::system_clock::time_point creationTime;
    std::chrono::system_clock::time_point lastUsedTime;
    std::chrono::system_clock::time_point lastEquippedTime;
    std::chrono::system_clock::time_point lastUnequippedTime;
    int totalUses;
    int totalEquips;
    int totalUnequips;
    int totalEvents;
    
    ItemStats() : totalUses(0), totalEquips(0), totalUnequips(0), totalEvents(0) {}
};

// 物品事件类型
enum class ItemEventType {
    ITEM_CREATED,
    ITEM_DESTROYED,
    ITEM_USED,
    ITEM_EQUIPPED,
    ITEM_UNEQUIPPED,
    ITEM_TRADED,
    ITEM_CRAFTED,
    ITEM_UPGRADED,
    ITEM_REPAIRED,
    ITEM_DAMAGED
};

// 物品事件数据
using ItemEventData = std::unordered_map<std::string, std::string>;

// 物品事件结构
struct ItemEvent {
    ItemEventType type;
    std::string name;
    ItemEventData data;
    std::chrono::system_clock::time_point timestamp;
    
    ItemEvent(ItemEventType t, const std::string& n, const ItemEventData& d = ItemEventData{})
        : type(t), name(n), data(d), timestamp(std::chrono::system_clock::now()) {}
};

// 事件回调函数类型
using ItemEventCallback = std::function<void(const ItemEvent&)>;

// 日志宏定义
#define ITEM_LOG_DEBUG(fmt, ...) 
#define ITEM_LOG_INFO(fmt, ...) 
#define ITEM_LOG_WARN(fmt, ...) 
#define ITEM_LOG_ERROR(fmt, ...) 

/**
 * @brief 物品基类
 * 
 * 提供物品的基础功能：
 * - 物品属性管理
 * - 效果系统
 * - 需求检查
 * - 使用功能
 */
class Item {
public:
    // 构造和析构
    Item(const std::string& id, const std::string& name, ItemType type, const ItemConfig& config);
    virtual ~Item();
    
    // 禁用拷贝
    Item(const Item&) = delete;
    Item& operator=(const Item&) = delete;
    
    // 基础操作
    virtual bool use(std::shared_ptr<Character> user);
    virtual bool equip(std::shared_ptr<Character> character);
    virtual bool unequip(std::shared_ptr<Character> character);
    virtual bool canUse(std::shared_ptr<Character> user) const;
    virtual bool canEquip(std::shared_ptr<Character> character) const;
    virtual void update(double deltaTime);
    
    // 物品类型检查
    virtual bool isUsable() const;
    virtual bool isEquippable() const;
    virtual bool isConsumable() const;
    virtual bool isStackable() const;
    virtual bool canStackWith(const Item& other) const;
    virtual bool addToStack(int amount);
    virtual bool removeFromStack(int amount);
    virtual int getMaxStack() const;
    virtual int getQuantity() const;
    virtual void setQuantity(int quantity);
    virtual bool isEquipped() const;
    virtual bool isBound() const;
    virtual void setBound(bool bound);
    
    // 效果管理
    virtual bool addEffect(const ItemEffect& effect);
    virtual bool removeEffect(const std::string& effectId);
    virtual std::vector<ItemEffect> getEffects() const;
    virtual bool hasEffect(const std::string& effectId) const;
    
    // 需求检查
    virtual bool meetsRequirements() const;
    virtual bool checkLevelRequirement(int level) const;
    virtual bool checkStatRequirement(const std::string& statName, int value) const;
    
    // 属性计算
    virtual ItemStats calculateStats() const;
    virtual int getValue() const;
    virtual float getWeight() const;
    
    // 事件系统
    virtual void emitEvent(const ItemEvent& event);
    virtual void emitEvent(ItemEventType type, const std::string& name, 
                          const ItemEventData& data = ItemEventData{});
    virtual void subscribeToEvent(ItemEventType type, ItemEventCallback callback);
    virtual void unsubscribeFromEvent(ItemEventType type, ItemEventCallback callback);
    
    // 获取器
    const std::string& getId() const { return id_; }
    const std::string& getName() const { return name_; }
    ItemType getType() const { return type_; }
    ItemRarity getRarity() const { return rarity_; }
    ItemQuality getQuality() const { return quality_; }
    int getLevel() const { return level_; }
    int getMaxQuantity() const { return maxQuantity_; }
    int getDurability() const { return durability_; }
    int getMaxDurability() const { return maxDurability_; }
    const ItemStats& getStats() const { return stats_; }
    const std::string& getDescription() const { return description_; }
    const std::string& getIcon() const { return icon_; }
    bool isTradeable() const { return tradeable_; }
    bool isDroppable() const { return droppable_; }
    
    // 设置器
    virtual void setName(const std::string& name) { name_ = name; }
    virtual void setDescription(const std::string& desc) { description_ = desc; }
    virtual void setRarity(ItemRarity rarity) { rarity_ = rarity; }
    virtual void setQuality(ItemQuality quality) { quality_ = quality; }
    virtual void setLevel(int level) { level_ = level; }
    virtual void setDurability(int durability) { durability_ = std::min(durability, maxDurability_); }
    virtual void setIcon(const std::string& icon) { icon_ = icon; }
    virtual void setStackable(bool stackable) { stackable_ = stackable; }
    virtual void setTradeable(bool tradeable) { tradeable_ = tradeable; }
    virtual void setDroppable(bool droppable) { droppable_ = droppable; }
    
    // 生命周期事件
    virtual void onCreated();
    virtual void onDestroyed();
    virtual void onUsed();
    virtual void onEquipped();
    virtual void onUnequipped();
    virtual void onTraded();
    virtual void onCrafted();
    virtual void onUpgraded();
    virtual void onRepaired();
    virtual void onDamaged();

protected:
    // 私有方法
    void initializeItem();
    void processEvents();
    void updateEffects(double deltaTime);
    void removeExpiredEffects();
    
    // 效果应用方法
    bool applyEffects(std::shared_ptr<Character> user);
    bool applyEffect(const ItemEffect& effect, std::shared_ptr<Character> user);
    bool applyHealEffect(const ItemEffect& effect, std::shared_ptr<Character> user);
    bool applyBuffEffect(const ItemEffect& effect, std::shared_ptr<Character> user);
    bool applyDebuffEffect(const ItemEffect& effect, std::shared_ptr<Character> user);
    bool applyTeleportEffect(const ItemEffect& effect, std::shared_ptr<Character> user);
    bool applySummonEffect(const ItemEffect& effect, std::shared_ptr<Character> user);
    bool applyTransformEffect(const ItemEffect& effect, std::shared_ptr<Character> user);
    bool applyEquipEffects(std::shared_ptr<Character> character);
    bool removeEquipEffects(std::shared_ptr<Character> character);
    int calculateHealAmount(const ItemEffect& effect, std::shared_ptr<Character> user);

private:
    // 基础属性
    std::string id_;
    std::string name_;
    std::string description_;
    std::string icon_;
    ItemType type_;
    ItemRarity rarity_;
    ItemQuality quality_;
    int level_;
    int quantity_;
    int maxQuantity_;
    int durability_;
    int maxDurability_;
    float weight_;
    int value_;
    bool stackable_;
    bool tradeable_;
    bool droppable_;
    bool bound_;
    bool equipped_;
    
    // 配置
    ItemConfig config_;
    std::vector<ItemEffect> effects_;
    ItemRequirements requirements_;
    int maxStack_;
    ItemStats stats_;
    
    // 事件系统
    std::vector<ItemEvent> eventQueue_;
    std::unordered_map<ItemEventType, std::vector<ItemEventCallback>> eventCallbacks_;
};

} // namespace Fantasy 