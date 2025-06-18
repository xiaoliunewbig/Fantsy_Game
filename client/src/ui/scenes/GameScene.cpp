/**
 * @file GameScene.cpp
 * @brief 游戏场景实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "ui/scenes/GameScene.h"
#include "utils/Logger.h"
#include <QVBoxLayout>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QGraphicsEllipseItem>
#include <QBrush>
#include <QPen>
#include <QColor>
#include <QApplication>
#include <QScreen>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>

GameScene::GameScene(QWidget* parent)
    : QWidget(parent)
    , m_graphicsView(new QGraphicsView(this))
    , m_graphicsScene(new QGraphicsScene(this))
    , m_isRunning(false)
    , m_inputEnabled(true)
    , m_followTarget(nullptr)
    , m_cameraPosition(0, 0)
    , m_cameraZoom(1.0f)
    , m_targetCameraPosition(0, 0)
    , m_mousePressed(false)
    , m_updateTimer(new QTimer(this))
    , m_gameTimer(nullptr) {
    
    CLIENT_LOG_DEBUG("Creating GameScene");
    
    setupUI();
    setupConnections();
    initializeGraphics();
    
    // 设置窗口属性
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    
    // 创建游戏定时器
    m_gameTimer = new QTimer(this);
    m_gameTimer->setInterval(16); // ~60 FPS
    connect(m_gameTimer, &QTimer::timeout, this, &GameScene::update);
    
    CLIENT_LOG_DEBUG("GameScene created successfully");
}

GameScene::~GameScene() {
    stopScene();
    CLIENT_LOG_DEBUG("Destroying GameScene");
}

void GameScene::setupUI() {
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_graphicsView);
    
    // 设置图形视图属性
    m_graphicsView->setRenderHint(QPainter::Antialiasing);
    m_graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);
    m_graphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_graphicsView->setDragMode(QGraphicsView::NoDrag);
    m_graphicsView->setFocusPolicy(Qt::StrongFocus);
    
    // 设置场景大小
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    m_graphicsScene->setSceneRect(0, 0, screenGeometry.width(), screenGeometry.height());
    
    m_graphicsView->setScene(m_graphicsScene);
}

void GameScene::setupConnections() {
    // 连接更新定时器
    connect(m_updateTimer, &QTimer::timeout, this, [this]() {
        updateScene(1.0f / 60.0f); // 60 FPS
    });
    
    // 连接游戏引擎信号
    if (GameEngine::instance()) {
        connect(GameEngine::instance(), &GameEngine::gameStateChanged,
                this, [this](GameState state) {
            if (state == GameState::PLAYING) {
                resumeScene();
            } else {
                pauseScene();
            }
        });
    }
}

void GameScene::initializeGraphics() {
    // 设置默认背景
    setBackground(":/resources/images/default_background.png");
    
    // 添加网格线（调试用）
    if (Logger::getLogLevel() == LogLevel::DEBUG) {
        drawGrid();
    }
}

void GameScene::drawGrid() {
    QPen gridPen(QColor(100, 100, 100, 50), 1);
    int gridSize = 50;
    int sceneWidth = m_graphicsScene->width();
    int sceneHeight = m_graphicsScene->height();
    
    // 绘制垂直线
    for (int x = 0; x <= sceneWidth; x += gridSize) {
        m_graphicsScene->addLine(x, 0, x, sceneHeight, gridPen);
    }
    
    // 绘制水平线
    for (int y = 0; y <= sceneHeight; y += gridSize) {
        m_graphicsScene->addLine(0, y, sceneWidth, y, gridPen);
    }
}

void GameScene::startScene() {
    CLIENT_LOG_INFO("Starting game scene");
    if (m_isRunning) {
        return;
    }
    
    m_isRunning = true;
    m_updateTimer->start(16); // ~60 FPS
    setInputEnabled(true);
    
    if (m_gameTimer) {
        m_gameTimer->start();
    }
    
    CLIENT_LOG_INFO("GameScene started");
}

void GameScene::pauseScene() {
    CLIENT_LOG_INFO("Pausing game scene");
    if (!m_isRunning) {
        return;
    }
    
    m_isRunning = false;
    m_updateTimer->stop();
    setInputEnabled(false);
    
    if (m_gameTimer) {
        m_gameTimer->stop();
    }
    
    CLIENT_LOG_INFO("GameScene paused");
}

void GameScene::resumeScene() {
    CLIENT_LOG_INFO("Resuming game scene");
    if (m_isRunning) {
        return;
    }
    
    m_isRunning = true;
    m_updateTimer->start(16);
    setInputEnabled(true);
    
    if (m_gameTimer) {
        m_gameTimer->start();
    }
    
    CLIENT_LOG_INFO("GameScene resumed");
}

void GameScene::stopScene() {
    CLIENT_LOG_INFO("Stopping game scene");
    m_isRunning = false;
    m_updateTimer->stop();
    setInputEnabled(false);
    
    if (m_gameTimer) {
        m_gameTimer->stop();
    }
    
    // 清理角色
    m_characters.clear();
    m_followTarget = nullptr;
    
    // 清理特效
    clearEffects();
    
    CLIENT_LOG_INFO("GameScene stopped");
}

void GameScene::addCharacter(Character* character) {
    if (!character || m_characters.contains(character)) {
        return;
    }
    
    m_characters.append(character);
    
    // 创建角色图形项
    QGraphicsEllipseItem* characterItem = new QGraphicsEllipseItem(-20, -20, 40, 40);
    characterItem->setBrush(QBrush(QColor(100, 150, 255)));
    characterItem->setPen(QPen(QColor(50, 100, 200), 2));
    characterItem->setData(0, QVariant::fromValue(character));
    
    // 添加角色名称标签
    QGraphicsTextItem* nameItem = new QGraphicsTextItem(character->getName());
    nameItem->setDefaultTextColor(QColor::White);
    nameItem->setFont(QFont("Arial", 10));
    nameItem->setPos(-30, -40);
    
    m_graphicsScene->addItem(characterItem);
    m_graphicsScene->addItem(nameItem);
    
    // 连接角色信号
    connect(character, &Character::positionChanged,
            this, &GameScene::onCharacterPositionChanged);
    
    CLIENT_LOG_INFO(QString("Character added: %1").arg(character->getName()));
}

void GameScene::removeCharacter(Character* character) {
    if (!character || !m_characters.contains(character)) {
        return;
    }
    
    m_characters.removeOne(character);
    
    if (m_followTarget == character) {
        m_followTarget = nullptr;
    }
    
    // 移除角色图形项
    QList<QGraphicsItem*> items = m_graphicsScene->items();
    for (QGraphicsItem* item : items) {
        if (item->data(0).value<Character*>() == character) {
            m_graphicsScene->removeItem(item);
            delete item;
        }
    }
    
    // 断开角色信号连接
    disconnect(character, &Character::positionChanged,
              this, &GameScene::onCharacterPositionChanged);
    
    CLIENT_LOG_INFO(QString("Character removed: %1").arg(character->getName()));
}

void GameScene::updateCharacterPosition(Character* character, const QVector2D& position) {
    if (!character) {
        return;
    }
    
    // 更新角色图形项位置
    QList<QGraphicsItem*> items = m_graphicsScene->items();
    for (QGraphicsItem* item : items) {
        if (item->data(0).value<Character*>() == character) {
            item->setPos(position.x(), position.y());
            break;
        }
    }
    
    emit characterMoved(character, position);
}

void GameScene::setBackground(const QString& backgroundPath) {
    m_currentBackground = backgroundPath;
    
    // 移除现有背景
    QList<QGraphicsItem*> items = m_graphicsScene->items();
    for (QGraphicsItem* item : items) {
        if (item->data(0).toString() == "background") {
            m_graphicsScene->removeItem(item);
            delete item;
        }
    }
    
    // 添加新背景
    QPixmap backgroundPixmap(backgroundPath);
    if (!backgroundPixmap.isNull()) {
        QGraphicsPixmapItem* backgroundItem = new QGraphicsPixmapItem(backgroundPixmap);
        backgroundItem->setData(0, "background");
        backgroundItem->setZValue(-1000); // 背景在最底层
        m_graphicsScene->addItem(backgroundItem);
    }
}

void GameScene::addEffect(const QString& effectType, const QVector2D& position) {
    QGraphicsItem* effectItem = nullptr;
    
    if (effectType == "explosion") {
        QGraphicsEllipseItem* explosion = new QGraphicsEllipseItem(-30, -30, 60, 60);
        explosion->setBrush(QBrush(QColor(255, 100, 0, 150)));
        explosion->setPen(QPen(QColor(255, 200, 0), 3));
        effectItem = explosion;
    } else if (effectType == "sparkle") {
        QGraphicsEllipseItem* sparkle = new QGraphicsEllipseItem(-5, -5, 10, 10);
        sparkle->setBrush(QBrush(QColor(255, 255, 0)));
        sparkle->setPen(QPen(QColor(255, 255, 255), 1));
        effectItem = sparkle;
    }
    
    if (effectItem) {
        effectItem->setPos(position.x(), position.y());
        effectItem->setData(0, "effect");
        effectItem->setData(1, effectType);
        m_graphicsScene->addItem(effectItem);
        m_effects.append(effectItem);
        
        // 特效自动消失
        QTimer::singleShot(1000, [this, effectItem]() {
            if (m_effects.contains(effectItem)) {
                m_effects.removeOne(effectItem);
                m_graphicsScene->removeItem(effectItem);
                delete effectItem;
            }
        });
    }
}

void GameScene::clearEffects() {
    for (QGraphicsItem* effect : m_effects) {
        m_graphicsScene->removeItem(effect);
        delete effect;
    }
    m_effects.clear();
}

void GameScene::setInputEnabled(bool enabled) {
    m_inputEnabled = enabled;
    m_graphicsView->setFocusPolicy(enabled ? Qt::StrongFocus : Qt::NoFocus);
}

void GameScene::setCameraPosition(const QVector2D& position) {
    m_cameraPosition = position;
    m_targetCameraPosition = position;
    updateCamera();
}

void GameScene::setCameraZoom(float zoom) {
    m_cameraZoom = qBound(0.1f, zoom, 3.0f);
    updateCamera();
}

void GameScene::followCharacter(Character* character) {
    m_followTarget = character;
}

void GameScene::updateScene(float deltaTime) {
    if (!m_isRunning) {
        return;
    }
    
    // 更新相机
    updateCamera();
    
    // 更新角色
    for (Character* character : m_characters) {
        if (character) {
            character->update(deltaTime);
        }
    }
    
    // 处理输入
    handleInput(deltaTime);
}

void GameScene::updateCamera() {
    if (m_followTarget) {
        QVector2D targetPos = m_followTarget->getPosition();
        m_targetCameraPosition = targetPos;
    }
    
    // 平滑相机移动
    QVector2D diff = m_targetCameraPosition - m_cameraPosition;
    m_cameraPosition += diff * 0.1f; // 平滑系数
    
    // 应用相机变换
    QTransform transform;
    transform.translate(m_graphicsView->width() / 2, m_graphicsView->height() / 2);
    transform.scale(m_cameraZoom, m_cameraZoom);
    transform.translate(-m_cameraPosition.x(), -m_cameraPosition.y());
    
    m_graphicsView->setTransform(transform);
}

void GameScene::handleInput(float deltaTime) {
    if (!m_inputEnabled) {
        return;
    }
    
    // 处理键盘输入
    QVector2D movement(0, 0);
    float moveSpeed = 200.0f; // 像素/秒
    
    if (m_keyStates.value(Qt::Key_W) || m_keyStates.value(Qt::Key_Up)) {
        movement.setY(-1);
    }
    if (m_keyStates.value(Qt::Key_S) || m_keyStates.value(Qt::Key_Down)) {
        movement.setY(1);
    }
    if (m_keyStates.value(Qt::Key_A) || m_keyStates.value(Qt::Key_Left)) {
        movement.setX(-1);
    }
    if (m_keyStates.value(Qt::Key_D) || m_keyStates.value(Qt::Key_Right)) {
        movement.setX(1);
    }
    
    // 标准化对角线移动
    if (movement.length() > 0) {
        movement.normalize();
        movement *= moveSpeed * deltaTime;
        
        // 移动玩家角色
        if (!m_characters.isEmpty()) {
            Character* player = m_characters.first();
            QVector2D newPos = player->getPosition() + movement;
            player->move(newPos);
        }
    }
}

void GameScene::onCharacterPositionChanged(Character* character, const QVector2D& position) {
    updateCharacterPosition(character, position);
}

QVector2D GameScene::screenToWorld(const QPoint& screenPos) {
    QPointF scenePos = m_graphicsView->mapToScene(screenPos);
    return QVector2D(scenePos.x(), scenePos.y());
}

QPoint GameScene::worldToScreen(const QVector2D& worldPos) {
    QPointF scenePos(worldPos.x(), worldPos.y());
    return m_graphicsView->mapFromScene(scenePos);
}

void GameScene::keyPressEvent(QKeyEvent* event) {
    if (!m_inputEnabled) {
        QWidget::keyPressEvent(event);
        return;
    }
    
    m_keyStates[event->key()] = true;
    emit keyPressed(event->key());
    
    switch (event->key()) {
        case Qt::Key_W:
        case Qt::Key_Up:
            CLIENT_LOG_DEBUG("Move up");
            break;
        case Qt::Key_S:
        case Qt::Key_Down:
            CLIENT_LOG_DEBUG("Move down");
            break;
        case Qt::Key_A:
        case Qt::Key_Left:
            CLIENT_LOG_DEBUG("Move left");
            break;
        case Qt::Key_D:
        case Qt::Key_Right:
            CLIENT_LOG_DEBUG("Move right");
            break;
        case Qt::Key_Space:
            CLIENT_LOG_DEBUG("Jump/Attack");
            break;
        case Qt::Key_Escape:
            CLIENT_LOG_DEBUG("Pause game");
            break;
        default:
            break;
    }
    
    QWidget::keyPressEvent(event);
}

void GameScene::keyReleaseEvent(QKeyEvent* event) {
    if (!m_inputEnabled) {
        QWidget::keyReleaseEvent(event);
        return;
    }
    
    m_keyStates[event->key()] = false;
    emit keyReleased(event->key());
    
    QWidget::keyReleaseEvent(event);
}

void GameScene::mousePressEvent(QMouseEvent* event) {
    if (!m_inputEnabled) {
        QWidget::mousePressEvent(event);
        return;
    }
    
    m_mousePressed = true;
    m_mousePosition = QVector2D(event->pos());
    
    QVector2D worldPos = screenToWorld(event->pos());
    emit sceneClicked(worldPos);
    
    CLIENT_LOG_DEBUG("Mouse pressed at (%d, %d)", event->x(), event->y());
    
    QWidget::mousePressEvent(event);
}

void GameScene::mouseReleaseEvent(QMouseEvent* event) {
    if (!m_inputEnabled) {
        QWidget::mouseReleaseEvent(event);
        return;
    }
    
    m_mousePressed = false;
    
    QWidget::mouseReleaseEvent(event);
}

void GameScene::mouseMoveEvent(QMouseEvent* event) {
    if (!m_inputEnabled) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    
    m_mousePosition = QVector2D(event->pos());
    
    QWidget::mouseMoveEvent(event);
}

void GameScene::wheelEvent(QWheelEvent* event) {
    if (!m_inputEnabled) {
        QWidget::wheelEvent(event);
        return;
    }
    
    // 鼠标滚轮缩放
    float zoomFactor = event->angleDelta().y() > 0 ? 1.1f : 0.9f;
    setCameraZoom(m_cameraZoom * zoomFactor);
    
    QWidget::wheelEvent(event);
}

void GameScene::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    
    // 更新场景大小
    m_graphicsScene->setSceneRect(0, 0, width(), height());
    
    // 重新绘制网格（调试模式）
    if (Logger::getLogLevel() == LogLevel::DEBUG) {
        // 清除现有网格
        QList<QGraphicsItem*> items = m_graphicsScene->items();
        for (QGraphicsItem* item : items) {
            if (item->data(0).toString() == "grid") {
                m_graphicsScene->removeItem(item);
                delete item;
            }
        }
        drawGrid();
    }
}

void GameScene::update() {
    if (!m_isRunning) {
        return;
    }
    
    // 更新游戏逻辑
    // TODO: 实现游戏逻辑更新
    
    // 触发重绘
    update();
}
