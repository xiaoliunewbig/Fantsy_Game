#include "MainWindow.h"
#include "../core/GameEngine.h"
#include "../utils/Logger.h"
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyle>
#include <QScreen>
#include <QMessageBox>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_stackedWidget(new QStackedWidget(this))
    , m_mainMenu(nullptr)
    , m_pauseMenu(nullptr)
    , m_settingsMenu(nullptr)
    , m_gameScene(new GameScene(this))
    , m_uiManager(new UIManager(this))
    , m_currentState(GameState::MENU)
    , m_previousState(GameState::MENU)
    , m_isFullscreen(false)
    , m_windowWidth(1280)
    , m_windowHeight(720)
    , m_updateTimer(new QTimer(this)) {
    
    setWindowTitle("幻境传说");
    setMinimumSize(1280, 720);
    resize(m_windowWidth, m_windowHeight);
    centerWindow();
    
    setupUI();
    setupConnections();
    applyStyles();
    
    // 启动更新定时器
    m_updateTimer->start(16); // ~60 FPS
    
    Logger::info("MainWindow initialized successfully");
}

MainWindow::~MainWindow() {
    Logger::info("MainWindow destroyed");
}

void MainWindow::setupUI() {
    setCentralWidget(m_stackedWidget);
    
    createMainMenu();
    createPauseMenu();
    createSettingsMenu();
    
    m_stackedWidget->addWidget(m_mainMenu);
    m_stackedWidget->addWidget(m_gameScene);
    m_stackedWidget->addWidget(m_pauseMenu);
    m_stackedWidget->addWidget(m_settingsMenu);
    
    showMainMenu();
}

void MainWindow::createMainMenu() {
    m_mainMenu = new QWidget();
    auto layout = new QVBoxLayout(m_mainMenu);
    
    // 标题
    auto titleLabel = new QLabel("幻境传说");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 48px; font-weight: bold; color: #FFD700; margin: 50px;");
    layout->addWidget(titleLabel);
    
    // 副标题
    auto subtitleLabel = new QLabel("Fantasy Legend");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet("font-size: 18px; color: #CCCCCC; margin-bottom: 50px;");
    layout->addWidget(subtitleLabel);
    
    layout->addStretch();
    
    // 按钮容器
    auto buttonContainer = new QWidget();
    auto buttonLayout = new QVBoxLayout(buttonContainer);
    buttonLayout->setSpacing(15);
    buttonLayout->setContentsMargins(100, 0, 100, 0);
    
    // 按钮
    auto newGameBtn = new QPushButton("新游戏");
    auto loadGameBtn = new QPushButton("加载游戏");
    auto settingsBtn = new QPushButton("设置");
    auto exitBtn = new QPushButton("退出");
    
    connect(newGameBtn, &QPushButton::clicked, this, &MainWindow::onNewGameClicked);
    connect(loadGameBtn, &QPushButton::clicked, this, &MainWindow::onLoadGameClicked);
    connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    connect(exitBtn, &QPushButton::clicked, this, &MainWindow::onExitClicked);
    
    buttonLayout->addWidget(newGameBtn);
    buttonLayout->addWidget(loadGameBtn);
    buttonLayout->addWidget(settingsBtn);
    buttonLayout->addWidget(exitBtn);
    
    layout->addWidget(buttonContainer);
    layout->addStretch();
    
    // 版权信息
    auto copyrightLabel = new QLabel("© 2024 Fantasy Legend. All rights reserved.");
    copyrightLabel->setAlignment(Qt::AlignCenter);
    copyrightLabel->setStyleSheet("font-size: 12px; color: #666666; margin: 20px;");
    layout->addWidget(copyrightLabel);
}

void MainWindow::createPauseMenu() {
    m_pauseMenu = new QWidget();
    auto layout = new QVBoxLayout(m_pauseMenu);
    
    // 半透明背景
    m_pauseMenu->setStyleSheet("background-color: rgba(0, 0, 0, 0.8);");
    
    // 暂停标题
    auto pauseLabel = new QLabel("游戏暂停");
    pauseLabel->setAlignment(Qt::AlignCenter);
    pauseLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #FFFFFF; margin: 50px;");
    layout->addWidget(pauseLabel);
    
    layout->addStretch();
    
    // 按钮容器
    auto buttonContainer = new QWidget();
    auto buttonLayout = new QVBoxLayout(buttonContainer);
    buttonLayout->setSpacing(15);
    buttonLayout->setContentsMargins(100, 0, 100, 0);
    
    // 按钮
    auto resumeBtn = new QPushButton("继续游戏");
    auto saveGameBtn = new QPushButton("保存游戏");
    auto loadGameBtn = new QPushButton("加载游戏");
    auto settingsBtn = new QPushButton("设置");
    auto mainMenuBtn = new QPushButton("返回主菜单");
    
    connect(resumeBtn, &QPushButton::clicked, this, &MainWindow::onResumeClicked);
    connect(saveGameBtn, &QPushButton::clicked, this, &MainWindow::onSaveGameClicked);
    connect(loadGameBtn, &QPushButton::clicked, this, &MainWindow::onLoadGameFromMenuClicked);
    connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    connect(mainMenuBtn, &QPushButton::clicked, this, &MainWindow::onBackToMainMenuClicked);
    
    buttonLayout->addWidget(resumeBtn);
    buttonLayout->addWidget(saveGameBtn);
    buttonLayout->addWidget(loadGameBtn);
    buttonLayout->addWidget(settingsBtn);
    buttonLayout->addWidget(mainMenuBtn);
    
    layout->addWidget(buttonContainer);
    layout->addStretch();
}

void MainWindow::createSettingsMenu() {
    m_settingsMenu = new QWidget();
    auto layout = new QVBoxLayout(m_settingsMenu);
    
    // 设置标题
    auto titleLabel = new QLabel("设置");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 36px; font-weight: bold; color: #FFFFFF; margin: 30px;");
    layout->addWidget(titleLabel);
    
    // 设置内容
    auto settingsContainer = new QWidget();
    auto settingsLayout = new QVBoxLayout(settingsContainer);
    
    // 这里可以添加各种设置选项
    auto graphicsLabel = new QLabel("图形设置");
    graphicsLabel->setStyleSheet("font-size: 18px; color: #FFFFFF; margin: 10px;");
    settingsLayout->addWidget(graphicsLabel);
    
    auto audioLabel = new QLabel("音频设置");
    audioLabel->setStyleSheet("font-size: 18px; color: #FFFFFF; margin: 10px;");
    settingsLayout->addWidget(audioLabel);
    
    auto controlLabel = new QLabel("控制设置");
    controlLabel->setStyleSheet("font-size: 18px; color: #FFFFFF; margin: 10px;");
    settingsLayout->addWidget(controlLabel);
    
    layout->addWidget(settingsContainer);
    layout->addStretch();
    
    // 返回按钮
    auto backBtn = new QPushButton("返回");
    backBtn->setStyleSheet("QPushButton { font-size: 16px; padding: 10px; margin: 20px; }");
    connect(backBtn, &QPushButton::clicked, [this]() {
        if (m_previousState == GameState::MENU) {
            showMainMenu();
        } else {
            showPauseMenu();
        }
    });
    layout->addWidget(backBtn);
}

void MainWindow::setupConnections() {
    // 连接游戏引擎信号
    if (GameEngine::instance()) {
        connect(GameEngine::instance(), &GameEngine::gameStateChanged,
                this, &MainWindow::onGameStateChanged);
    }
    
    // 连接UI管理器信号
    connect(m_uiManager, &UIManager::uiStateChanged,
            this, [this](const QString& state) {
        Logger::debug(QString("UI state changed: %1").arg(state));
    });
}

void MainWindow::applyStyles() {
    // 主窗口样式
    setStyleSheet(R"(
        QMainWindow {
            background-color: #1a1a1a;
        }
    )");
    
    // 按钮样式
    QString buttonStyle = R"(
        QPushButton {
            font-size: 18px;
            padding: 15px;
            margin: 5px;
            background-color: #4A4A4A;
            color: white;
            border: 2px solid #666;
            border-radius: 8px;
            min-width: 200px;
        }
        QPushButton:hover {
            background-color: #666;
            border-color: #888;
        }
        QPushButton:pressed {
            background-color: #333;
            border-color: #555;
        }
        QPushButton:disabled {
            background-color: #2a2a2a;
            color: #666;
            border-color: #444;
        }
    )";
    
    // 应用按钮样式到所有按钮
    QList<QPushButton*> buttons = findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        button->setStyleSheet(buttonStyle);
    }
}

void MainWindow::showMainMenu() {
    m_currentState = GameState::MENU;
    m_stackedWidget->setCurrentWidget(m_mainMenu);
    Logger::info("Showing main menu");
}

void MainWindow::showGameScene() {
    m_currentState = GameState::PLAYING;
    m_stackedWidget->setCurrentWidget(m_gameScene);
    m_gameScene->setFocus();
    Logger::info("Showing game scene");
}

void MainWindow::showPauseMenu() {
    m_previousState = m_currentState;
    m_currentState = GameState::PAUSED;
    m_stackedWidget->setCurrentWidget(m_pauseMenu);
    Logger::info("Showing pause menu");
}

void MainWindow::showSettingsMenu() {
    m_previousState = m_currentState;
    m_currentState = GameState::MENU;
    m_stackedWidget->setCurrentWidget(m_settingsMenu);
    Logger::info("Showing settings menu");
}

void MainWindow::showInventory() {
    m_previousState = m_currentState;
    m_currentState = GameState::INVENTORY;
    // 这里可以显示背包界面
    Logger::info("Showing inventory");
}

void MainWindow::showDialogue() {
    m_previousState = m_currentState;
    m_currentState = GameState::DIALOGUE;
    // 这里可以显示对话界面
    Logger::info("Showing dialogue");
}

void MainWindow::setFullscreen(bool fullscreen) {
    m_isFullscreen = fullscreen;
    if (fullscreen) {
        showFullScreen();
    } else {
        showNormal();
        resize(m_windowWidth, m_windowHeight);
        centerWindow();
    }
}

void MainWindow::setWindowSize(int width, int height) {
    m_windowWidth = width;
    m_windowHeight = height;
    if (!m_isFullscreen) {
        resize(width, height);
        centerWindow();
    }
}

void MainWindow::centerWindow() {
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    Logger::info("Application closing");
    
    // 保存游戏状态
    if (GameEngine::instance()) {
        GameEngine::instance()->saveGame("autosave");
    }
    
    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Escape:
            onEscapePressed();
            break;
        case Qt::Key_Space:
            onSpacePressed();
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            onEnterPressed();
            break;
        case Qt::Key_F11:
            setFullscreen(!m_isFullscreen);
            break;
        default:
            QMainWindow::keyPressEvent(event);
            break;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent* event) {
    QMainWindow::keyReleaseEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    if (!m_isFullscreen) {
        m_windowWidth = width();
        m_windowHeight = height();
    }
}

void MainWindow::onGameStateChanged(GameState state) {
    m_currentState = state;
    Logger::info(QString("Game state changed to: %1").arg(static_cast<int>(state)));
}

void MainWindow::onNewGameClicked() {
    Logger::info("New game button clicked");
    
    // 这里可以添加新游戏确认对话框
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "新游戏", "确定要开始新游戏吗？\n当前游戏进度将会丢失。",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        if (GameEngine::instance()) {
            GameEngine::instance()->startGame();
        }
        showGameScene();
    }
}

void MainWindow::onLoadGameClicked() {
    Logger::info("Load game button clicked");
    // 这里可以显示存档选择界面
    showGameScene();
}

void MainWindow::onSettingsClicked() {
    Logger::info("Settings button clicked");
    showSettingsMenu();
}

void MainWindow::onExitClicked() {
    Logger::info("Exit button clicked");
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "退出游戏", "确定要退出游戏吗？",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        QApplication::quit();
    }
}

void MainWindow::onResumeClicked() {
    Logger::info("Resume button clicked");
    if (m_previousState == GameState::PLAYING) {
        showGameScene();
    } else {
        showMainMenu();
    }
}

void MainWindow::onSaveGameClicked() {
    Logger::info("Save game button clicked");
    if (GameEngine::instance()) {
        GameEngine::instance()->saveGame("manual_save");
        QMessageBox::information(this, "保存游戏", "游戏已保存！");
    }
}

void MainWindow::onLoadGameFromMenuClicked() {
    Logger::info("Load game from menu button clicked");
    // 这里可以显示存档选择界面
    showGameScene();
}

void MainWindow::onBackToMainMenuClicked() {
    Logger::info("Back to main menu button clicked");
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "返回主菜单", "确定要返回主菜单吗？\n当前游戏进度将会丢失。",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        showMainMenu();
    }
}

void MainWindow::onEscapePressed() {
    switch (m_currentState) {
        case GameState::PLAYING:
            showPauseMenu();
            break;
        case GameState::PAUSED:
            onResumeClicked();
            break;
        case GameState::INVENTORY:
        case GameState::DIALOGUE:
            if (m_previousState == GameState::PLAYING) {
                showGameScene();
            } else {
                showMainMenu();
            }
            break;
        default:
            break;
    }
}

void MainWindow::onSpacePressed() {
    if (m_currentState == GameState::PLAYING) {
        // 在游戏场景中，空格键可以暂停游戏
        showPauseMenu();
    }
}

void MainWindow::onEnterPressed() {
    // 回车键通常用于确认选择
    if (m_currentState == GameState::PLAYING) {
        // 在游戏场景中，回车键可以打开菜单
        showPauseMenu();
    }
}