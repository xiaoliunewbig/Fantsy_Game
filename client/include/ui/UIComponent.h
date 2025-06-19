/**
 * @file UIComponent.h
 * @brief UI 组件基类 - 定义 UI 元素的基础行为
 * @author [pengchengkang]
 * date 2025.06.19
 */
#ifndef UI_COMPONENT_H
#define UI_COMPONENT_H

class UIComponent {
public:
    virtual ~UIComponent() = default;

    virtual void Render() = 0;
    virtual bool IsVisible() const = 0;
    virtual void SetVisible(bool visible) = 0;
};

#endif // UI_COMPONENT_H