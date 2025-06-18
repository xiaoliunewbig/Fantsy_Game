/**
 * @file MainMenu.cpp
 * @brief 主菜单组件实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "ui/components/MainMenu.h"
#include "utils/Logger.h"
#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QTimer>
#include <QKeyEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QApplication>
#include <QScreen>

namespace Fantasy {

MainMenu::MainMenu(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_titleLabel(nullptr)
    , m_subtitleLabel(nullptr)
    , m_buttonContainer(nullptr)
    , m_buttonLayout(nullptr)
    , m_copyrightLabel(nullptr)
    , m_newGameButton(nullptr)
    , m_loadGameButton(nullptr)
    , m_settingsButton(nullptr)
    , m_exitButton(nullptr)
    , m_titleAnimation(nullptr)
    , m_subtitleAnimation(nullptr)
    , m_buttonAnimationTimer(nullptr)
    , m_backgroundAnimationTimer(nullptr)
    , m_currentButtonIndex(0)
    , m_isAnimating(false)
    , m_backgroundOffset(0.0)
{
    CLIENT_LOG_DEBUG("Creating MainMenu");
    
    // 设置窗口属性
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_TranslucentBackground);
    
    // 初始化UI
    setupUI();
    setupAnimations();
    applyStyles();
    
    CLIENT_LOG_DEBUG("MainMenu created successfully");
}

MainMenu::~MainMenu()
{
    CLIENT_LOG_DEBUG("Destroying MainMenu");
    
    // 停止动画
    stopBackgroundAnimation();
}

void MainMenu::showMenu()
{
    CLIENT_LOG_DEBUG("Showing main menu");
    
    // 启动背景动画
    startBackgroundAnimation();
    
    // 开始按钮动画
    animateButtons();
}

void MainMenu::hideMenu()
{
    CLIENT_LOG_DEBUG("Hiding main menu");
    
    // 停止背景动画
    stopBackgroundAnimation();
    
    // 停止按钮动画
    if (m_buttonAnimationTimer) {
        m_buttonAnimationTimer->stop();
    }
}

void MainMenu::animateButtons()
{
    CLIENT_LOG_DEBUG("Starting button animations");
    
    m_currentButtonIndex = 0;
    m_isAnimating = true;
    
    // 重置按钮位置
    QList<QPushButton*> buttons = {m_newGameButton, m_loadGameButton, m_settingsButton, m_exitButton};
    for (QPushButton* button : buttons) {
        if (button) {
            button->setGeometry(button->x() - 200, button->y(), button->width(), button->height());
            button->setOpacity(0.0);
        }
    }
    
    // 开始动画定时器
    if (m_buttonAnimationTimer) {
        m_buttonAnimationTimer->start(100); // 每100ms动画一个按钮
    }
}

void MainMenu::onNewGameButtonClicked()
{
    CLIENT_LOG_INFO("New game button clicked");
    emit newGameClicked();
}

void MainMenu::onLoadGameButtonClicked()
{
    CLIENT_LOG_INFO("Load game button clicked");
    emit loadGameClicked();
}

void MainMenu::onSettingsButtonClicked()
{
    CLIENT_LOG_INFO("Settings button clicked");
    emit settingsClicked();
}

void MainMenu::onExitButtonClicked()
{
    CLIENT_LOG_INFO("Exit button clicked");
    emit exitClicked();
}

void MainMenu::onButtonAnimationFinished()
{
    m_currentButtonIndex++;
    
    if (m_currentButtonIndex >= 4) {
        // 所有按钮动画完成
        m_isAnimating = false;
        if (m_buttonAnimationTimer) {
            m_buttonAnimationTimer->stop();
        }
        CLIENT_LOG_DEBUG("Button animations completed");
    } else {
        // 动画下一个按钮
        QList<QPushButton*> buttons = {m_newGameButton, m_loadGameButton, m_settingsButton, m_exitButton};
        if (m_currentButtonIndex < buttons.size() && buttons[m_currentButtonIndex]) {
            animateButton(buttons[m_currentButtonIndex], 0);
        }
    }
}

void MainMenu::onBackgroundAnimationUpdate()
{
    m_backgroundOffset += 0.5;
    if (m_backgroundOffset > 100) {
        m_backgroundOffset = 0;
    }
    update(); // 触发重绘
}

void MainMenu::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 创建渐变背景
    QLinearGradient gradient(0, 0, width(), height());
    gradient.setColorAt(0, QColor(20, 20, 40));
    gradient.setColorAt(0.5, QColor(40, 20, 60));
    gradient.setColorAt(1, QColor(20, 40, 80));
    
    painter.fillRect(rect(), gradient);
    
    // 添加动态背景效果
    QRadialGradient radialGradient(width() / 2 + m_backgroundOffset, height() / 2, 300);
    radialGradient.setColorAt(0, QColor(100, 50, 150, 30));
    radialGradient.setColorAt(1, QColor(50, 25, 75, 0));
    
    painter.fillRect(rect(), radialGradient);
    
    // 添加装饰性元素
    painter.setPen(QPen(QColor(255, 255, 255, 20), 1));
    for (int i = 0; i < 10; ++i) {
        int x = (i * 100 + static_cast<int>(m_backgroundOffset)) % width();
        painter.drawLine(x, 0, x, height());
    }
}

void MainMenu::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    showMenu();
}

void MainMenu::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    hideMenu();
}

void MainMenu::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
        case Qt::Key_Up:
        case Qt::Key_W:
            // 向上选择按钮
            break;
        case Qt::Key_Down:
        case Qt::Key_S:
            // 向下选择按钮
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Space:
            // 激活当前选中的按钮
            break;
        case Qt::Key_Escape:
            // 退出游戏
            onExitButtonClicked();
            break;
        default:
            QWidget::keyPressEvent(event);
            break;
    }
}

void MainMenu::setupUI()
{
    CLIENT_LOG_DEBUG("Setting up MainMenu UI");
    
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
    
    // 创建页脚
    createFooter();
    
    CLIENT_LOG_DEBUG("MainMenu UI setup completed");
}

void MainMenu::createTitle()
{
    // 创建标题标签
    m_titleLabel = new QLabel("幻境传说", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setObjectName("titleLabel");
    
    // 创建副标题标签
    m_subtitleLabel = new QLabel("Fantasy Legend", this);
    m_subtitleLabel->setAlignment(Qt::AlignCenter);
    m_subtitleLabel->setObjectName("subtitleLabel");
    
    // 添加到布局
    m_mainLayout->addWidget(m_titleLabel);
    m_mainLayout->addWidget(m_subtitleLabel);
}

void MainMenu::createButtons()
{
    // 创建按钮容器
    m_buttonContainer = new QWidget(this);
    m_buttonLayout = new QVBoxLayout(m_buttonContainer);
    m_buttonLayout->setSpacing(15);
    m_buttonLayout->setContentsMargins(100, 0, 100, 0);
    
    // 创建按钮
    m_newGameButton = new QPushButton("新游戏", this);
    m_newGameButton->setObjectName("menuButton");
    m_newGameButton->setProperty("buttonType", "newGame");
    
    m_loadGameButton = new QPushButton("加载游戏", this);
    m_loadGameButton->setObjectName("menuButton");
    m_loadGameButton->setProperty("buttonType", "loadGame");
    
    m_settingsButton = new QPushButton("设置", this);
    m_settingsButton->setObjectName("menuButton");
    m_settingsButton->setProperty("buttonType", "settings");
    
    m_exitButton = new QPushButton("退出", this);
    m_exitButton->setObjectName("menuButton");
    m_exitButton->setProperty("buttonType", "exit");
    
    // 连接信号
    connect(m_newGameButton, &QPushButton::clicked, this, &MainMenu::onNewGameButtonClicked);
    connect(m_loadGameButton, &QPushButton::clicked, this, &MainMenu::onLoadGameButtonClicked);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainMenu::onSettingsButtonClicked);
    connect(m_exitButton, &QPushButton::clicked, this, &MainMenu::onExitButtonClicked);
    
    // 添加到布局
    m_buttonLayout->addWidget(m_newGameButton);
    m_buttonLayout->addWidget(m_loadGameButton);
    m_buttonLayout->addWidget(m_settingsButton);
    m_buttonLayout->addWidget(m_exitButton);
    
    // 添加到主布局
    m_mainLayout->addWidget(m_buttonContainer);
}

void MainMenu::createFooter()
{
    // 创建版权信息标签
    m_copyrightLabel = new QLabel("© 2024 Fantasy Legend. All rights reserved.", this);
    m_copyrightLabel->setAlignment(Qt::AlignCenter);
    m_copyrightLabel->setObjectName("copyrightLabel");
    
    // 添加到布局
    m_mainLayout->addWidget(m_copyrightLabel);
}

void MainMenu::setupAnimations()
{
    CLIENT_LOG_DEBUG("Setting up MainMenu animations");
    
    // 创建标题动画
    m_titleAnimation = new QPropertyAnimation(m_titleLabel, "geometry", this);
    m_titleAnimation->setDuration(1000);
    m_titleAnimation->setEasingCurve(QEasingCurve::OutBounce);
    
    // 创建副标题动画
    m_subtitleAnimation = new QPropertyAnimation(m_subtitleLabel, "geometry", this);
    m_subtitleAnimation->setDuration(1000);
    m_subtitleAnimation->setEasingCurve(QEasingCurve::OutBounce);
    
    // 创建按钮动画定时器
    m_buttonAnimationTimer = new QTimer(this);
    m_buttonAnimationTimer->setSingleShot(true);
    connect(m_buttonAnimationTimer, &QTimer::timeout, this, &MainMenu::onButtonAnimationFinished);
    
    // 创建背景动画定时器
    m_backgroundAnimationTimer = new QTimer(this);
    connect(m_backgroundAnimationTimer, &QTimer::timeout, this, &MainMenu::onBackgroundAnimationUpdate);
    
    CLIENT_LOG_DEBUG("MainMenu animations setup completed");
}

void MainMenu::applyStyles()
{
    CLIENT_LOG_DEBUG("Applying MainMenu styles");
    
    // 按钮样式
    m_buttonStyle = R"(
        QPushButton#menuButton {
            font-size: 18px;
            font-weight: bold;
            padding: 15px 30px;
            margin: 5px;
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #4A4A4A, stop:1 #2A2A2A);
            color: white;
            border: 2px solid #666666;
            border-radius: 10px;
            min-width: 200px;
            min-height: 50px;
        }
        
        QPushButton#menuButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #666666, stop:1 #4A4A4A);
            border-color: #888888;
            transform: scale(1.05);
        }
        
        QPushButton#menuButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #333333, stop:1 #1A1A1A);
            border-color: #555555;
        }
        
        QPushButton#menuButton:disabled {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #2A2A2A, stop:1 #1A1A1A);
            color: #666666;
            border-color: #444444;
        }
    )";
    
    // 标题样式
    m_titleStyle = R"(
        QLabel#titleLabel {
            font-size: 48px;
            font-weight: bold;
            color: #FFD700;
            margin: 20px;
            text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.8);
        }
    )";
    
    // 副标题样式
    m_subtitleStyle = R"(
        QLabel#subtitleLabel {
            font-size: 18px;
            color: #CCCCCC;
            margin-bottom: 30px;
            text-shadow: 1px 1px 2px rgba(0, 0, 0, 0.8);
        }
    )";
    
    // 版权信息样式
    QString copyrightStyle = R"(
        QLabel#copyrightLabel {
            font-size: 12px;
            color: #666666;
            margin: 20px;
        }
    )";
    
    // 应用样式
    setStyleSheet(m_buttonStyle + m_titleStyle + m_subtitleStyle + copyrightStyle);
    
    // 添加阴影效果
    QGraphicsDropShadowEffect* titleShadow = new QGraphicsDropShadowEffect(this);
    titleShadow->setBlurRadius(20);
    titleShadow->setColor(QColor(0, 0, 0, 100));
    titleShadow->setOffset(2, 2);
    m_titleLabel->setGraphicsEffect(titleShadow);
    
    QGraphicsDropShadowEffect* subtitleShadow = new QGraphicsDropShadowEffect(this);
    subtitleShadow->setBlurRadius(10);
    subtitleShadow->setColor(QColor(0, 0, 0, 80));
    subtitleShadow->setOffset(1, 1);
    m_subtitleLabel->setGraphicsEffect(subtitleShadow);
    
    CLIENT_LOG_DEBUG("MainMenu styles applied");
}

void MainMenu::animateButton(QPushButton* button, int delay)
{
    if (!button) {
        return;
    }
    
    // 创建位置动画
    QPropertyAnimation* posAnimation = new QPropertyAnimation(button, "geometry", this);
    posAnimation->setDuration(500);
    posAnimation->setStartValue(QRect(button->x() - 200, button->y(), button->width(), button->height()));
    posAnimation->setEndValue(button->geometry());
    posAnimation->setEasingCurve(QEasingCurve::OutBack);
    
    // 创建透明度动画
    QPropertyAnimation* opacityAnimation = new QPropertyAnimation(button, "windowOpacity", this);
    opacityAnimation->setDuration(500);
    opacityAnimation->setStartValue(0.0);
    opacityAnimation->setEndValue(1.0);
    opacityAnimation->setEasingCurve(QEasingCurve::OutQuad);
    
    // 启动动画
    posAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    opacityAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainMenu::startBackgroundAnimation()
{
    if (m_backgroundAnimationTimer) {
        m_backgroundAnimationTimer->start(50); // 20 FPS
    }
}

void MainMenu::stopBackgroundAnimation()
{
    if (m_backgroundAnimationTimer) {
        m_backgroundAnimationTimer->stop();
    }
}

} // namespace Fantasy 