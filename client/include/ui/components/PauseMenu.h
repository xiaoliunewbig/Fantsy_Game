/**
 * @file PauseMenu.h
 * @brief 暂停菜单组件
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#ifndef PURE_PAUSEMENU_H
#define PURE_PAUSEMENU_H

#include <string>
#include <vector>
#include <functional>

namespace Fantasy {

// 按钮类型定义
struct Button {
    std::string label;
    std::function<void()> onClick;
};

/**
 * @brief 暂停菜单组件（无 Qt 依赖）
 */
class PauseMenu {
public:
    // 构造函数
    PauseMenu();

    // 显示/隐藏菜单
    void showMenu();
    void hideMenu();

    // 更新与渲染
    void update(float deltaTime);
    void render() const;

    // 添加按钮
    void addButton(const std::string& label, const std::function<void()>& callback);

    // 设置标题
    void setTitle(const std::string& title);

    // 键盘输入模拟
    void onKeyPress(const std::string& key);

private:
    std::string m_title;                ///< 标题
    std::vector<Button> m_buttons;      ///< 按钮列表
    bool m_isVisible;                   ///< 是否可见
    float m_opacity;                    ///< 不透明度 [0.0f - 1.0f]

    // 内部方法
    void fadeIn(float deltaTime);
    void fadeOut(float deltaTime);
    void drawBackground() const;
    void drawText(const std::string& text, int x, int y, int fontSize, const std::string& color) const;
};

} // namespace Fantasy

#endif // PURE_PAUSEMENU_H