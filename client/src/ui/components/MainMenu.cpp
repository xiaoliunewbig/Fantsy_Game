/**
 * @file MainMenu.cpp
 * @brief 主菜单组件实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "ui/components/MainMenu.h"
#include <iostream>
#include <cmath>

namespace Fantasy {

MainMenu::MainMenu()
    : m_title("Fantasy Legend"),
      m_subtitle("Press Enter to Start"),
      m_copyright("© 2025 Fantasy Studio"),
      m_isVisible(false),
      m_animationProgress(0.0f),
      m_backgroundOffset(0.0f)
{
}

void MainMenu::showMenu() {
    m_isVisible = true;
    m_animationProgress = 0.0f;
    std::cout << "[MainMenu] Showing menu." << std::endl;
}

void MainMenu::hideMenu() {
    m_isVisible = false;
    std::cout << "[MainMenu] Hiding menu." << std::endl;
}

void MainMenu::update(float deltaTime) {
    if (!m_isVisible) return;

    animateTitle(deltaTime);
    animateButtons(deltaTime);

    m_backgroundOffset += deltaTime * 10.0f;
}

void MainMenu::render() const {
    if (!m_isVisible) return;

    drawBackground();

    // 渲染标题
    drawText(m_title, 400, 100 + static_cast<int>(sin(m_backgroundOffset) * 10), 36, "#FFFFFF");
    drawText(m_subtitle, 400, 180, 18, "#CCCCCC");

    // 渲染按钮
    int buttonY = 300;
    for (const auto& button : m_buttons) {
        drawText(button.label, 400, buttonY, 16, "#4CAF50");
        buttonY += 60;
    }

    drawText(m_copyright, 700, 580, 12, "#AAAAAA");
}

void MainMenu::addButton(const std::string& label, const std::function<void()>& callback) {
    m_buttons.push_back({label, callback});
}

void MainMenu::setTitle(const std::string& title) {
    m_title = title;
}

void MainMenu::setSubtitle(const std::string& subtitle) {
    m_subtitle = subtitle;
}

void MainMenu::setCopyright(const std::string& copyright) {
    m_copyright = copyright;
}

void MainMenu::onKeyPress(const std::string& key) {
    if (key == "Enter") {
        if (!m_buttons.empty()) {
            m_buttons[0].onClick(); // 触发第一个按钮
        }
    }
}

void MainMenu::animateTitle(float deltaTime) {
    m_animationProgress += deltaTime;
    if (m_animationProgress > 1.0f) m_animationProgress = 1.0f;
}

void MainMenu::animateButtons(float deltaTime) {
    // 可以扩展按钮动画逻辑
}

void MainMenu::drawBackground() const {
    // 模拟绘制渐变背景
    std::cout << "[MainMenu] Drawing background..." << std::endl;
}

void MainMenu::drawText(const std::string& text, int x, int y, int fontSize, const std::string& color) const {
    std::cout << "[MainMenu] Render text: \"" << text << "\" at (" << x << ", " << y << ") with size " << fontSize << " and color " << color << std::endl;
}

} // namespace Fantasy