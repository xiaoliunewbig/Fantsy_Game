/**
 * @file GameScene.cpp
 * @brief 游戏场景实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "ui/scenes/GameScene.h"
#include <iostream>
#include <cmath>

namespace Fantasy {

Vector2 Vector2::screenToWorld(const std::pair<int, int>& screenPos, float cameraX, float cameraY, float zoom) {
    return Vector2(
        (static_cast<float>(screenPos.first) - 400.0f) / zoom + cameraX,
        (static_cast<float>(screenPos.second) - 300.0f) / zoom + cameraY
    );
}

std::pair<int, int> Vector2::worldToScreen(const Vector2& worldPos, float cameraX, float cameraY, float zoom) {
    int x = static_cast<int>((worldPos.x - cameraX) * zoom + 400.0f);
    int y = static_cast<int>((worldPos.y - cameraY) * zoom + 300.0f);
    return {x, y};
}

GameScene::GameScene()
    : m_isRunning(false),
      m_inputEnabled(true),
      m_cameraPosition(0.0f, 0.0f),
      m_cameraZoom(1.0f),
      m_followTarget(nullptr),
      m_updateInterval(1.0f / 60.0f)
{
    setupUI();
    setupConnections();
}

GameScene::~GameScene() {
    std::cout << "[GameScene] Destroyed." << std::endl;
}

void GameScene::setupUI() {
    initializeGraphics();
}

void GameScene::setupConnections() {
    // 这里可以绑定回调或事件监听
}

void GameScene::initializeGraphics() {
    std::cout << "[GameScene] Graphics initialized." << std::endl;
}

void GameScene::startGame() {
    m_isRunning = true;
    std::cout << "[GameScene] Game started." << std::endl;
}

void GameScene::pauseGame() {
    m_isRunning = false;
    std::cout << "[GameScene] Game paused." << std::endl;
}

void GameScene::resumeGame() {
    m_isRunning = true;
    std::cout << "[GameScene] Game resumed." << std::endl;
}

void GameScene::stopGame() {
    m_isRunning = false;
    std::cout << "[GameScene] Game stopped." << std::endl;
}

void GameScene::startScene() {
    std::cout << "[GameScene] Scene started." << std::endl;
}

void GameScene::pauseScene() {
    std::cout << "[GameScene] Scene paused." << std::endl;
}

void GameScene::resumeScene() {
    std::cout << "[GameScene] Scene resumed." << std::endl;
}

void GameScene::stopScene() {
    std::cout << "[GameScene] Scene stopped." << std::endl;
}

void GameScene::addCharacter(Character* character) {
    m_characters.push_back(character);
    std::cout << "[GameScene] Character added to scene." << std::endl;
}

void GameScene::removeCharacter(Character* character) {
    m_characters.erase(std::remove(m_characters.begin(), m_characters.end(), character), m_characters.end());
    std::cout << "[GameScene] Character removed from scene." << std::endl;
}

void GameScene::updateCharacterPosition(Character* character, const Vector2& position) {
    for (auto cb : m_characterMovedCallbacks) {
        cb(character, position);
    }
    std::cout << "[GameScene] Character moved to (" << position.x << ", " << position.y << ")." << std::endl;
}

void GameScene::setBackground(const std::string& backgroundPath) {
    m_currentBackground = backgroundPath;
    std::cout << "[GameScene] Background set to: " << backgroundPath << std::endl;
}

void GameScene::addEffect(const std::string& effectType, const Vector2& position) {
    m_effects.emplace_back(effectType, position);
    std::cout << "[GameScene] Effect '" << effectType << "' added at (" << position.x << ", " << position.y << ")." << std::endl;
}

void GameScene::clearEffects() {
    m_effects.clear();
    std::cout << "[GameScene] All effects cleared." << std::endl;
}

void GameScene::setInputEnabled(bool enabled) {
    m_inputEnabled = enabled;
    std::cout << "[GameScene] Input " << (enabled ? "enabled" : "disabled") << "." << std::endl;
}

void GameScene::setCameraPosition(const Vector2& position) {
    m_cameraPosition = position;
    std::cout << "[GameScene] Camera moved to (" << position.x << ", " << position.y << ")." << std::endl;
}

void GameScene::setCameraZoom(float zoom) {
    m_cameraZoom = std::max(0.1f, zoom);
    std::cout << "[GameScene] Camera zoom set to " << zoom << "." << std::endl;
}

void GameScene::followCharacter(Character* character) {
    m_followTarget = character;
    std::cout << "[GameScene] Following character." << std::endl;
}

void GameScene::onCharacterMoved(CharacterMovedCallback cb) {
    m_characterMovedCallbacks.push_back(cb);
}

void GameScene::onCharacterSelected(CharacterSelectedCallback cb) {
    m_characterSelectedCallbacks.push_back(cb);
}

void GameScene::onSceneClicked(SceneClickedCallback cb) {
    m_sceneClickedCallbacks.push_back(cb);
}

void GameScene::onKeyPressed(KeyPressedCallback cb) {
    m_keyPressedCallbacks.push_back(cb);
}

void GameScene::onKeyReleased(KeyReleasedCallback cb) {
    m_keyReleasedCallbacks.push_back(cb);
}

void GameScene::update(float deltaTime) {
    if (!m_isRunning) return;

    for (auto cb : m_keyPressedCallbacks) {
        // 模拟按键按住逻辑
    }

    updateCamera();
    for (auto& effect : m_effects) {
        // 更新特效动画
    }

    // 如果有跟随目标，自动调整摄像机
    if (m_followTarget != nullptr) {
        //Vector2 pos = m_followTarget->getPosition(); // 假设 Character 有 getPosition()
        //setCameraPosition(pos);
    }
}

void GameScene::render() const {
    std::cout << "[GameScene] Rendering scene..." << std::endl;
    std::cout << "  - Background: " << m_currentBackground << std::endl;
    std::cout << "  - Characters: " << m_characters.size() << std::endl;
    std::cout << "  - Effects: " << m_effects.size() << std::endl;
}

void GameScene::simulateKeyPress(int key) {
    for (auto cb : m_keyPressedCallbacks) {
        cb(key);
    }
    std::cout << "[GameScene] Key pressed: " << key << std::endl;
}

void GameScene::simulateKeyRelease(int key) {
    for (auto cb : m_keyReleasedCallbacks) {
        cb(key);
    }
    std::cout << "[GameScene] Key released: " << key << std::endl;
}

void GameScene::simulateMousePress(int x, int y) {
    Vector2 worldPos = Vector2::screenToWorld({x, y}, m_cameraPosition.x, m_cameraPosition.y, m_cameraZoom);
    for (auto cb : m_sceneClickedCallbacks) {
        cb(worldPos);
    }
    std::cout << "[GameScene] Mouse clicked at (" << worldPos.x << ", " << worldPos.y << ")." << std::endl;
}

void GameScene::simulateMouseMove(int x, int y) {
    m_mousePosition = Vector2::screenToWorld({x, y}, m_cameraPosition.x, m_cameraPosition.y, m_cameraZoom);
}

void GameScene::simulateMouseRelease(int x, int y) {
    m_mousePressed = false;
}

void GameScene::simulateWheelEvent(int delta) {
    float zoomFactor = 1.0f + delta * 0.001f;
    setCameraZoom(m_cameraZoom * zoomFactor);
}

void GameScene::updateCamera() {
    // 插值更新相机位置
    m_cameraPosition.x += (m_targetCameraPosition.x - m_cameraPosition.x) * 0.1f;
    m_cameraPosition.y += (m_targetCameraPosition.y - m_cameraPosition.y) * 0.1f;
}

} // namespace Fantasy