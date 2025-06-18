/**
 * @file MainWindow.h
 * @brief 幻境传说主窗口类
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QTimer>
#include <QSettings>
#include <memory>

// 前向声明
class GameScene;
class MainMenu;
class PauseMenu;
class UIManager;
class GameController;
class NetworkManager;

namespace Fantasy {
class GameScene;
class MainMenu;
class PauseMenu;
class UIManager;
class GameController;
class NetworkManager;

/**
 * @brief 游戏状态枚举
 */
enum class GameState {
    MAIN_MENU,      ///< 主菜单
    PLAYING,        ///< 游戏中
    PAUSED,         ///< 暂停
    SETTINGS,       ///< 设置
    LOADING,        ///< 加载中
    EXIT            ///< 退出
};

/**
 * @brief 主窗口类
 * 
 * 负责管理整个应用程序的主窗口，包括：
 * - 窗口布局和UI管理
 * - 游戏状态切换
 * - 菜单系统
 * - 设置管理
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
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

public slots:
    // 游戏状态变化处理
    void onGameStateChanged(GameState newState);
    void onNewGameClicked();
    void onLoadGameClicked();
    void onSettingsClicked();
    void onExitClicked();
    void onResumeClicked();
    void onBackToMainMenuClicked();
    
    // 窗口事件处理
    void onWindowStateChanged(Qt::WindowStates state);
    void onCloseRequested();
    
    // 更新处理
    void update();

signals:
    // 游戏事件信号
    void gameStarted();
    void gamePaused();
    void gameResumed();
    void gameExited();
    void settingsChanged();

protected:
    // 事件处理
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    // UI初始化
    void setupUI();
    void createMenuBar();
    void createStatusBar();
    void createCentralWidget();
    void setupConnections();
    
    // 样式设置
    void applyStylesheet();
    void loadStylesheet(const QString& filename);
    
    // 工具方法
    void logMessage(const QString& message);
    void showError(const QString& title, const QString& message);

private:
    // UI组件
    QStackedWidget* m_stackedWidget;    ///< 堆叠窗口组件
    Fantasy::MainMenu* m_mainMenu;               ///< 主菜单
    Fantasy::GameScene* m_gameScene;             ///< 游戏场景
    Fantasy::PauseMenu* m_pauseMenu;             ///< 暂停菜单
    QWidget* m_settingsWidget;          ///< 设置界面
    QWidget* m_loadingWidget;           ///< 加载界面
    
    // 管理器
    std::unique_ptr<Fantasy::UIManager> m_uiManager;      ///< UI管理器
    std::unique_ptr<Fantasy::GameController> m_gameController; ///< 游戏控制器
    std::unique_ptr<Fantasy::NetworkManager> m_networkManager; ///< 网络管理器
    
    // 状态管理
    GameState m_currentState;           ///< 当前游戏状态
    GameState m_previousState;          ///< 前一个游戏状态
    bool m_isFullscreen;                ///< 是否全屏
    
    // 定时器
    QTimer* m_updateTimer;              ///< 更新定时器
    QTimer* m_fpsTimer;                 ///< FPS定时器
    
    // 设置
    QSettings* m_settings;              ///< 设置管理器
    
    // 性能监控
    int m_frameCount;                   ///< 帧数计数
    double m_lastFpsTime;               ///< 上次FPS计算时间
    double m_currentFps;                ///< 当前FPS
    
    // 窗口属性
    QSize m_normalSize;                 ///< 正常窗口大小
    QPoint m_normalPosition;            ///< 正常窗口位置
};

} // namespace Fantasy

#endif // MAINWINDOW_H