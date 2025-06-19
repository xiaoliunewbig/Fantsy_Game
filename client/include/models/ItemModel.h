/**
 * @file ItemModel.h
 * @brief 道具数据模型 - 包含游戏内道具的基本属性和操作
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 道具ID、名称、类型管理
 * - 使用效果绑定
 * - 支持堆叠数量
 * - 支持持久化存储
 * - 支持自定义脚本扩展（TODO）
 */
#ifndef ITEM_MODEL_H
#define ITEM_MODEL_H

#include <string>
#include <functional>
#include <unordered_map>

class ItemModel {
public:
    enum class ItemType {
        CONSUMABLE,
        EQUIPMENT,
        QUEST,
        MISC
    };

    /**
     * @brief 构造函数
     * @param id 道具唯一标识
     * @param name 道具名称
     * @param type 道具类型
     */
    ItemModel(const std::string& id, const std::string& name, ItemType type);

    /**
     * @brief 获取道具ID
     * @return ID字符串
     */
    std::string GetId() const;

    /**
     * @brief 获取道具名称
     * @return 名称字符串
     */
    std::string GetName() const;

    /**
     * @brief 获取道具类型
     * @return 类型枚举
     */
    ItemType GetType() const;

    /**
     * @brief 设置道具堆叠数量
     * @param count 数量
     */
    void SetStackCount(int count);

    /**
     * @brief 获取当前堆叠数量
     * @return 数量
     */
    int GetStackCount() const;

    /**
     * @brief 增加堆叠数量
     * @param amount 增加量
     */
    void AddStackCount(int amount);

    /**
     * @brief 减少堆叠数量
     * @param amount 减少量
     */
    void SubtractStackCount(int amount);

    /**
     * @brief 注册使用道具的回调函数
     * @param callback 回调函数
     */
    void RegisterUseCallback(const std::function<void()>& callback);

    /**
     * @brief 使用道具
     */
    void Use();

    /**
     * @brief 序列化道具数据到字符串（如 JSON）
     * @return 数据字符串
     * @todo 实现序列化格式选择（JSON / Protobuf）
     */
    std::string Serialize() const;

    /**
     * @brief 从字符串反序列化道具数据
     * @param data 数据字符串
     * @todo 实现反序列化逻辑
     */
    void Deserialize(const std::string& data);

private:
    std::string id_;
    std::string name_;
    ItemType type_;
    int stackCount_;
    std::function<void()> useCallback_;
};

#endif // ITEM_MODEL_H