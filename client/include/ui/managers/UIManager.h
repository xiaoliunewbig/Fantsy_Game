/**
 * @file UIManager.h
 * @brief UI 管理器 - 统一管理界面显示、切换与交互逻辑
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 支持界面打开/关闭
 * - 支持界面层级管理（Z-order）
 * - 支持界面动画过渡
 * - 支持多语言界面切换（TODO）
 * - 支持UI资源热加载（TODO）
 */
#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <string>
#include <unordered_map>
#include <functional>

class UIWindow;

/**
 * @brief UIManager 类
 * 
 * 统一管理游戏或应用程序中的 UI 窗口显示、切换与交互逻辑。
 */
class UIManager {
public:
    /**
     * @brief 获取单例实例
     * @return 单例引用
     */
    static UIManager& GetInstance();

    /**
     * @brief 初始化 UIManager
     */
    void Init();

    /**
     * @brief 打开指定名称的界面
     * @param windowName 界面名称
     */
    void OpenWindow(const std::string& windowName);

    /**
     * @brief 关闭指定名称的界面
     * @param windowName 界面名称
     */
    void CloseWindow(const std::string& windowName);

    /**
     * @brief 切换到指定界面（关闭其他所有界面）
     * @param windowName 界面名称
     */
    void SwitchWindow(const std::string& windowName);

    /**
     * @brief 注册一个新界面
     * @param windowName 界面名称
     * @param window 界面对象指针
     */
    void RegisterWindow(const std::string& windowName, UIWindow* window);

    /**
     * @brief 获取当前激活的界面名称
     * @return 界面名称字符串
     */
    std::string GetCurrentWindow() const;

    /**
     * @brief 更新所有界面状态
     * @param dt 时间间隔（秒）
     */
    void Update(float dt);

    /**
     * @brief 销毁 UIManager 并释放资源
     */
    void Destroy();

    /**
     * @brief 设置默认界面
     * @param windowName 界面名称
     * @todo 实现启动时自动加载默认界面
     */
    void SetDefaultWindow(const std::string& windowName);

private:
    UIManager() = default;
    ~UIManager() = default;

    std::unordered_map<std::string, UIWindow*> windows_;
    std::string currentWindow_;
    bool initialized_{false};

    // 禁止拷贝
    UIManager(const UIManager&) = delete;
    UIManager& operator=(const UIManager&) = delete;
};

#endif // PURE_UI_MANAGER_H