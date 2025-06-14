#include "GameEngine.h"
#include "Character.h"
#include "Level.h"
#include "../ui/UIManager.h"
#include "../data/ConfigManager.h"
#include "../utils/Logger.h"

GameEngine* GameEngine::s_instance = nullptr;

GameEngine* GameEngine::instance() {
    if (!s_instance) {
        s_instance = new GameEngine();
    }
    return s_instance;
}

GameEngine::GameEngine() 
    : m_currentState(GameState::MENU)
    , m_gameTimer(new QTimer(this))
    , m_uiManager(nullptr) {
    
    // 连接定时器
    connect(m_gameTimer, &QTimer::timeout, this, [this]() {
        updateGame(1.0f / 60.0f);  // 60 FPS
    });
    
    initializeSystems();
}

GameEngine::~GameEngine() {
    cleanupSystems();
}

void GameEngine::initializeSystems() {
    Logger::info("Initializing game engine...");
    
    // 初始化配置管理器
    ConfigManager::instance();
    
    // 初始化UI管理器
    m_uiManager = new UIManager(this);
    
    // 创建玩家角色
    m_player = std::make_unique<Character>("Player", 1);
    
    Logger::info("Game engine initialized successfully");
}

void GameEngine::startGame() {
    Logger::info("Starting game...");
    
    m_currentState = GameState::PLAYING;
    m_gameTimer->start(16);  // ~60 FPS
    
    emit gameStateChanged(m_currentState);
}

void GameEngine::pauseGame() {
    if (m_currentState == GameState::PLAYING) {
        m_currentState = GameState::PAUSED;
        m_gameTimer->stop();
        emit gameStateChanged(m_currentState);
    }
}

void GameEngine::resumeGame() {
    if (m_currentState == GameState::PAUSED) {
        m_currentState = GameState::PLAYING;
        m_gameTimer->start(16);
        emit gameStateChanged(m_currentState);
    }
}

void GameEngine::updateGame(float deltaTime) {
    // 更新当前关卡
    if (m_currentLevel) {
        m_currentLevel->update(deltaTime);
    }
    
    // 更新玩家
    if (m_player) {
        m_player->update(deltaTime);
    }
    
    // 更新UI
    if (m_uiManager) {
        m_uiManager->update(deltaTime);
    }
}