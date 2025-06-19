/**
 * @file UIManagerBase.h
 * @brief UI 管理器基类 - 提供通用 UI 管理接口
 * @author [pengchengkang]
 * date 2025.06.19
 */
#ifndef UI_MANAGER_BASE_H
#define UI_MANAGER_BASE_H

#include <string>
#include <memory>

class UIWindow;

class UIManagerBase {
public:
    virtual ~UIManagerBase() = default;

    virtual void Init() = 0;
    virtual void Update(float dt) = 0;
    virtual void Render() = 0;

    virtual void ShowWindow(const std::string& windowName) = 0;
    virtual void HideWindow(const std::string& windowName) = 0;
    virtual void RegisterWindow(const std::string& name, std::unique_ptr<UIWindow> window) = 0;
    virtual void UnregisterWindow(const std::string& name) = 0;
};

#endif // UI_MANAGER_BASE_H