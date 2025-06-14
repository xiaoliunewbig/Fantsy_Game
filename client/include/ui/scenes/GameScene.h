#pragma once

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QVector2D>
#include <QList>
#include <QMap>

// 前向声明
class Character;
class GameEngine;
class UIManager;

class GameScene : public QWidget {
    Q_OBJECT
    
public:
    GameScene(QWidget* parent = nullptr);
    ~GameScene();
    
    // 场景控制
    void startScene();
    void pauseScene();
    void resumeScene();
    void stopScene();
    
    // 角色管理
    void addCharacter(Character* character);
    void removeCharacter(Character* character);
    void updateCharacterPosition(Character* character, const QVector2D& position);
    
    // 渲染控制
    void setBackground(const QString& backgroundPath);
    void addEffect(const QString& effectType, const QVector2D& position);
    void clearEffects();
    
    // 输入处理
    void setInputEnabled(bool enabled);
    bool isInputEnabled() const { return m_inputEnabled; }
    
    // 视图控制
    void setCameraPosition(const QVector2D& position);
    void setCameraZoom(float zoom);
    void followCharacter(Character* character);
    
    // 获取器
    QGraphicsScene* getGraphicsScene() const { return m_graphicsScene; }
    QGraphicsView* getGraphicsView() const { return m_graphicsView; }
    QList<Character*> getCharacters() const { return m_characters; }
    
signals:
    void characterMoved(Character* character, const QVector2D& position);
    void characterSelected(Character* character);
    void sceneClicked(const QVector2D& position);
    void keyPressed(int key);
    void keyReleased(int key);
    
protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    
private slots:
    void updateScene(float deltaTime);
    void onCharacterPositionChanged(Character* character, const QVector2D& position);
    
private:
    void setupUI();
    void setupConnections();
    void initializeGraphics();
    void updateCamera();
    QVector2D screenToWorld(const QPoint& screenPos);
    QPoint worldToScreen(const QVector2D& worldPos);
    
    // UI组件
    QGraphicsView* m_graphicsView;
    QGraphicsScene* m_graphicsScene;
    
    // 场景状态
    bool m_isRunning;
    bool m_inputEnabled;
    QString m_currentBackground;
    
    // 角色管理
    QList<Character*> m_characters;
    Character* m_followTarget;
    
    // 相机控制
    QVector2D m_cameraPosition;
    float m_cameraZoom;
    QVector2D m_targetCameraPosition;
    
    // 输入状态
    QMap<int, bool> m_keyStates;
    QVector2D m_mousePosition;
    bool m_mousePressed;
    
    // 定时器
    QTimer* m_updateTimer;
    
    // 特效管理
    QList<QGraphicsItem*> m_effects;
};
