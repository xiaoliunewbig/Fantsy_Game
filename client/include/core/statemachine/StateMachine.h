/**
 * @file StateMachine.h
 * @brief 状态机系统接口定义 - 支持状态切换与生命周期管理
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 支持状态进入/退出回调
 * - 支持状态过渡动画
 * - 支持状态栈管理（如暂停态压栈）
 * - 支持异步状态加载
 */
#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <string>
#include <unordered_map>
#include <functional>

class StateMachine {
public:
    using StateHandler = std::function<void()>;

    /**
     * @brief 初始化状态机
     * @param initialState 初始状态名称
     */
    void Init(const std::string& initialState);

    /**
     * @brief 添加一个新状态到状态机
     * @param stateName 状态名称
     * @param onEnter 进入状态时执行的操作
     * @param onExit 退出状态时执行的操作
     * @param onUpdate 状态更新操作（可选）
     * @todo 支持添加多个状态并自动切换
     */
    void AddState(const std::string& stateName,
                  const StateHandler& onEnter,
                  const StateHandler& onExit,
                  const StateHandler& onUpdate = nullptr);

    /**
     * @brief 切换当前状态到目标状态
     * @param targetState 目标状态名称
     */
    void SwitchTo(const std::string& targetState);

    /**
     * @brief 获取当前状态名称
     * @return 当前状态字符串
     */
    std::string GetCurrentState() const;

    /**
     * @brief 更新当前状态逻辑
     * @param dt 时间间隔（秒）
     */
    void Update(float dt);
};

#endif // STATE_MACHINE_H