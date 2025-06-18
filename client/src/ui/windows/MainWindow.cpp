/**
 * @file MainWindow.cpp
 * @brief 幻境传说主窗口类实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "ui/windows/MainWindow.h"
#include "ui/scenes/GameScene.h"
#include "ui/components/MainMenu.h"
#include "ui/components/PauseMenu.h"
#include "ui/managers/UIManager.h"
#include "controllers/GameController.h"
#include "network/NetworkManager.h"
#include "utils/Logger.h"
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QScreen>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QShowEvent>
#include <QResizeEvent>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QStandardPaths>
#include <QElapsedTimer>

namespace Fantasy {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_stackedWidget(nullptr)
    , m_mainMenu(nullptr)
    , m_gameScene(nullptr)
    , m_pauseMenu(nullptr)
    , m_settingsWidget(nullptr)
    , m_loadingWidget(nullptr)
    , m_currentState(GameState::MAIN_MENU)
    , m_previousState(GameState::MAIN_MENU)
    , m_isFullscreen(false)
    , m_updateTimer(nullptr)
    , m_fpsTimer(nullptr)
    , m_settings(nullptr)
    , m_frameCount(0)
    , m_lastFpsTime(0.0)
    , m_currentFps(0.0)
{
    CLIENT_LOG_INFO("Creating MainWindow");
    
    // 初始化设置
    m_settings = new QSettings("FantasyLegend", "Client", this);
    
    // 设置窗口属性
    setWindowTitle("幻境传说 - Fantasy Legend");
    setMinimumSize(1024, 768);
    resize(1280, 720);
    
    // 初始化UI
    setupUI();
    setupConnections();
    
    // 加载设置
    loadSettings();
    applySettings();
    
    // 应用样式
    applyStylesheet();
    
    // 初始化定时器
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(16); // ~60 FPS
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::update);
    
    m_fpsTimer = new QTimer(this);
    m_fpsTimer->setInterval(1000); // 1秒更新一次FPS
    connect(m_fpsTimer, &QTimer::timeout, [this]() {
        m_currentFps = m_frameCount;
        m_frameCount = 0;
        statusBar()->showMessage(QString("FPS: %1").arg(m_currentFps));
    });
    
    // 居中显示窗口
    centerOnScreen();
    
    // 显示主菜单
    showMainMenu();
    
    CLIENT_LOG_INFO("MainWindow created successfully");
}

MainWindow::~MainWindow()
{
    CLIENT_LOG_INFO("Destroying MainWindow");
    
    // 保存设置
    saveSettings();
    
    // 停止定时器
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
    if (m_fpsTimer) {
        m_fpsTimer->stop();
    }
    
    CLIENT_LOG_INFO("MainWindow destroyed");
}

void MainWindow::showMainMenu()
{
    CLIENT_LOG_DEBUG("Showing main menu");
    m_previousState = m_currentState;
    m_currentState = GameState::MAIN_MENU;
    
    if (m_stackedWidget) {
        m_stackedWidget->setCurrentWidget(m_mainMenu);
    }
    
    // 停止游戏更新
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
    if (m_fpsTimer) {
        m_fpsTimer->stop();
    }
    
    emit onGameStateChanged(m_currentState);
}

void MainWindow::showGameScene()
{
    CLIENT_LOG_DEBUG("Showing game scene");
    m_previousState = m_currentState;
    m_currentState = GameState::PLAYING;
    
    if (m_stackedWidget) {
        m_stackedWidget->setCurrentWidget(m_gameScene);
    }
    
    // 启动游戏更新
    if (m_updateTimer) {
        m_updateTimer->start();
    }
    if (m_fpsTimer) {
        m_fpsTimer->start();
    }
    
    emit gameStarted();
    emit onGameStateChanged(m_currentState);
}

void MainWindow::showPauseMenu()
{
    CLIENT_LOG_DEBUG("Showing pause menu");
    m_previousState = m_currentState;
    m_currentState = GameState::PAUSED;
    
    if (m_stackedWidget) {
    m_stackedWidget->setCurrentWidget(m_pauseMenu);
    }
    
    // 暂停游戏更新
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
    if (m_fpsTimer) {
        m_fpsTimer->stop();
    }
    
    emit gamePaused();
    emit onGameStateChanged(m_currentState);
}

void MainWindow::showSettings()
{
    CLIENT_LOG_DEBUG("Showing settings");
    m_previousState = m_currentState;
    m_currentState = GameState::SETTINGS;
    
    if (m_stackedWidget) {
        m_stackedWidget->setCurrentWidget(m_settingsWidget);
    }
    
    emit onGameStateChanged(m_currentState);
}

void MainWindow::showLoadingScreen()
{
    CLIENT_LOG_DEBUG("Showing loading screen");
    m_previousState = m_currentState;
    m_currentState = GameState::LOADING;
    
    if (m_stackedWidget) {
        m_stackedWidget->setCurrentWidget(m_loadingWidget);
    }
    
    emit onGameStateChanged(m_currentState);
}

void MainWindow::setFullscreen(bool fullscreen)
{
    if (m_isFullscreen == fullscreen) {
        return;
    }
    
    m_isFullscreen = fullscreen;
    
    if (fullscreen) {
        // 保存当前窗口状态
        m_normalSize = size();
        m_normalPosition = pos();
        
        // 进入全屏
        showFullScreen();
        CLIENT_LOG_INFO("Entered fullscreen mode");
    } else {
        // 退出全屏
        showNormal();
        resize(m_normalSize);
        move(m_normalPosition);
        CLIENT_LOG_INFO("Exited fullscreen mode");
    }
}

void MainWindow::toggleFullscreen()
{
    setFullscreen(!m_isFullscreen);
}

void MainWindow::centerOnScreen()
{
    QScreen* screen = QApplication::primaryScreen();
    if (screen) {
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}
}

void MainWindow::loadSettings()
{
    CLIENT_LOG_DEBUG("Loading settings");
    
    // 窗口设置
    m_isFullscreen = m_settings->value("window/fullscreen", false).toBool();
    m_normalSize = m_settings->value("window/size", QSize(1280, 720)).toSize();
    m_normalPosition = m_settings->value("window/position", QPoint(100, 100)).toPoint();
    
    // 游戏设置
    // TODO: 加载游戏相关设置
    
    CLIENT_LOG_DEBUG("Settings loaded");
}

void MainWindow::saveSettings()
{
    CLIENT_LOG_DEBUG("Saving settings");
    
    // 窗口设置
    m_settings->setValue("window/fullscreen", m_isFullscreen);
    m_settings->setValue("window/size", m_normalSize);
    m_settings->setValue("window/position", m_normalPosition);
    
    // 游戏设置
    // TODO: 保存游戏相关设置
    
    m_settings->sync();
    CLIENT_LOG_DEBUG("Settings saved");
}

void MainWindow::applySettings()
{
    CLIENT_LOG_DEBUG("Applying settings");
    
    // 应用窗口设置
    if (m_isFullscreen) {
        showFullScreen();
    } else {
        showNormal();
        resize(m_normalSize);
        move(m_normalPosition);
    }
    
    // 应用游戏设置
    // TODO: 应用游戏相关设置
    
    CLIENT_LOG_DEBUG("Settings applied");
}

void MainWindow::onGameStateChanged(GameState newState)
{
    CLIENT_LOG_DEBUG("Game state changed to: %d", static_cast<int>(newState));
    
    // 更新状态栏
    QString stateText;
    switch (newState) {
        case GameState::MAIN_MENU:
            stateText = "主菜单";
            break;
        case GameState::PLAYING:
            stateText = "游戏中";
            break;
        case GameState::PAUSED:
            stateText = "暂停";
            break;
        case GameState::SETTINGS:
            stateText = "设置";
            break;
        case GameState::LOADING:
            stateText = "加载中";
            break;
        case GameState::EXIT:
            stateText = "退出";
            break;
    }
    
    statusBar()->showMessage(QString("状态: %1 | FPS: %2").arg(stateText).arg(m_currentFps));
}

void MainWindow::onNewGameClicked()
{
    CLIENT_LOG_INFO("New game clicked");
    showLoadingScreen();
    
    // TODO: 初始化新游戏
    // 这里可以添加游戏初始化逻辑
    
    // 延迟切换到游戏场景
    QTimer::singleShot(1000, this, &MainWindow::showGameScene);
}

void MainWindow::onLoadGameClicked()
{
    CLIENT_LOG_INFO("Load game clicked");
    showLoadingScreen();
    
    // TODO: 加载游戏存档
    // 这里可以添加存档加载逻辑
    
    // 延迟切换到游戏场景
    QTimer::singleShot(1000, this, &MainWindow::showGameScene);
}

void MainWindow::onSettingsClicked()
{
    CLIENT_LOG_INFO("Settings clicked");
    showSettings();
}

void MainWindow::onExitClicked()
{
    CLIENT_LOG_INFO("Exit clicked");
    close();
}

void MainWindow::onResumeClicked()
{
    CLIENT_LOG_INFO("Resume clicked");
    if (m_previousState == GameState::PLAYING) {
        showGameScene();
    } else {
        showMainMenu();
    }
}

void MainWindow::onBackToMainMenuClicked()
{
    CLIENT_LOG_INFO("Back to main menu clicked");
    showMainMenu();
}

void MainWindow::onWindowStateChanged(Qt::WindowStates state)
{
    CLIENT_LOG_DEBUG("Window state changed: %d", static_cast<int>(state));
    
    if (state & Qt::WindowFullScreen) {
        m_isFullscreen = true;
    } else {
        m_isFullscreen = false;
    }
}

void MainWindow::onCloseRequested()
{
    CLIENT_LOG_INFO("Close requested");
    close();
}

void MainWindow::update()
{
    m_frameCount++;
    
    // 更新游戏逻辑
    if (m_gameController) {
        m_gameController->update();
    }
    
    // 更新UI
    if (m_uiManager) {
        m_uiManager->update();
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    CLIENT_LOG_INFO("Close event received");
    
    // 保存设置
    saveSettings();
    
    // 确认退出
    if (m_currentState == GameState::PLAYING) {
    QMessageBox::StandardButton reply = QMessageBox::question(
            this, "确认退出", "游戏正在进行中，确定要退出吗？",
        QMessageBox::Yes | QMessageBox::No
    );
    
        if (reply == QMessageBox::No) {
            event->ignore();
            return;
        }
    }
    
    emit gameExited();
    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
        case Qt::Key_Escape:
            if (m_currentState == GameState::PLAYING) {
            showPauseMenu();
            } else if (m_currentState == GameState::PAUSED) {
                onResumeClicked();
            }
            break;
            
        case Qt::Key_F11:
            toggleFullscreen();
            break;
            
        case Qt::Key_F1:
            // TODO: 显示帮助
            break;
            
        default:
            QMainWindow::keyPressEvent(event);
            break;
    }
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    
    // 保存窗口大小
    if (!m_isFullscreen) {
        m_normalSize = size();
    }
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    
    // 窗口显示后的初始化
    if (m_currentState == GameState::MAIN_MENU) {
        // 确保主菜单可见
        if (m_stackedWidget) {
            m_stackedWidget->setCurrentWidget(m_mainMenu);
        }
    }
}

void MainWindow::setupUI()
{
    CLIENT_LOG_DEBUG("Setting up UI");
    
    // 创建中央组件
    createCentralWidget();
    
    // 创建菜单栏
    createMenuBar();
    
    // 创建状态栏
    createStatusBar();
    
    CLIENT_LOG_DEBUG("UI setup completed");
}

void MainWindow::createMenuBar()
{
    QMenuBar* menuBar = this->menuBar();
    
    // 文件菜单
    QMenu* fileMenu = menuBar->addMenu("文件(&F)");
    fileMenu->addAction("新建游戏(&N)", this, &MainWindow::onNewGameClicked, QKeySequence::New);
    fileMenu->addAction("加载游戏(&L)", this, &MainWindow::onLoadGameClicked, QKeySequence::Open);
    fileMenu->addSeparator();
    fileMenu->addAction("设置(&S)", this, &MainWindow::onSettingsClicked, QKeySequence::Preferences);
    fileMenu->addSeparator();
    fileMenu->addAction("退出(&X)", this, &MainWindow::onExitClicked, QKeySequence::Quit);
    
    // 游戏菜单
    QMenu* gameMenu = menuBar->addMenu("游戏(&G)");
    gameMenu->addAction("暂停(&P)", this, &MainWindow::showPauseMenu, QKeySequence("P"));
    gameMenu->addAction("返回主菜单(&M)", this, &MainWindow::onBackToMainMenuClicked);
    
    // 视图菜单
    QMenu* viewMenu = menuBar->addMenu("视图(&V)");
    viewMenu->addAction("全屏(&F)", this, &MainWindow::toggleFullscreen, QKeySequence("F11"));
    
    // 帮助菜单
    QMenu* helpMenu = menuBar->addMenu("帮助(&H)");
    helpMenu->addAction("关于(&A)", [this]() {
        QMessageBox::about(this, "关于", "幻境传说 v1.0.0\n\n一个充满魔法的冒险世界");
    });
}

void MainWindow::createStatusBar()
{
    QStatusBar* statusBar = this->statusBar();
    statusBar->showMessage("就绪");
}

void MainWindow::createCentralWidget()
{
    // 创建堆叠窗口组件
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);
    
    // 创建主菜单
    m_mainMenu = new Fantasy::MainMenu(this);
    m_stackedWidget->addWidget(m_mainMenu);
    
    // 创建游戏场景
    m_gameScene = new Fantasy::GameScene(this);
    m_stackedWidget->addWidget(m_gameScene);
    
    // 创建暂停菜单
    m_pauseMenu = new Fantasy::PauseMenu(this);
    m_stackedWidget->addWidget(m_pauseMenu);
    
    // 创建设置界面
    m_settingsWidget = new QWidget(this);
    m_stackedWidget->addWidget(m_settingsWidget);
    
    // 创建加载界面
    m_loadingWidget = new QWidget(this);
    m_stackedWidget->addWidget(m_loadingWidget);
    
    // 创建管理器
    m_uiManager = std::make_unique<Fantasy::UIManager>(this);
    m_gameController = std::make_unique<Fantasy::GameController>(this);
    m_networkManager = std::make_unique<Fantasy::NetworkManager>(this);
}

void MainWindow::setupConnections()
{
    CLIENT_LOG_DEBUG("Setting up connections");
    
    // 连接窗口状态变化信号
    connect(this, &QMainWindow::windowStateChanged, 
            this, &MainWindow::onWindowStateChanged);
    
    // 连接主菜单信号
    if (m_mainMenu) {
        connect(m_mainMenu, &Fantasy::MainMenu::newGameClicked, this, &MainWindow::onNewGameClicked);
        connect(m_mainMenu, &Fantasy::MainMenu::loadGameClicked, this, &MainWindow::onLoadGameClicked);
        connect(m_mainMenu, &Fantasy::MainMenu::settingsClicked, this, &MainWindow::onSettingsClicked);
        connect(m_mainMenu, &Fantasy::MainMenu::exitClicked, this, &MainWindow::onExitClicked);
    }
    
    // 连接暂停菜单信号
    if (m_pauseMenu) {
        connect(m_pauseMenu, &Fantasy::PauseMenu::resumeClicked, this, &MainWindow::onResumeClicked);
        connect(m_pauseMenu, &Fantasy::PauseMenu::backToMainMenuClicked, this, &MainWindow::onBackToMainMenuClicked);
        connect(m_pauseMenu, &Fantasy::PauseMenu::settingsClicked, this, &MainWindow::onSettingsClicked);
    }
    
    CLIENT_LOG_DEBUG("Connections setup completed");
}

void MainWindow::applyStylesheet()
{
    CLIENT_LOG_DEBUG("Applying stylesheet");
    
    // 尝试加载自定义样式表
    QString stylesheetPath = "assets/ui/styles/main.qss";
    if (QFile::exists(stylesheetPath)) {
        loadStylesheet(stylesheetPath);
    } else {
        // 使用默认样式
        QString defaultStyle = R"(
            QMainWindow {
                background-color: #2a2a2a;
                color: #ffffff;
            }
            
            QMenuBar {
                background-color: #3a3a3a;
                color: #ffffff;
                border-bottom: 1px solid #555555;
            }
            
            QMenuBar::item {
                background-color: transparent;
                padding: 5px 10px;
            }
            
            QMenuBar::item:selected {
                background-color: #555555;
            }
            
            QMenu {
                background-color: #3a3a3a;
                color: #ffffff;
                border: 1px solid #555555;
            }
            
            QMenu::item {
                padding: 5px 20px;
            }
            
            QMenu::item:selected {
                background-color: #555555;
            }
            
            QStatusBar {
                background-color: #3a3a3a;
                color: #ffffff;
                border-top: 1px solid #555555;
            }
        )";
        
        setStyleSheet(defaultStyle);
    }
    
    CLIENT_LOG_DEBUG("Stylesheet applied");
}

void MainWindow::loadStylesheet(const QString& filename)
{
    QFile file(filename);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        QString stylesheet = stream.readAll();
        setStyleSheet(stylesheet);
        CLIENT_LOG_INFO("Loaded stylesheet from: %s", filename.toUtf8().constData());
    } else {
        CLIENT_LOG_WARN("Failed to load stylesheet from: %s", filename.toUtf8().constData());
    }
}

void MainWindow::logMessage(const QString& message)
{
    CLIENT_LOG_INFO("%s", message.toUtf8().constData());
}

void MainWindow::showError(const QString& title, const QString& message)
{
    QMessageBox::critical(this, title, message);
    CLIENT_LOG_ERROR("%s: %s", title.toUtf8().constData(), message.toUtf8().constData());
}

} // namespace Fantasy