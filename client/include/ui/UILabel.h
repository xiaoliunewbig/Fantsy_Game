/**
 * @file UILabel.h
 * @brief 文本标签组件 - 显示静态或动态文本
 * @author [pengchengkang]
 * date 2025.06.19
 */
#ifndef UI_LABEL_H
#define UI_LABEL_H

#include <string>
#include "UIComponent.h"

class UILabel : public UIComponent {
public:
    UILabel();
    explicit UILabel(const std::string& text);

    void SetText(const std::string& text);
    const std::string& GetText() const;

    void SetFontSize(int size);
    int GetFontSize() const;

    void SetColor(float r, float g, float b, float a);
    void GetColor(float& r, float& g, float& b, float& a) const;

    void Render() override;

private:
    std::string text_;
    int fontSize_ = 16;
    float color_[4] = {1.0f, 1.0f, 1.0f, 1.0f};
};

#endif // UI_LABEL_H