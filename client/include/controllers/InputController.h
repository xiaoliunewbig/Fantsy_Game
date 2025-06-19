/**
 * @file InputController.h
 * @brief 输入事件分发控制器 - 统一处理所有输入设备事件
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 支持多种输入设备（键盘、鼠标、手柄）
 * - 支持绑定自定义键位
 * - 支持输入事件过滤
 * - 支持输入记录回放（TODO）
 * - 支持输入组合检测（如连招）（TODO）
 */
#ifndef INPUT_CONTROLLER_H
#define INPUT_CONTROLLER_H

#include "GameController.h"
#include <functional>
#include <map>

class InputController : public GameController {
public:
    using InputHandler = std::function<void(float)>;

    /**
     * @brief 初始化输入控制器
     */
    void Init() override;

    /**
     * @brief 处理输入事件
     * @param inputType 输入类型
     * @param value 输入值
     */
    void HandleInput(const std::string& inputType, float value) override;

    /**
     * @brief 注册一个输入事件处理器
     * @param actionName 动作名称（如 "move_forward"）
     * @param handler 回调函数
     */
    void RegisterInputHandler(const std::string& actionName, const InputHandler& handler);

    /**
     * @brief 移除指定动作的输入处理器
     * @param actionName 动作名称
     */
    void RemoveInputHandler(const std::string& actionName);

    /**
     * @brief 获取控制器名称
     * @return 名称字符串
     */
    std::string GetName() const override;

    /**
     * @brief 设置输入映射配置
     * @param config 输入映射表
     * @todo 实现从配置文件加载映射
     */
    void SetInputMapping(const std::map<std::string, std::string>& config);

private:
    std::map<std::string, InputHandler> handlers_;
};

#endif // INPUT_CONTROLLER_H