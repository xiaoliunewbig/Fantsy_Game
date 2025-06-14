#pragma once

#include <QMainWindow>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QKeyEvent>
#include <QCloseEvent>
#include "GameScene.h"
#include "UIManager.h"

// 前向声明
class GameEngine;

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    BATTLE,
    DIALOGUE,
    INVENTORY
};

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
    // 界面切换
    void showMainMenu();
    void showGameScene();
    void showPauseMenu();
    void showSettingsMenu();
    void showInventory();
    void showDialogue();
    
    // 窗口控制
    void setFullscreen(bool fullscreen);
    void setWindowSize(int width, int height);
    void centerWindow();
    
    // 获取器
    GameScene* getGameScene() const { return m_gameScene; }
    UIManager* getUIManager() const { return m_uiManager; }
    GameState getCurrentState() const { return m_currentState; }
    
protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    
private slots:
    void onGameStateChanged(GameState state);
    void onNewGameClicked();
    void onLoadGameClicked();
    void onSettingsClicked();
    void onExitClicked();
    void onResumeClicked();
    void onSaveGameClicked();
    void onLoadGameFromMenuClicked();
    void onBackToMainMenuClicked();
    
    // 键盘快捷键处理
    void onEscapePressed();
    void onSpacePressed();
    void onEnterPressed();
    
private:
    void setupUI();
    void createMainMenu();
    void createPauseMenu();
    void createSettingsMenu();
    void setupConnections();
    void applyStyles();
    
    // UI组件
    QStackedWidget* m_stackedWidget;
    QWidget* m_mainMenu;
    QWidget* m_pauseMenu;
    QWidget* m_settingsMenu;
    GameScene* m_gameScene;
    UIManager* m_uiManager;
    
    // 状态管理
    GameState m_currentState;
    GameState m_previousState;
    
    // 窗口设置
    bool m_isFullscreen;
    int m_windowWidth;
    int m_windowHeight;
    
    // 定时器
    QTimer* m_updateTimer;
};