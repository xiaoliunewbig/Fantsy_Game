/**
 * @file PauseMenu.cpp
 * @brief 暂停菜单组件实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "ui/components/PauseMenu.h"
#include <iostream>

namespace Fantasy {

PauseMenu::PauseMenu()
    : m_title("Game Paused"),
      m_isVisible(false),
      m_opacity(0.0f)
{
}

void PauseMenu::showMenu() {
    m_isVisible = true;
    m_opacity = 0.0f;
    std::cout << "[PauseMenu] Showing menu." << std::endl;
}

void PauseMenu::hideMenu() {
    m_isVisible = false;
    std::cout << "[PauseMenu] Hiding menu." << std::endl;
}

void PauseMenu::update(float deltaTime) {
    if (!m_isVisible) return;

    fadeIn(deltaTime);
}

void PauseMenu::render() const {
    if (!m_isVisible) return;

    drawBackground();

    // 绘制标题
    drawText(m_title, 400, 200, 28, "#FFFFFF");

    // 绘制按钮
    int buttonY = 300;
    for (const auto& button : m_buttons) {
        drawText(button.label, 400, buttonY, 16, "#2196F3");
        buttonY += 60;
    }
}

void PauseMenu::addButton(const std::string& label, const std::function<void()>& callback) {
    m_buttons.push_back({label, callback});
}

void PauseMenu::setTitle(const std::string& title) {
    m_title = title;
}

void PauseMenu::onKeyPress(const std::string& key) {
    if (key == "Escape") {
        if (!m_buttons.empty()) {
            m_buttons[0].onClick(); // 默认触发第一个按钮（继续游戏）
        }
    }
}

void PauseMenu::fadeIn(float deltaTime) {
    m_opacity += deltaTime * 2.0f;
    if (m_opacity > 1.0f) m_opacity = 1.0f;
}

void PauseMenu::fadeOut(float deltaTime) {
    //Q_UNUSED(deltaTime);
    hideMenu();
}

void PauseMenu::drawBackground() const {
    // 半透明黑色背景
    std::cout << "[PauseMenu] Drawing translucent background..." << std::endl;
}

void PauseMenu::drawText(const std::string& text, int x, int y, int fontSize, const std::string& color) const {
    std::cout << "[PauseMenu] Render text: \"" << text << "\" at (" << x << ", " << y << ") with size " << fontSize << " and color " << color << std::endl;
}

} // namespace Fantasy