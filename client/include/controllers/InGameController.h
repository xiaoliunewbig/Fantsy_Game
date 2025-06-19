/**
 * @file InGameController.h
 * @brief 游戏运行时控制器 - 处理战斗、移动、交互等核心玩法逻辑
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 玩家移动控制
 * - 技能释放处理
 * - 怪物AI交互
 * - 场景切换协调
 * - UI同步更新
 * - 支持暂停与恢复（TODO）
 */
#ifndef IN_GAME_CONTROLLER_H
#define IN_GAME_CONTROLLER_H

#include "GameController.h"

class InGameController : public GameController {
public:
    /**
     * @brief 初始化游戏内控制器
     */
    void Init() override;

    /**
     * @brief 处理玩家输入（键盘/鼠标/手柄）
     * @param inputType 输入类型
     * @param value 输入值
     */
    void HandleInput(const std::string& inputType, float value) override;

    /**
     * @brief 每帧更新逻辑
     * @param dt 时间间隔（秒）
     */
    void Update(float dt) override;

    /**
     * @brief 销毁控制器并清理数据
     */
    void Destroy() override;

    /**
     * @brief 获取控制器名称
     * @return 名称字符串
     */
    std::string GetName() const override;

    /**
     * @brief 触发玩家跳跃动作
     * @todo 实现物理引擎联动
     */
    void Jump();

    /**
     * @brief 触发玩家攻击动作
     * @todo 添加技能系统支持
     */
    void Attack();

    /**
     * @brief 移动玩家到指定位置
     * @param x X轴目标坐标
     * @param y Y轴目标坐标
     * @param z Z轴目标坐标
     */
    void MoveTo(float x, float y, float z);
};

#endif // IN_GAME_CONTROLLER_H