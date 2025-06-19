/**
 * @file GameScene.h
 * @brief 游戏场景
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>

// 前向声明
class Character;
class GameEngine;
class UIManager;

namespace Fantasy {

struct Vector2 {
    float x, y;

    Vector2(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}

    static Vector2 screenToWorld(const std::pair<int, int>& screenPos, float cameraX, float cameraY, float zoom);
    static std::pair<int, int> worldToScreen(const Vector2& worldPos, float cameraX, float cameraY, float zoom);
};

/**
 * @brief 游戏场景类（无 Qt 依赖）
 *
 * 负责渲染游戏世界和角色
 */
class GameScene {
public:
    using Callback = std::function<void()>;
    using CharacterMovedCallback = std::function<void(Character*, const Vector2&)>;
    using CharacterSelectedCallback = std::function<void(Character*)>;
    using SceneClickedCallback = std::function<void(const Vector2&)>;
    using KeyPressedCallback = std::function<void(int key)>;
    using KeyReleasedCallback = std::function<void(int key)>;

public:
    GameScene();
    ~GameScene();

    // 游戏控制
    void startGame();
    void pauseGame();
    void resumeGame();
    void stopGame();

    // 场景控制
    void startScene();
    void pauseScene();
    void resumeScene();
    void stopScene();

    // 角色管理
    void addCharacter(Character* character);
    void removeCharacter(Character* character);
    void updateCharacterPosition(Character* character, const Vector2& position);

    // 渲染控制
    void setBackground(const std::string& backgroundPath);
    void addEffect(const std::string& effectType, const Vector2& position);
    void clearEffects();

    // 输入处理
    void setInputEnabled(bool enabled);
    bool isInputEnabled() const { return m_inputEnabled; }

    // 摄像机控制
    void setCameraPosition(const Vector2& position);
    void setCameraZoom(float zoom);
    void followCharacter(Character* character);

    // 获取器
    std::vector<Character*> getCharacters() const { return m_characters; }

    // 回调注册
    void onCharacterMoved(CharacterMovedCallback cb);
    void onCharacterSelected(CharacterSelectedCallback cb);
    void onSceneClicked(SceneClickedCallback cb);
    void onKeyPressed(KeyPressedCallback cb);
    void onKeyReleased(KeyReleasedCallback cb);

    // 更新与渲染
    void update(float deltaTime);
    void render() const;

    // 模拟输入事件
    void simulateKeyPress(int key);
    void simulateKeyRelease(int key);
    void simulateMousePress(int x, int y);
    void simulateMouseMove(int x, int y);
    void simulateMouseRelease(int x, int y);
    void simulateWheelEvent(int delta);

private:
    void setupUI();
    void setupConnections();
    void initializeGraphics();
    void updateCamera();

private:
    // UI组件（模拟）
    std::string m_currentBackground;
    std::vector<std::pair<std::string, Vector2>> m_effects;

    // 场景状态
    bool m_isRunning;
    bool m_inputEnabled;

    // 角色管理
    std::vector<Character*> m_characters;
    Character* m_followTarget;

    // 摄像机控制
    Vector2 m_cameraPosition;
    float m_cameraZoom;
    Vector2 m_targetCameraPosition;

    // 输入状态
    std::map<int, bool> m_keyStates;
    Vector2 m_mousePosition;
    bool m_mousePressed;

    // 定时器
    float m_updateInterval;

    // 回调列表
    std::vector<CharacterMovedCallback> m_characterMovedCallbacks;
    std::vector<CharacterSelectedCallback> m_characterSelectedCallbacks;
    std::vector<SceneClickedCallback> m_sceneClickedCallbacks;
    std::vector<KeyPressedCallback> m_keyPressedCallbacks;
    std::vector<KeyReleasedCallback> m_keyReleasedCallbacks;
};

} // namespace Fantasy

#endif // PURE_GAMESCENE_H