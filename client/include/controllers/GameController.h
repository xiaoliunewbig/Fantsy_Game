/**
 * @file GameController.h
 * @brief 游戏控制器基类 - 定义通用控制器接口
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 提供标准初始化/更新/销毁流程
 * - 支持输入事件处理
 * - 支持状态同步与UI交互
 * - 支持子控制器管理（TODO）
 */
#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include <string>

class GameController {
public:
    /**
     * @brief 构造函数
     */
    GameController();

    /**
     * @brief 初始化控制器逻辑
     */
    virtual void Init();

    /**
     * @brief 处理输入事件
     * @param inputType 输入类型（如 "keyboard", "mouse"）
     * @param value 输入值（如按键码或坐标）
     */
    virtual void HandleInput(const std::string& inputType, float value);

    /**
     * @brief 更新控制器逻辑
     * @param dt 时间间隔（秒）
     */
    virtual void Update(float dt);

    /**
     * @brief 销毁控制器并释放资源
     */
    virtual void Destroy();

    /**
     * @brief 获取当前控制器名称
     * @return 控制器标识字符串
     * @todo 实现控制器名称注册机制
     */
    virtual std::string GetName() const;
};

#endif // GAME_CONTROLLER_H