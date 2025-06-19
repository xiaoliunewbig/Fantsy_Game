/**
 * @file CharacterModel.h
 * @brief 角色通用模型定义 - 包含角色基础属性和动画状态
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 移动状态跟踪
 * - 动画播放控制
 * - AI行为管理
 * - 支持子角色扩展（如NPC、怪物）
 * - 支持自定义动作（TODO）
 */
#ifndef CHARACTER_MODEL_H
#define CHARACTER_MODEL_H

#include <string>
#include <vector>
#include <functional>
#include <map>

class CharacterModel {
public:
    enum class AnimationState {
        IDLE,
        WALK,
        RUN,
        JUMP,
        ATTACK,
        DEAD
    };

    /**
     * @brief 初始化角色模型
     */
    virtual void Init();

    /**
     * @brief 设置角色名称
     * @param name 名称
     */
    void SetName(const std::string& name);

    /**
     * @brief 获取角色名称
     * @return 名称字符串
     */
    std::string GetName() const;

    /**
     * @brief 设置角色坐标
     * @param x X轴位置
     * @param y Y轴位置
     * @param z Z轴位置
     */
    void SetPosition(float x, float y, float z);

    /**
     * @brief 获取角色坐标
     * @return 坐标三元组
     */
    std::tuple<float, float, float> GetPosition() const;

    /**
     * @brief 设置当前动画状态
     * @param state 动画状态
     */
    void SetAnimationState(AnimationState state);

    /**
     * @brief 获取当前动画状态
     * @return 动画状态枚举
     */
    AnimationState GetAnimationState() const;

    /**
     * @brief 更新角色逻辑
     * @param dt 时间间隔（秒）
     */
    virtual void Update(float dt);

    /**
     * @brief 销毁角色模型并释放资源
     */
    virtual void Destroy();

    /**
     * @brief 注册动画状态变化回调
     * @param callback 回调函数
     */
    void OnAnimationStateChanged(const std::function<void(AnimationState)>& callback);

    /**
     * @brief 添加一个AI行为
     * @param behavior AI行为名称
     * @param callback 行为执行函数
     * @todo 实现完整AI行为系统
     */
    void AddBehavior(const std::string& behavior, const std::function<void(float)>& callback);

private:
    std::string name_;
    float posX_, posY_, posZ_;
    AnimationState currentState_;
    std::vector<std::function<void(AnimationState)>> animationCallbacks_;
    std::map<std::string, std::function<void(float)>> behaviors_;
};

#endif // CHARACTER_MODEL_H