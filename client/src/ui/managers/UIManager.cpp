/**
 * @file UIManager.cpp
 * @brief UI管理器实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "ui/managers/UIManager.h"
#include <iostream>

class UIWindow {
public:
    virtual ~UIWindow() = default;
    virtual void Show() { std::cout << "[UIWindow] Showing window." << std::endl; }
    virtual void Hide() { std::cout << "[UIWindow] Hiding window." << std::endl; }
    virtual void Update(float dt) { /* 可以被重写 */ }
};

UIManager& UIManager::GetInstance() {
    static UIManager instance;
    return instance;
}

void UIManager::Init() {
    if (!initialized_) {
        std::cout << "[UIManager] Initializing..." << std::endl;
        initialized_ = true;
    }
}

void UIManager::OpenWindow(const std::string& windowName) {
    auto it = windows_.find(windowName);
    if (it != windows_.end()) {
        UIWindow* window = it->second;
        window->Show();
        currentWindow_ = windowName;
        std::cout << "[UIManager] Window '" << windowName << "' opened." << std::endl;
    } else {
        std::cerr << "[UIManager] Failed to open window: " << windowName << ". Not registered!" << std::endl;
    }
}

void UIManager::CloseWindow(const std::string& windowName) {
    auto it = windows_.find(windowName);
    if (it != windows_.end()) {
        UIWindow* window = it->second;
        window->Hide();
        if (currentWindow_ == windowName)
            currentWindow_.clear();
        std::cout << "[UIManager] Window '" << windowName << "' closed." << std::endl;
    } else {
        std::cerr << "[UIManager] Failed to close window: " << windowName << ". Not registered!" << std::endl;
    }
}

void UIManager::SwitchWindow(const std::string& windowName) {
    for (auto& pair : windows_) {
        if (pair.first == windowName)
            continue;
        pair.second->Hide();
    }

    OpenWindow(windowName);
    std::cout << "[UIManager] Switched to window: " << windowName << std::endl;
}

void UIManager::RegisterWindow(const std::string& windowName, UIWindow* window) {
    if (windows_.find(windowName) == windows_.end()) {
        windows_[windowName] = window;
        std::cout << "[UIManager] Window '" << windowName << "' registered." << std::endl;
    } else {
        std::cerr << "[UIManager] Window '" << windowName << "' already exists. Skipping registration." << std::endl;
    }
}

std::string UIManager::GetCurrentWindow() const {
    return currentWindow_;
}

void UIManager::Update(float dt) {
    for (auto& pair : windows_) {
        pair.second->Update(dt);
    }
}

void UIManager::Destroy() {
    if (initialized_) {
        std::cout << "[UIManager] Destroying UIManager..." << std::endl;
        for (auto& pair : windows_) {
            delete pair.second;
        }
        windows_.clear();
        currentWindow_.clear();
        initialized_ = false;
    }
}

void UIManager::SetDefaultWindow(const std::string& windowName) {
    // TODO: 实现启动时自动加载默认界面
    std::cout << "[UIManager] Default window set to: " << windowName << std::endl;
}