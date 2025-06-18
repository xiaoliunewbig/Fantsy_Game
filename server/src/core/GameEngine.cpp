/**
 * @file GameEngine.cpp
 * @brief 游戏引擎实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "core/GameEngine.h"
#include "utils/resources/ResourceLogger.h"
#include "utils/config/GameConfigManager.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>

namespace Fantasy {

// GameEngine::Impl 内部实现
class GameEngine::Impl {
public:
    bool Initd_ = false;
    GameState currentState_ = GameState::INITIALIZING;
    GameConfig config_;
    GameStats stats_;
    
    std::mutex engineMutex_;
    std::condition_variable pauseCondition_;
    std::atomic<bool> running_ = false;
    std::atomic<bool> paused_ = false;
    bool debugMode_;
    bool profilingEnabled_;
    
    // 系统组件
    std::shared_ptr<IResourceManager> resourceManager_;
    std::shared_ptr<GameConfigManager> configManager_;
    std::shared_ptr<DatabaseManager> databaseManager_;
    std::shared_ptr<EventSystem> eventSystem_;
    std::shared_ptr<InputManager> inputManager_;
    std::shared_ptr<AudioManager> audioManager_;
    std::shared_ptr<RenderManager> renderManager_;
    std::shared_ptr<PhysicsEngine> physicsEngine_;
    std::shared_ptr<AIEngine> aiEngine_;
    std::shared_ptr<NetworkManager> networkManager_;
    std::shared_ptr<UIManager> uiManager_;
    
    // 游戏数据
    std::shared_ptr<Level> currentLevel_;
    std::shared_ptr<Character> player_;
    std::vector<std::shared_ptr<Character>> allCharacters_;
    
    // 主循环控制
    std::thread mainLoopThread_;
    std::atomic<bool> shouldStop_;
    
    // 事件系统
    std::unordered_map<std::string, std::vector<GameEventCallback>> eventCallbacks_;
    std::vector<GameEvent> eventQueue_;
    std::mutex eventMutex_;
    
    // 性能监控
    std::chrono::high_resolution_clock::time_point lastFrameTime_;
    std::chrono::high_resolution_clock::time_point lastUpdateTime_;
    double deltaTime_;
    double frameTime_;
    
    // 保存系统
    std::string saveDirectory_;
    std::vector<std::string> saveSlots_;
    
    Impl() : debugMode_(false), profilingEnabled_(false), deltaTime_(0.0), frameTime_(0.0),
             shouldStop_(false), configManager_(std::make_shared<GameConfigManager>()) {
        stats_ = GameStats{};
        lastFrameTime_ = std::chrono::high_resolution_clock::now();
        lastUpdateTime_ = lastFrameTime_;
    }
};

GameEngine& GameEngine::getInstance() {
    static GameEngine instance;
    return instance;
}

GameEngine::GameEngine() : pImpl_(std::make_unique<Impl>()) {}
GameEngine::~GameEngine() = default;

bool GameEngine::Init(const GameConfig& config) {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    
    if (pImpl_->Initd_) {
        GAME_LOG_WARN("GameEngine already Initd");
        return true;
    }
    
    GAME_LOG_INFO("Initializing GameEngine...");
    
    pImpl_->config_ = config;
    pImpl_->currentState_ = GameState::INITIALIZING;
    
    // 创建必要的目录
    std::filesystem::create_directories("saves");
    std::filesystem::create_directories("logs");
    std::filesystem::create_directories("resources");
    std::filesystem::create_directories("config");
    
    // 初始化所有系统
    if (!InitSystems()) {
        GAME_LOG_ERROR("Failed to Init game systems");
        return false;
    }
    
    pImpl_->Initd_ = true;
    pImpl_->currentState_ = GameState::MENU;
    
    GAME_LOG_INFO("GameEngine Initd successfully");
    return true;
}

void GameEngine::shutdown() {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    
    if (!pImpl_->Initd_) {
        return;
    }
    
    GAME_LOG_INFO("Shutting down GameEngine...");
    
    // 停止主循环
    stop();
    
    // 清理所有系统
    shutdownSystems();
    
    pImpl_->Initd_ = false;
    pImpl_->currentState_ = GameState::QUITTING;
    
    GAME_LOG_INFO("GameEngine shutdown complete");
}

bool GameEngine::isInit() const {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    return pImpl_->Initd_;
}

void GameEngine::start() {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    
    if (!pImpl_->Initd_ || pImpl_->running_) {
        return;
    }
    
    GAME_LOG_INFO("Starting game engine...");
    
    pImpl_->running_ = true;
    pImpl_->shouldStop_ = false;
    pImpl_->currentState_ = GameState::PLAYING;
    
    // 启动主循环线程
    pImpl_->mainLoopThread_ = std::thread(&GameEngine::mainLoop, this);
    
    GAME_LOG_INFO("Game engine started");
}

void GameEngine::stop() {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    
    if (!pImpl_->running_) {
        return;
    }
    
    GAME_LOG_INFO("Stopping game engine...");
    
    pImpl_->shouldStop_ = true;
    pImpl_->running_ = false;
    pImpl_->currentState_ = GameState::QUITTING;
    
    // 等待主循环线程结束
    if (pImpl_->mainLoopThread_.joinable()) {
        pImpl_->mainLoopThread_.join();
    }
    
    GAME_LOG_INFO("Game engine stopped");
}

void GameEngine::pause() {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    
    if (pImpl_->currentState_ == GameState::PLAYING) {
        pImpl_->paused_ = true;
        pImpl_->currentState_ = GameState::PAUSED;
        GAME_LOG_INFO("Game paused");
    }
}

void GameEngine::resume() {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    
    if (pImpl_->currentState_ == GameState::PAUSED) {
        pImpl_->paused_ = false;
        pImpl_->currentState_ = GameState::PLAYING;
        GAME_LOG_INFO("Game resumed");
    }
}

void GameEngine::update() {
    // 这个方法主要用于外部调用，内部更新在主循环中进行
    if (pImpl_->running_ && !pImpl_->paused_) {
        updateSystems(pImpl_->deltaTime_);
    }
}

void GameEngine::setState(GameState state) {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    
    if (pImpl_->currentState_ != state) {
        GameState oldState = pImpl_->currentState_;
        pImpl_->currentState_ = state;
        
        // 发送状态改变事件
        emitEvent(GameEventType::STATE_CHANGED, "GameStateChanged", 
                 std::unordered_map<std::string, std::string>{
                     {"oldState", std::to_string(static_cast<int>(oldState))},
                     {"newState", std::to_string(static_cast<int>(state))}
                 });
        
        GAME_LOG_INFO("Game state changed: {} -> {}", 
                     static_cast<int>(oldState), static_cast<int>(state));
    }
}

GameState GameEngine::getState() const {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    return pImpl_->currentState_;
}

bool GameEngine::isState(GameState state) const {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    return pImpl_->currentState_ == state;
}

bool GameEngine::loadLevel(const std::string& levelId) {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    
    GAME_LOG_INFO("Loading level: {}", levelId);
    
    pImpl_->currentState_ = GameState::LOADING;
    
    // 使用GameConfigManager加载关卡配置
    auto configManager = std::dynamic_pointer_cast<GameConfigManager>(pImpl_->configManager_);
    if (!configManager) {
        GAME_LOG_ERROR("Failed to cast ConfigManager to GameConfigManager");
        return false;
    }
    
    // 加载关卡配置
    LevelConfig levelConfig = configManager->loadLevelConfig(levelId);
    
    // 加载关卡地形
    std::vector<TerrainTile> terrain = configManager->loadLevelTerrain(levelId);
    GAME_LOG_INFO("Loaded {} terrain tiles for level {}", terrain.size(), levelId);
    
    // 加载关卡实体
    std::vector<std::string> entities = configManager->loadLevelEntities(levelId);
    GAME_LOG_INFO("Loaded {} entities for level {}", entities.size(), levelId);
    
    // TODO: 创建关卡对象并初始化
    // pImpl_->currentLevel_ = std::make_shared<Level>(levelConfig);
    // pImpl_->currentLevel_->setTerrain(terrain);
    // 加载实体...
    
    pImpl_->currentState_ = GameState::PLAYING;
    
    emitEvent(GameEventType::LEVEL_LOADED, "LevelLoaded", levelId);
    
    GAME_LOG_INFO("Level loaded successfully: {}", levelId);
    return true;
}

bool GameEngine::unloadLevel() {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    
    if (pImpl_->currentLevel_) {
        GAME_LOG_INFO("Unloading current level");
        pImpl_->currentLevel_.reset();
        return true;
    }
    
    return false;
}

std::shared_ptr<Level> GameEngine::getCurrentLevel() const {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    return pImpl_->currentLevel_;
}

std::shared_ptr<Character> GameEngine::getPlayer() const {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    return pImpl_->player_;
}

bool GameEngine::setPlayer(std::shared_ptr<Character> player) {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    
    if (player) {
        pImpl_->player_ = player;
        GAME_LOG_INFO("Player character set");
        return true;
    }
    
    return false;
}

std::vector<std::shared_ptr<Character>> GameEngine::getAllCharacters() const {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    return pImpl_->allCharacters_;
}

void GameEngine::emitEvent(const GameEvent& event) {
    std::lock_guard<std::mutex> lock(pImpl_->eventMutex_);
    pImpl_->eventQueue_.push_back(event);
    pImpl_->stats_.totalEvents++;
}

void GameEngine::emitEvent(GameEventType type, const std::string& name, const GameEventData& data) {
    emitEvent(GameEvent(type, name, data));
}

void GameEngine::subscribeToEvent(GameEventType type, GameEventCallback callback) {
    std::string eventName = getEventName(type);
    if (pImpl_->eventCallbacks_.find(eventName) == pImpl_->eventCallbacks_.end()) {
        pImpl_->eventCallbacks_[eventName] = std::vector<GameEventCallback>();
    }
    pImpl_->eventCallbacks_[eventName].push_back(callback);
}

void GameEngine::unsubscribeFromEvent(GameEventType type, GameEventCallback callback) {
    std::string eventName = getEventName(type);
    auto it = pImpl_->eventCallbacks_.find(eventName);
    if (it != pImpl_->eventCallbacks_.end()) {
        auto& callbacks = it->second;
        callbacks.erase(
            std::remove_if(callbacks.begin(), callbacks.end(),
                [&callback](const GameEventCallback& cb) {
                    return &cb == &callback;
                }),
            callbacks.end()
        );
    }
}

bool GameEngine::saveGame(const std::string& saveSlot) {
    (void)saveSlot; // 避免未使用参数警告
    
    try {
        // 获取系统配置
        SystemConfig sysConfig;
        auto configManager = std::dynamic_pointer_cast<GameConfigManager>(pImpl_->configManager_);
        if (configManager) {
            sysConfig = configManager->loadSystemConfig();
        }
        
        // 保存游戏状态
        return true;
    } catch (const std::exception& e) {
        // 记录错误
        return false;
    }
}

bool GameEngine::loadGame(const std::string& slotName) {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    
    GAME_LOG_INFO("Loading game from slot: {}", slotName);
    
    pImpl_->currentState_ = GameState::LOADING_GAME;
    
    // 使用GameConfigManager加载游戏数据
    auto configManager = std::dynamic_pointer_cast<GameConfigManager>(pImpl_->configManager_);
    if (!configManager) {
        GAME_LOG_ERROR("Failed to cast ConfigManager to GameConfigManager");
        return false;
    }
    
    // 加载游戏配置
    pImpl_->config_ = configManager->loadGameConfig();
    
    // 加载系统配置
    SystemConfig sysConfig = configManager->loadSystemConfig();
    
    // TODO: 加载角色数据
    // std::string characterId = ...;
    // CharacterStats stats = configManager->loadCharacterStats(characterId);
    // std::unordered_map<std::string, CharacterSkill> skills = configManager->loadCharacterSkills(characterId);
    // std::unordered_map<EquipmentSlot, std::string> equipment = configManager->loadCharacterEquipment(characterId);
    // 
    // // 创建角色
    // pImpl_->player_ = std::make_shared<Character>(characterId);
    // pImpl_->player_->setStats(stats);
    // // 设置技能和装备...
    
    // TODO: 加载关卡数据
    // std::string levelId = ...;
    // LevelConfig levelConfig = configManager->loadLevelConfig(levelId);
    // std::vector<TerrainTile> terrain = configManager->loadLevelTerrain(levelId);
    // std::vector<std::string> entities = configManager->loadLevelEntities(levelId);
    // 
    // // 创建关卡
    // pImpl_->currentLevel_ = std::make_shared<Level>(levelConfig);
    // // 设置地形和实体...
    
    pImpl_->currentState_ = GameState::PLAYING;
    
    emitEvent(GameEventType::GAME_LOADED, "GameLoaded", slotName);
    
    GAME_LOG_INFO("Game loaded successfully from slot: {}", slotName);
    return true;
}

bool GameEngine::deleteGame(const std::string& slotName) {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    
    GAME_LOG_INFO("Deleting game slot: {}", slotName);
    
    // TODO: 实现游戏删除逻辑
    // 1. 从数据库或文件删除保存数据
    // 2. 从保存槽列表中移除
    
    auto it = std::find(pImpl_->saveSlots_.begin(), pImpl_->saveSlots_.end(), slotName);
    if (it != pImpl_->saveSlots_.end()) {
        pImpl_->saveSlots_.erase(it);
        GAME_LOG_INFO("Game slot deleted: {}", slotName);
        return true;
    }
    
    return false;
}

std::vector<std::string> GameEngine::getSaveSlots() const {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    return pImpl_->saveSlots_;
}

// 系统访问器实现
std::shared_ptr<IResourceManager> GameEngine::getResourceManager() const {
    return pImpl_->resourceManager_;
}

std::shared_ptr<GameConfigManager> GameEngine::getConfigManager() const {
    return std::dynamic_pointer_cast<GameConfigManager>(pImpl_->configManager_);
}

std::shared_ptr<DatabaseManager> GameEngine::getDatabaseManager() const {
    return pImpl_->databaseManager_;
}

std::shared_ptr<EventSystem> GameEngine::getEventSystem() const {
    return pImpl_->eventSystem_;
}

std::shared_ptr<InputManager> GameEngine::getInputManager() const {
    return pImpl_->inputManager_;
}

std::shared_ptr<AudioManager> GameEngine::getAudioManager() const {
    return pImpl_->audioManager_;
}

std::shared_ptr<RenderManager> GameEngine::getRenderManager() const {
    return pImpl_->renderManager_;
}

std::shared_ptr<PhysicsEngine> GameEngine::getPhysicsEngine() const {
    return pImpl_->physicsEngine_;
}

std::shared_ptr<AIEngine> GameEngine::getAIEngine() const {
    return pImpl_->aiEngine_;
}

std::shared_ptr<NetworkManager> GameEngine::getNetworkManager() const {
    return pImpl_->networkManager_;
}

std::shared_ptr<UIManager> GameEngine::getUIManager() const {
    return pImpl_->uiManager_;
}

const GameConfig& GameEngine::getConfig() const {
    return pImpl_->config_;
}

void GameEngine::setConfig(const GameConfig& config) {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    pImpl_->config_ = config;
}

const GameStats& GameEngine::getStats() const {
    return pImpl_->stats_;
}

void GameEngine::resetStats() {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    pImpl_->stats_ = GameStats{};
}

double GameEngine::getDeltaTime() const {
    return pImpl_->deltaTime_;
}

uint64_t GameEngine::getFrameCount() const {
    return pImpl_->stats_.totalFrames;
}

double GameEngine::getFPS() const {
    return pImpl_->stats_.currentFPS;
}

size_t GameEngine::getMemoryUsage() const {
    return pImpl_->stats_.memoryUsage;
}

std::string GameEngine::getVersion() const {
    return pImpl_->config_.version;
}

void GameEngine::enableDebugMode(bool enable) {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    pImpl_->debugMode_ = enable;
    GAME_LOG_INFO("Debug mode {}", enable ? "enabled" : "disabled");
}

bool GameEngine::isDebugMode() const {
    return pImpl_->debugMode_;
}

void GameEngine::enableProfiling(bool enable) {
    std::lock_guard<std::mutex> lock(pImpl_->engineMutex_);
    pImpl_->profilingEnabled_ = enable;
    GAME_LOG_INFO("Profiling {}", enable ? "enabled" : "disabled");
}

bool GameEngine::isProfiling() const {
    return pImpl_->profilingEnabled_;
}

void GameEngine::logPerformance(const std::string& operation, double timeMs) {
    if (pImpl_->profilingEnabled_) {
        GAME_LOG_DEBUG("Performance: {} took {:.2f}ms", operation, timeMs);
    }
}

// 私有方法实现
void GameEngine::mainLoop() {
    GAME_LOG_INFO("Main loop started");
    
    auto targetFrameTime = std::chrono::microseconds(1000000 / pImpl_->config_.targetFPS);
    
    while (!pImpl_->shouldStop_) {
        auto frameStart = std::chrono::high_resolution_clock::now();
        
        // 计算帧时间
        pImpl_->frameTime_ = std::chrono::duration<double>(frameStart - pImpl_->lastFrameTime_).count();
        pImpl_->lastFrameTime_ = frameStart;
        
        // 限制帧率
        if (pImpl_->frameTime_ < targetFrameTime.count() / 1000000.0) {
            std::this_thread::sleep_for(targetFrameTime - std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::high_resolution_clock::now() - frameStart));
        }
        
        // 更新游戏
        if (!pImpl_->paused_) {
            updateSystems(pImpl_->deltaTime_);
            renderSystems();
            processEvents();
        }
        
        // 更新统计信息
        updateStats();
        
        pImpl_->stats_.totalFrames++;
    }
    
    GAME_LOG_INFO("Main loop stopped");
}

void GameEngine::updateSystems(double deltaTime) {
    pImpl_->deltaTime_ = deltaTime;
    
    // TODO: 实现系统更新逻辑
    // 1. 更新输入系统
    // 2. 更新物理引擎
    // 3. 更新AI系统
    // 4. 更新游戏对象
    // 5. 更新音频系统
    // 6. 更新UI系统
    // 7. 更新网络系统
    
    // 模拟系统更新
    if (pImpl_->currentLevel_) {
        // pImpl_->currentLevel_->update(deltaTime);
    }
    
    if (pImpl_->player_) {
        // pImpl_->player_->update(deltaTime);
    }
}

void GameEngine::renderSystems() {
    // TODO: 实现渲染系统
    // 1. 清除屏幕
    // 2. 渲染游戏世界
    // 3. 渲染UI界面
    // 4. 交换缓冲区
}

void GameEngine::processEvents() {
    // 处理事件队列
    while (!pImpl_->eventQueue_.empty()) {
        GameEvent event = pImpl_->eventQueue_.front();
        pImpl_->eventQueue_.erase(pImpl_->eventQueue_.begin());
        
        // 查找并调用事件回调
        auto it = pImpl_->eventCallbacks_.find(event.name);
        if (it != pImpl_->eventCallbacks_.end()) {
            for (const auto& callback : it->second) {
                try {
                    callback(event);
                } catch (const std::exception& e) {
                    // 记录回调执行错误
                }
            }
        }
    }
}

void GameEngine::updateStats() {
    auto now = std::chrono::system_clock::now();
    auto timeSinceLastUpdate = std::chrono::duration<double>(now - pImpl_->stats_.lastUpdateTime).count();
    
    if (timeSinceLastUpdate >= 1.0) { // 每秒更新一次
        pImpl_->stats_.currentFPS = pImpl_->stats_.totalFrames / timeSinceLastUpdate;
        pImpl_->stats_.averageFPS = pImpl_->stats_.totalFrames / 
                                   std::chrono::duration<double>(now - pImpl_->stats_.startTime).count();
        
        // TODO: 获取实际内存使用情况
        // pImpl_->stats_.memoryUsage = getCurrentMemoryUsage();
        // pImpl_->stats_.peakMemoryUsage = std::max(pImpl_->stats_.peakMemoryUsage, pImpl_->stats_.memoryUsage);
        
        pImpl_->stats_.lastUpdateTime = now;
        pImpl_->stats_.totalFrames = 0;
    }
}

void GameEngine::cleanupResources() {
    // TODO: 实现资源清理
    // 1. 清理未使用的资源
    // 2. 释放内存
    // 3. 清理缓存
}

// 系统初始化方法
bool GameEngine::InitSystems() {
    GAME_LOG_INFO("Initializing game systems...");
    
    bool success = true;
    
    if (!InitResourceManager()) success = false;
    if (!InitConfigManager()) success = false;
    if (!InitDatabaseManager()) success = false;
    if (!InitEventSystem()) success = false;
    if (!InitInputManager()) success = false;
    if (!InitAudioManager()) success = false;
    if (!InitRenderManager()) success = false;
    if (!InitPhysicsEngine()) success = false;
    if (!InitAIEngine()) success = false;
    if (!InitNetworkManager()) success = false;
    if (!InitUIManager()) success = false;
    
    return success;
}

bool GameEngine::InitResourceManager() {
    GAME_LOG_INFO("Initializing ResourceManager...");
    // TODO: 实现资源管理器初始化
    // pImpl_->resourceManager_ = std::make_shared<ResourceManager>();
    // return pImpl_->resourceManager_->Init();
    return true;
}

bool GameEngine::InitConfigManager() {
    GAME_LOG_INFO("Initializing ConfigManager...");
    
    // 使用GameConfigManager
    pImpl_->configManager_ = std::make_shared<GameConfigManager>();
    auto* configManager = static_cast<GameConfigManager*>(pImpl_->configManager_.get());
    
    // 初始化配置管理器，使用游戏配置中指定的配置目录
    std::filesystem::path configDir = "config";
    if (!configManager->Init(configDir)) {
        GAME_LOG_ERROR("Failed to initialize ConfigManager");
        return false;
    }
    
    // 加载游戏配置
    pImpl_->config_ = configManager->loadGameConfig();
    
    GAME_LOG_INFO("ConfigManager initialized successfully");
    return true;
}

bool GameEngine::InitDatabaseManager() {
    GAME_LOG_INFO("Initializing DatabaseManager...");
    // TODO: 实现数据库管理器初始化
    // pImpl_->databaseManager_ = std::make_shared<DatabaseManager>();
    // return pImpl_->databaseManager_->Init();
    return true;
}

bool GameEngine::InitEventSystem() {
    GAME_LOG_INFO("Initializing EventSystem...");
    // TODO: 实现事件系统初始化
    // pImpl_->eventSystem_ = std::make_shared<EventSystem>();
    // return pImpl_->eventSystem_->Init();
    return true;
}

bool GameEngine::InitInputManager() {
    GAME_LOG_INFO("Initializing InputManager...");
    // TODO: 实现输入管理器初始化
    // pImpl_->inputManager_ = std::make_shared<InputManager>();
    // return pImpl_->inputManager_->Init();
    return true;
}

bool GameEngine::InitAudioManager() {
    GAME_LOG_INFO("Initializing AudioManager...");
    // TODO: 实现音频管理器初始化
    // pImpl_->audioManager_ = std::make_shared<AudioManager>();
    // return pImpl_->audioManager_->Init();
    return true;
}

bool GameEngine::InitRenderManager() {
    GAME_LOG_INFO("Initializing RenderManager...");
    // TODO: 实现渲染管理器初始化
    // pImpl_->renderManager_ = std::make_shared<RenderManager>();
    // return pImpl_->renderManager_->Init();
    return true;
}

bool GameEngine::InitPhysicsEngine() {
    GAME_LOG_INFO("Initializing PhysicsEngine...");
    // TODO: 实现物理引擎初始化
    // pImpl_->physicsEngine_ = std::make_shared<PhysicsEngine>();
    // return pImpl_->physicsEngine_->Init();
    return true;
}

bool GameEngine::InitAIEngine() {
    GAME_LOG_INFO("Initializing AIEngine...");
    // TODO: 实现AI引擎初始化
    // pImpl_->aiEngine_ = std::make_shared<AIEngine>();
    // return pImpl_->aiEngine_->Init();
    return true;
}

bool GameEngine::InitNetworkManager() {
    GAME_LOG_INFO("Initializing NetworkManager...");
    // TODO: 实现网络管理器初始化
    // pImpl_->networkManager_ = std::make_shared<NetworkManager>();
    // return pImpl_->networkManager_->Init();
    return true;
}

bool GameEngine::InitUIManager() {
    GAME_LOG_INFO("Initializing UIManager...");
    // TODO: 实现UI管理器初始化
    // pImpl_->uiManager_ = std::make_shared<UIManager>();
    // return pImpl_->uiManager_->Init();
    return true;
}

// 系统清理方法
void GameEngine::shutdownSystems() {
    GAME_LOG_INFO("Shutting down game systems...");
    
    shutdownUIManager();
    shutdownNetworkManager();
    shutdownAIEngine();
    shutdownPhysicsEngine();
    shutdownRenderManager();
    shutdownAudioManager();
    shutdownInputManager();
    shutdownEventSystem();
    shutdownDatabaseManager();
    shutdownConfigManager();
    shutdownResourceManager();
}

void GameEngine::shutdownResourceManager() {
    if (pImpl_->resourceManager_) {
        pImpl_->resourceManager_->shutdown();
        pImpl_->resourceManager_.reset();
    }
}

void GameEngine::shutdownConfigManager() {
    if (pImpl_->configManager_) {
        pImpl_->configManager_->shutdown();
        pImpl_->configManager_.reset();
    }
}

void GameEngine::shutdownDatabaseManager() {
    if (pImpl_->databaseManager_) {
        pImpl_->databaseManager_->shutdown();
        pImpl_->databaseManager_.reset();
    }
}

void GameEngine::shutdownEventSystem() {
    if (pImpl_->eventSystem_) {
        pImpl_->eventSystem_->shutdown();
        pImpl_->eventSystem_.reset();
    }
}

void GameEngine::shutdownInputManager() {
    if (pImpl_->inputManager_) {
        pImpl_->inputManager_->shutdown();
        pImpl_->inputManager_.reset();
    }
}

void GameEngine::shutdownAudioManager() {
    if (pImpl_->audioManager_) {
        pImpl_->audioManager_->shutdown();
        pImpl_->audioManager_.reset();
    }
}

void GameEngine::shutdownRenderManager() {
    if (pImpl_->renderManager_) {
        pImpl_->renderManager_->shutdown();
        pImpl_->renderManager_.reset();
    }
}

void GameEngine::shutdownPhysicsEngine() {
    if (pImpl_->physicsEngine_) {
        pImpl_->physicsEngine_->shutdown();
        pImpl_->physicsEngine_.reset();
    }
}

void GameEngine::shutdownAIEngine() {
    if (pImpl_->aiEngine_) {
        pImpl_->aiEngine_->shutdown();
        pImpl_->aiEngine_.reset();
    }
}

void GameEngine::shutdownNetworkManager() {
    if (pImpl_->networkManager_) {
        pImpl_->networkManager_->shutdown();
        pImpl_->networkManager_.reset();
    }
}

void GameEngine::shutdownUIManager() {
    if (pImpl_->uiManager_) {
        pImpl_->uiManager_->shutdown();
        pImpl_->uiManager_.reset();
    }
}

std::string GameEngine::getEventName(GameEventType type) {
    switch (type) {
        case GameEventType::STATE_CHANGED: return "StateChanged";
        case GameEventType::LEVEL_LOADED: return "LevelLoaded";
        case GameEventType::GAME_SAVED: return "GameSaved";
        case GameEventType::GAME_LOADED: return "GameLoaded";
        case GameEventType::CHARACTER_CREATED: return "CharacterCreated";
        case GameEventType::CHARACTER_DIED: return "CharacterDied";
        case GameEventType::QUEST_STARTED: return "QuestStarted";
        case GameEventType::QUEST_COMPLETED: return "QuestCompleted";
        case GameEventType::ITEM_ACQUIRED: return "ItemAcquired";
        case GameEventType::SKILL_LEARNED: return "SkillLearned";
        case GameEventType::BATTLE_STARTED: return "BattleStarted";
        case GameEventType::BATTLE_ENDED: return "BattleEnded";
        case GameEventType::DIALOGUE_STARTED: return "DialogueStarted";
        case GameEventType::DIALOGUE_ENDED: return "DialogueEnded";
        case GameEventType::INVENTORY_OPENED: return "InventoryOpened";
        case GameEventType::INVENTORY_CLOSED: return "InventoryClosed";
        case GameEventType::SETTINGS_CHANGED: return "SettingsChanged";
        case GameEventType::ERROR_OCCURRED: return "ErrorOccurred";
        default: return "UnknownEvent";
    }
}

} // namespace Fantasy