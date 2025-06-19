/**
 * @file MainWindow.h
 * @brief 幻境传说主窗口类
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>

// 前向声明
class GameScene;
class MainMenu;
class PauseMenu;
// class UIManager;
// class GameController;
// class NetworkManager;

namespace Fantasy {

// 游戏状态枚举
enum class GameState {
    MAIN_MENU,      ///< 主菜单
    PLAYING,        ///< 游戏中
    PAUSED,         ///< 暂停
    SETTINGS,       ///< 设置
    LOADING,        ///< 加载中
    EXIT            ///< 退出
};

// 窗口状态枚举
enum class WindowState {
    NORMAL,
    FULLSCREEN
};

// 简化模拟类
class Widget {
public:
    virtual void show() {}
    virtual void hide() {}
    virtual void update(float dt) {}
    virtual void render() const {}
};

/**
 * @brief 主窗口类（无 Qt 依赖）
 *
 * 负责管理整个应用程序的主窗口，包括：
 * - 窗口布局和UI管理
 * - 游戏状态切换
 * - 菜单系统
 * - 设置管理
 */
class MainWindow {
public:
    // 构造函数与析构函数
    MainWindow();
    ~MainWindow();

    // 游戏状态管理
    void showMainMenu();
    void showGameScene();
    void showPauseMenu();
    void showSettings();
    void showLoadingScreen();

    // 窗口管理
    void setFullscreen(bool fullscreen);
    void toggleFullscreen();
    void centerOnScreen();

    // 设置管理
    void loadSettings();
    void saveSettings();
    void applySettings();

    // 事件处理（模拟）
    void onGameStateChanged(GameState newState);
    void onNewGameClicked();
    void onLoadGameClicked();
    void onSettingsClicked();
    void onExitClicked();
    void onResumeClicked();
    void onBackToMainMenuClicked();

    void onWindowStateChanged(WindowState state);
    void onCloseRequested();

    void update();  // 更新循环
    void render();  // 渲染循环

    // 信号回调注册
    using Callback = std::function<void()>;
    void onGameStarted(Callback cb);
    void onGamePaused(Callback cb);
    void onGameResumed(Callback cb);
    void onGameExited(Callback cb);
    void onSettingsChanged(Callback cb);

    // 系统事件模拟
    void closeEvent();
    void keyPressEvent(const std::string& key);
    void resizeEvent(int width, int height);
    void showEvent();

private:
    // 初始化
    void setupUI();
    void createMenuBar();
    void createStatusBar();
    void createCentralWidget();
    void setupConnections();

    // 样式设置
    void applyStylesheet();
    void loadStylesheet(const std::string& filename);

    // 工具方法
    void logMessage(const std::string& message);
    void showError(const std::string& title, const std::string& message);

private:
    // UI组件
    std::map<GameState, std::shared_ptr<Widget>> m_widgets; ///< 所有界面组件
    std::shared_ptr<MainMenu> m_mainMenu;                   ///< 主菜单
    std::shared_ptr<GameScene> m_gameScene;                 ///< 游戏场景
    std::shared_ptr<PauseMenu> m_pauseMenu;                 ///< 暂停菜单
    std::shared_ptr<Widget> m_settingsWidget;               ///< 设置界面
    std::shared_ptr<Widget> m_loadingWidget;                ///< 加载界面

    // 管理器
    // std::unique_ptr<UIManager> m_uiManager;         ///< UI管理器
    // std::unique_ptr<GameController> m_gameController; ///< 游戏控制器
    // std::unique_ptr<NetworkManager> m_networkManager; ///< 网络管理器

    // 状态管理
    GameState m_currentState;           ///< 当前游戏状态
    GameState m_previousState;          ///< 前一个游戏状态
    bool m_isFullscreen;                ///< 是否全屏

    // 定时器
    float m_updateInterval;             ///< 更新间隔（秒）
    double m_lastUpdateTime;            ///< 上次更新时间

    // 设置
    std::map<std::string, std::string> m_settings; ///< 设置键值对

    // 性能监控
    int m_frameCount;                   ///< 帧数计数
    double m_lastFpsTime;               ///< 上次FPS计算时间
    double m_currentFps;                ///< 当前FPS

    // 窗口属性
    int m_width;
    int m_height;
    int m_x;
    int m_y;

    // 回调函数
    std::vector<Callback> m_gameStartedCallbacks;
    std::vector<Callback> m_gamePausedCallbacks;
    std::vector<Callback> m_gameResumedCallbacks;
    std::vector<Callback> m_gameExitedCallbacks;
    std::vector<Callback> m_settingsChangedCallbacks;
};

} // namespace Fantasy

#endif // MAINWINDOW_H