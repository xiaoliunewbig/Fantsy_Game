/**
 * @file UIButton.h
 * @brief 按钮组件 - 可点击交互元素
 * @author [pengchengkang]
 * date 2025.06.19
 */
#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include <string>
#include <functional>
#include "UIComponent.h"

class UIButton : public UIComponent {
public:
    using ClickCallback = std::function<void()>;

    UIButton();
    explicit UIButton(const std::string& label);

    void SetLabel(const std::string& label);
    const std::string& GetLabel() const;

    void SetPosition(float x, float y);
    void GetPosition(float& x, float& y) const;

    void SetSize(float width, float height);
    void GetSize(float& width, float& height) const;

    void SetOnClick(ClickCallback callback);
    void OnClick();

    void Render() override;

private:
    std::string label_;
    float position_[2] = {0.0f, 0.0f};
    float size_[2] = {100.0f, 30.0f};
    ClickCallback clickCallback_;
};

#endif // UI_BUTTON_H