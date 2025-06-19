/**
 * @file PlayerModel.h
 * @brief 玩家数据模型定义 - 包含玩家属性和状态
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 属性存储（HP、MP、等级等）
 * - 状态变更通知
 * - 支持序列化与反序列化
 * - 支持本地缓存和同步
 */
#ifndef PLAYER_MODEL_H
#define PLAYER_MODEL_H

#include <string>
#include <functional>

class PlayerModel {
public:
    /**
     * @brief 初始化玩家模型
     */
    void Init();

    /**
     * @brief 获取玩家ID
     * @return 玩家唯一标识符
     */
    std::string GetPlayerId() const;

    /**
     * @brief 设置玩家ID
     * @param id 新ID
     */
    void SetPlayerId(const std::string& id);

    /**
     * @brief 获取当前生命值
     * @return HP值
     */
    int GetHealth() const;

    /**
     * @brief 设置生命值
     * @param hp 新生命值
     */
    void SetHealth(int hp);

    /**
     * @brief 增加生命值
     * @param amount 增加量
     */
    void AddHealth(int amount);

    /**
     * @brief 减少生命值
     * @param amount 减少量
     */
    void SubtractHealth(int amount);

    /**
     * @brief 注册健康值变化回调
     * @param callback 回调函数
     */
    void OnHealthChanged(const std::function<void(int)>& callback);

    /**
     * @brief 获取当前魔法值
     * @return MP值
     */
    int GetMana() const;

    /**
     * @brief 设置魔法值
     * @param mp 新魔法值
     */
    void SetMana(int mp);

    /**
     * @brief 增加魔法值
     * @param amount 增加量
     */
    void AddMana(int amount);

    /**
     * @brief 减少魔法值
     * @param amount 减少量
     */
    void SubtractMana(int amount);

    /**
     * @brief 注册魔法值变化回调
     * @param callback 回调函数
     */
    void OnManaChanged(const std::function<void(int)>& callback);

    /**
     * @brief 序列化玩家数据到字符串（如 JSON）
     * @return 数据字符串
     * @todo 实现序列化格式选择（JSON / Protobuf）
     */
    std::string Serialize() const;

    /**
     * @brief 从字符串反序列化玩家数据
     * @param data 数据字符串
     * @todo 实现反序列化逻辑
     */
    void Deserialize(const std::string& data);
};

#endif // PLAYER_MODEL_H