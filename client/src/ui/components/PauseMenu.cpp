/**
 * @file PauseMenu.cpp
 * @brief 暂停菜单组件实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "ui/components/PauseMenu.h"
#include "utils/Logger.h"
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QKeyEvent>
#include <QShowEvent>
#include <QHideEvent>

namespace Fantasy {

PauseMenu::PauseMenu(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_buttonContainer(nullptr)
    , m_buttonLayout(nullptr)
    , m_resumeButton(nullptr)
    , m_saveGameButton(nullptr)
    , m_loadGameButton(nullptr)
    , m_settingsButton(nullptr)
    , m_backToMainMenuButton(nullptr)
    , m_fadeInAnimation(nullptr)
    , m_fadeOutAnimation(nullptr)
{
    CLIENT_LOG_DEBUG("Creating PauseMenu");
    
    // 设置窗口属性
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_TranslucentBackground);
    
    // 初始化UI
    setupUI();
    setupAnimations();
    applyStyles();
    
    CLIENT_LOG_DEBUG("PauseMenu created successfully");
}

PauseMenu::~PauseMenu()
{
    CLIENT_LOG_DEBUG("Destroying PauseMenu");
}

void PauseMenu::showMenu()
{
    CLIENT_LOG_DEBUG("Showing pause menu");
    
    if (m_fadeInAnimation) {
        m_fadeInAnimation->start();
    }
}

void PauseMenu::hideMenu()
{
    CLIENT_LOG_DEBUG("Hiding pause menu");
    
    if (m_fadeOutAnimation) {
        m_fadeOutAnimation->start();
    }
}

void PauseMenu::onResumeButtonClicked()
{
    CLIENT_LOG_INFO("Resume button clicked");
    emit resumeClicked();
}

void PauseMenu::onSaveGameButtonClicked()
{
    CLIENT_LOG_INFO("Save game button clicked");
    emit saveGameClicked();
}

void PauseMenu::onLoadGameButtonClicked()
{
    CLIENT_LOG_INFO("Load game button clicked");
    emit loadGameClicked();
}

void PauseMenu::onSettingsButtonClicked()
{
    CLIENT_LOG_INFO("Settings button clicked");
    emit settingsClicked();
}

void PauseMenu::onBackToMainMenuButtonClicked()
{
    CLIENT_LOG_INFO("Back to main menu button clicked");
    emit backToMainMenuClicked();
}

void PauseMenu::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制半透明背景
    painter.fillRect(rect(), QColor(0, 0, 0, 180));
    
    // 绘制装饰性边框
    painter.setPen(QPen(QColor(255, 255, 255, 50), 2));
    painter.drawRect(rect().adjusted(10, 10, -10, -10));
}

void PauseMenu::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    showMenu();
}

void PauseMenu::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    hideMenu();
}

void PauseMenu::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
        case Qt::Key_Escape:
        case Qt::Key_P:
            // 继续游戏
            onResumeButtonClicked();
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Space:
            // 激活当前选中的按钮
            break;
        default:
            QWidget::keyPressEvent(event);
            break;
    }
}

void PauseMenu::setupUI()
{
    CLIENT_LOG_DEBUG("Setting up PauseMenu UI");
    
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(50, 50, 50, 50);
    m_mainLayout->setSpacing(30);
    
    // 创建标题
    createTitle();
    
    // 添加弹性空间
    m_mainLayout->addStretch();
    
    // 创建按钮
    createButtons();
    
    // 添加弹性空间
    m_mainLayout->addStretch();
    
    CLIENT_LOG_DEBUG("PauseMenu UI setup completed");
}

void PauseMenu::createTitle()
{
    // 创建标题标签
    m_titleLabel = new QLabel("游戏暂停", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setObjectName("pauseTitleLabel");
    
    // 添加到布局
    m_mainLayout->addWidget(m_titleLabel);
}

void PauseMenu::createButtons()
{
    // 创建按钮容器
    m_buttonContainer = new QWidget(this);
    m_buttonLayout = new QVBoxLayout(m_buttonContainer);
    m_buttonLayout->setSpacing(15);
    m_buttonLayout->setContentsMargins(100, 0, 100, 0);
    
    // 创建按钮
    m_resumeButton = new QPushButton("继续游戏", this);
    m_resumeButton->setObjectName("pauseMenuButton");
    m_resumeButton->setProperty("buttonType", "resume");
    
    m_saveGameButton = new QPushButton("保存游戏", this);
    m_saveGameButton->setObjectName("pauseMenuButton");
    m_saveGameButton->setProperty("buttonType", "saveGame");
    
    m_loadGameButton = new QPushButton("加载游戏", this);
    m_loadGameButton->setObjectName("pauseMenuButton");
    m_loadGameButton->setProperty("buttonType", "loadGame");
    
    m_settingsButton = new QPushButton("设置", this);
    m_settingsButton->setObjectName("pauseMenuButton");
    m_settingsButton->setProperty("buttonType", "settings");
    
    m_backToMainMenuButton = new QPushButton("返回主菜单", this);
    m_backToMainMenuButton->setObjectName("pauseMenuButton");
    m_backToMainMenuButton->setProperty("buttonType", "backToMainMenu");
    
    // 连接信号
    connect(m_resumeButton, &QPushButton::clicked, this, &PauseMenu::onResumeButtonClicked);
    connect(m_saveGameButton, &QPushButton::clicked, this, &PauseMenu::onSaveGameButtonClicked);
    connect(m_loadGameButton, &QPushButton::clicked, this, &PauseMenu::onLoadGameButtonClicked);
    connect(m_settingsButton, &QPushButton::clicked, this, &PauseMenu::onSettingsButtonClicked);
    connect(m_backToMainMenuButton, &QPushButton::clicked, this, &PauseMenu::onBackToMainMenuButtonClicked);
    
    // 添加到布局
    m_buttonLayout->addWidget(m_resumeButton);
    m_buttonLayout->addWidget(m_saveGameButton);
    m_buttonLayout->addWidget(m_loadGameButton);
    m_buttonLayout->addWidget(m_settingsButton);
    m_buttonLayout->addWidget(m_backToMainMenuButton);
    
    // 添加到主布局
    m_mainLayout->addWidget(m_buttonContainer);
}

void PauseMenu::setupAnimations()
{
    CLIENT_LOG_DEBUG("Setting up PauseMenu animations");
    
    // 创建淡入动画
    m_fadeInAnimation = new QPropertyAnimation(this, "windowOpacity", this);
    m_fadeInAnimation->setDuration(300);
    m_fadeInAnimation->setStartValue(0.0);
    m_fadeInAnimation->setEndValue(1.0);
    m_fadeInAnimation->setEasingCurve(QEasingCurve::OutQuad);
    
    // 创建淡出动画
    m_fadeOutAnimation = new QPropertyAnimation(this, "windowOpacity", this);
    m_fadeOutAnimation->setDuration(300);
    m_fadeOutAnimation->setStartValue(1.0);
    m_fadeOutAnimation->setEndValue(0.0);
    m_fadeOutAnimation->setEasingCurve(QEasingCurve::InQuad);
    
    CLIENT_LOG_DEBUG("PauseMenu animations setup completed");
}

void PauseMenu::applyStyles()
{
    CLIENT_LOG_DEBUG("Applying PauseMenu styles");
    
    // 按钮样式
    m_buttonStyle = R"(
        QPushButton#pauseMenuButton {
            font-size: 16px;
            font-weight: bold;
            padding: 12px 25px;
            margin: 3px;
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #4A4A4A, stop:1 #2A2A2A);
            color: white;
            border: 2px solid #666666;
            border-radius: 8px;
            min-width: 180px;
            min-height: 45px;
        }
        
        QPushButton#pauseMenuButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #666666, stop:1 #4A4A4A);
            border-color: #888888;
        }
        
        QPushButton#pauseMenuButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #333333, stop:1 #1A1A1A);
            border-color: #555555;
        }
        
        QPushButton#pauseMenuButton:disabled {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #2A2A2A, stop:1 #1A1A1A);
            color: #666666;
            border-color: #444444;
        }
    )";
    
    // 标题样式
    m_titleStyle = R"(
        QLabel#pauseTitleLabel {
            font-size: 36px;
            font-weight: bold;
            color: #FFFFFF;
            margin: 20px;
            text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.8);
        }
    )";
    
    // 应用样式
    setStyleSheet(m_buttonStyle + m_titleStyle);
    
    // 添加阴影效果
    QGraphicsDropShadowEffect* titleShadow = new QGraphicsDropShadowEffect(this);
    titleShadow->setBlurRadius(15);
    titleShadow->setColor(QColor(0, 0, 0, 100));
    titleShadow->setOffset(2, 2);
    m_titleLabel->setGraphicsEffect(titleShadow);
    
    CLIENT_LOG_DEBUG("PauseMenu styles applied");
}

} // namespace Fantasy 