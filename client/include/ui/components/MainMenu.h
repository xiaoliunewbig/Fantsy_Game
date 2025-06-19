/**
 * @file MainMenu.h
 * @brief 主菜单组件
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#ifndef PURE_MAINMENU_H
#define PURE_MAINMENU_H

#include <vector>
#include <string>
#include <functional>

namespace Fantasy {

// 按钮类型定义
struct Button {
    std::string label;
    std::function<void()> onClick;
};

/**
 * @brief 主菜单组件（无 Qt 依赖）
 */
class MainMenu {
public:
    // 构造函数
    MainMenu();

    // 显示/隐藏菜单
    void showMenu();
    void hideMenu();

    // 更新与渲染（模拟）
    void update(float deltaTime);
    void render() const;

    // 添加按钮
    void addButton(const std::string& label, const std::function<void()>& callback);

    // 设置标题和副标题
    void setTitle(const std::string& title);
    void setSubtitle(const std::string& subtitle);

    // 设置版权信息
    void setCopyright(const std::string& copyright);

    // 键盘输入模拟
    void onKeyPress(const std::string& key);

private:
    std::string m_title;                ///< 标题
    std::string m_subtitle;             ///< 副标题
    std::string m_copyright;            ///< 版权信息
    std::vector<Button> m_buttons;      ///< 按钮列表
    bool m_isVisible;                   ///< 是否可见
    float m_animationProgress;          ///< 动画进度 [0.0f - 1.0f]
    float m_backgroundOffset;           ///< 背景偏移动画值

    // 内部方法
    void animateTitle(float deltaTime);
    void animateButtons(float deltaTime);
    void drawBackground() const;
    void drawText(const std::string& text, int x, int y, int fontSize, const std::string& color) const;
};

} // namespace Fantasy

#endif // PURE_MAINMENU_H