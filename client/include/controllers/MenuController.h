/**
 * @file MenuController.h
 * @brief 菜单界面控制器 - 处理主菜单、设置菜单等界面交互
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 主菜单按钮点击响应
 * - 设置菜单选项处理
 * - 鼠标悬停高亮反馈
 * - 支持多语言菜单（TODO）
 * - 支持动画过渡效果（TODO）
 */
#ifndef MENU_CONTROLLER_H
#define MENU_CONTROLLER_H

#include "GameController.h"

class MenuController : public GameController {
public:
    /**
     * @brief 初始化菜单控制器
     */
    void Init() override;

    /**
     * @brief 处理菜单输入事件
     * @param inputType 输入类型
     * @param value 输入值
     */
    void HandleInput(const std::string& inputType, float value) override;

    /**
     * @brief 更新菜单状态
     * @param dt 时间间隔（秒）
     */
    void Update(float dt) override;

    /**
     * @brief 销毁控制器
     */
    void Destroy() override;

    /**
     * @brief 获取控制器名称
     * @return 名称字符串
     */
    std::string GetName() const override;

    /**
     * @brief 进入开始游戏界面
     */
    void EnterStartGame();

    /**
     * @brief 打开设置菜单
     */
    void OpenSettings();

    /**
     * @brief 显示退出确认对话框
     */
    void ShowExitConfirmDialog();

    /**
     * @brief 切换全屏模式
     * @todo 实现窗口/全屏切换逻辑
     */
    void ToggleFullscreen();
};

#endif // MENU_CONTROLLER_H