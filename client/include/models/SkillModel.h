/**
 * @file SkillModel.h
 * @brief 技能数据模型 - 包含角色技能的基本属性和效果
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 技能ID、名称、等级管理
 * - 冷却时间与消耗管理
 * - 支持绑定技能触发事件
 * - 支持序列化与反序列化
 * - 支持脚本扩展技能效果（TODO）
 */
#ifndef SKILL_MODEL_H
#define SKILL_MODEL_H

#include <string>
#include <functional>
#include <chrono>

class SkillModel {
public:
    using SkillEffectCallback = std::function<void()>;

    /**
     * @brief 构造函数
     * @param id 技能唯一标识
     * @param name 技能名称
     * @param level 技能等级
     */
    SkillModel(const std::string& id, const std::string& name, int level);

    /**
     * @brief 获取技能ID
     * @return ID字符串
     */
    std::string GetId() const;

    /**
     * @brief 获取技能名称
     * @return 名称字符串
     */
    std::string GetName() const;

    /**
     * @brief 获取技能等级
     * @return 技能等级
     */
    int GetLevel() const;

    /**
     * @brief 设置技能等级
     * @param level 新等级
     */
    void SetLevel(int level);

    /**
     * @brief 获取冷却时间（毫秒）
     * @return 冷却时间
     */
    int GetCooldown() const;

    /**
     * @brief 设置冷却时间（毫秒）
     * @param cooldownMs 冷却时间
     */
    void SetCooldown(int cooldownMs);

    /**
     * @brief 获取当前剩余冷却时间
     * @return 剩余时间（ms）
     */
    int GetRemainingCooldown() const;

    /**
     * @brief 开始冷却计时
     */
    void StartCooldown();

    /**
     * @brief 是否处于冷却中
     * @return 是否冷却中
     */
    bool IsOnCooldown() const;

    /**
     * @brief 触发技能效果
     */
    void Trigger();

    /**
     * @brief 绑定技能触发后的效果函数
     * @param callback 效果回调
     */
    void BindEffect(const SkillEffectCallback& callback);

    /**
     * @brief 序列化技能数据到字符串（如 JSON）
     * @return 数据字符串
     * @todo 实现序列化格式选择（JSON / Protobuf）
     */
    std::string Serialize() const;

    /**
     * @brief 从字符串反序列化技能数据
     * @param data 数据字符串
     * @todo 实现反序列化逻辑
     */
    void Deserialize(const std::string& data);

private:
    std::string id_;
    std::string name_;
    int level_;
    int cooldownMs_;
    std::chrono::steady_clock::time_point lastUsedTime_;
    SkillEffectCallback effectCallback_;
};

#endif // SKILL_MODEL_H