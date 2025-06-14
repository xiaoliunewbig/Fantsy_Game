#pragma once

#include <QObject>
#include <QTimer>
#include <QVariantMap>
#include <memory>

class Character;
class Level;
class UIManager;

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    BATTLE,
    DIALOGUE,
    INVENTORY
};

class GameEngine : public QObject {
    Q_OBJECT
    
public:
    static GameEngine* instance();
    
    // 游戏状态管理
    void startGame();
    void pauseGame();
    void resumeGame();
    void stopGame();
    
    // 场景管理
    void loadLevel(int levelId);
    void switchScene(const QString& sceneName);
    
    // 数据管理
    void saveGame(const QString& slotName);
    void loadGame(const QString& slotName);
    
    // 事件系统
    void emitEvent(const QString& eventName, const QVariantMap& data);
    
    // 获取器
    GameState getCurrentState() const { return m_currentState; }
    Character* getPlayer() const { return m_player.get(); }
    Level* getCurrentLevel() const { return m_currentLevel.get(); }
    
signals:
    void gameStateChanged(GameState state);
    void levelLoaded(int levelId);
    void gameSaved(const QString& slotName);
    void gameLoaded(const QString& slotName);
    void eventEmitted(const QString& eventName, const QVariantMap& data);
    
private slots:
    void updateGame(float deltaTime);
    
private:
    GameEngine();
    ~GameEngine();
    
    void initializeSystems();
    void cleanupSystems();
    
    GameState m_currentState;
    QTimer* m_gameTimer;
    QVariantMap m_gameData;
    
    std::unique_ptr<Character> m_player;
    std::unique_ptr<Level> m_currentLevel;
    UIManager* m_uiManager;
    
    static GameEngine* s_instance;
};