/**
 * @file GameEngine.h
 * @brief 游戏引擎核心类 - 管理游戏主循环和系统
 * @author [pengchengkang]
 * @date 2025.06.17
 * 
 * 功能特性:
 * - 游戏状态管理
 * - 主循环控制
 * - 场景管理
 * - 事件系统
 * - 数据持久化
 * - 系统集成
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <variant>
#include <optional>

// 前向声明
namespace Fantasy {
    class Character;
    class Level;
    class UIManager;
    class ResourceManager;
    class GameConfigManager;
    class DatabaseManager;
    class EventSystem;
    class InputManager;
    class AudioManager;
    class RenderManager;
    class PhysicsEngine;
    class AIEngine;
    class NetworkManager;
}

namespace Fantasy {

// 游戏状态枚举
enum class GameState {
    INITIALIZING,   // 初始化中
    MENU,           // 主菜单
    LOADING,        // 加载中
    PLAYING,        // 游戏中
    PAUSED,         // 暂停
    BATTLE,         // 战斗中
    DIALOGUE,       // 对话中
    INVENTORY,      // 背包界面
    SETTINGS,       // 设置界面
    SAVING,         // 保存中
    LOADING_GAME,   // 加载游戏
    QUITTING        // 退出中
};

// 游戏事件类型
enum class GameEventType {
    STATE_CHANGED,
    LEVEL_LOADED,
    GAME_SAVED,
    GAME_LOADED,
    CHARACTER_CREATED,
    CHARACTER_DIED,
    QUEST_STARTED,
    QUEST_COMPLETED,
    ITEM_ACQUIRED,
    SKILL_LEARNED,
    BATTLE_STARTED,
    BATTLE_ENDED,
    DIALOGUE_STARTED,
    DIALOGUE_ENDED,
    INVENTORY_OPENED,
    INVENTORY_CLOSED,
    SETTINGS_CHANGED,
    ERROR_OCCURRED
};

// 游戏事件数据
using GameEventData = std::variant<
    std::string,                    // 字符串数据
    int,                           // 整数数据
    double,                        // 浮点数数据
    bool,                          // 布尔数据
    std::vector<std::string>,      // 字符串数组
    std::unordered_map<std::string, std::string>  // 键值对数据
>;

// 游戏事件结构
struct GameEvent {
    GameEventType type;
    std::string name;
    GameEventData data;
    std::chrono::system_clock::time_point timestamp;
    
    GameEvent(GameEventType t, const std::string& n, const GameEventData& d = std::string{})
        : type(t), name(n), data(d), timestamp(std::chrono::system_clock::now()) {}
};

// 游戏配置
struct GameConfig {
    std::string gameTitle = "Fantasy Legend";
    std::string version = "1.0.0";
    int targetFPS = 60;
    bool enableVSync = true;
    bool enableFullscreen = false;
    int windowWidth = 1280;
    int windowHeight = 720;
    std::string defaultLanguage = "zh_CN";
    bool enableDebugMode = false;
    bool enableProfiling = false;
    std::string saveDirectory = "saves";
    std::string logDirectory = "logs";
    std::string resourceDirectory = "resources";
    std::string configDirectory = "config";
};

// 游戏统计信息
struct GameStats {
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point lastUpdateTime;
    uint64_t totalFrames;
    uint64_t totalUpdateTime;
    double averageFPS;
    double currentFPS;
    size_t memoryUsage;
    size_t peakMemoryUsage;
    uint64_t totalEvents;
    uint64_t totalErrors;
    
    GameStats() : totalFrames(0), totalUpdateTime(0), averageFPS(0.0), 
                  currentFPS(0.0), memoryUsage(0), peakMemoryUsage(0),
                  totalEvents(0), totalErrors(0) {
        startTime = std::chrono::system_clock::now();
        lastUpdateTime = startTime;
    }
};

// 事件回调函数类型
using EventCallback = std::function<void(const GameEvent&)>;

// 基础系统接口类
class ResourceManager {
public:
    virtual ~ResourceManager() = default;
    virtual bool Init() = 0;
    virtual void shutdown() = 0;
};

class DatabaseManager {
public:
    virtual ~DatabaseManager() = default;
    virtual bool Init() = 0;
    virtual void shutdown() = 0;
};

class EventSystem {
public:
    virtual ~EventSystem() = default;
    virtual bool Init() = 0;
    virtual void shutdown() = 0;
};

class InputManager {
public:
    virtual ~InputManager() = default;
    virtual bool Init() = 0;
    virtual void shutdown() = 0;
};

class AudioManager {
public:
    virtual ~AudioManager() = default;
    virtual bool Init() = 0;
    virtual void shutdown() = 0;
};

class RenderManager {
public:
    virtual ~RenderManager() = default;
    virtual bool Init() = 0;
    virtual void shutdown() = 0;
};

class PhysicsEngine {
public:
    virtual ~PhysicsEngine() = default;
    virtual bool Init() = 0;
    virtual void shutdown() = 0;
};

class AIEngine {
public:
    virtual ~AIEngine() = default;
    virtual bool Init() = 0;
    virtual void shutdown() = 0;
};

class NetworkManager {
public:
    virtual ~NetworkManager() = default;
    virtual bool Init() = 0;
    virtual void shutdown() = 0;
};

class UIManager {
public:
    virtual ~UIManager() = default;
    virtual bool Init() = 0;
    virtual void shutdown() = 0;
};

/**
 * @brief 游戏引擎主类
 * 
 * 负责管理游戏的核心功能：
 * - 游戏状态管理
 * - 主循环控制
 * - 系统集成
 * - 事件处理
 * - 数据持久化
 */
class GameEngine {
public:
    // 单例模式
    static GameEngine& getInstance();
    
    // 禁用拷贝和赋值
    GameEngine(const GameEngine&) = delete;
    GameEngine& operator=(const GameEngine&) = delete;
    
    // 初始化和清理
    bool Init(const GameConfig& config = GameConfig{});
    void shutdown();
    bool isInit() const;
    
    // 游戏主循环控制
    void start();
    void stop();
    void pause();
    void resume();
    void update();
    
    // 游戏状态管理
    void setState(GameState state);
    GameState getState() const;
    bool isState(GameState state) const;
    
    // 场景管理
    bool loadLevel(const std::string& levelId);
    bool unloadLevel();
    std::shared_ptr<Level> getCurrentLevel() const;
    
    // 角色管理
    std::shared_ptr<Character> getPlayer() const;
    bool setPlayer(std::shared_ptr<Character> player);
    std::vector<std::shared_ptr<Character>> getAllCharacters() const;
    
    // 事件系统
    void emitEvent(const GameEvent& event);
    void emitEvent(GameEventType type, const std::string& name, const GameEventData& data = std::string{});
    void subscribeToEvent(GameEventType type, EventCallback callback);
    void unsubscribeFromEvent(GameEventType type, EventCallback callback);
    
    // 数据持久化
    bool saveGame(const std::string& slotName);
    bool loadGame(const std::string& slotName);
    bool deleteGame(const std::string& slotName);
    std::vector<std::string> getSaveSlots() const;
    
    // 系统访问器
    std::shared_ptr<ResourceManager> getResourceManager() const;
    std::shared_ptr<GameConfigManager> getConfigManager() const;
    std::shared_ptr<DatabaseManager> getDatabaseManager() const;
    std::shared_ptr<EventSystem> getEventSystem() const;
    std::shared_ptr<InputManager> getInputManager() const;
    std::shared_ptr<AudioManager> getAudioManager() const;
    std::shared_ptr<RenderManager> getRenderManager() const;
    std::shared_ptr<PhysicsEngine> getPhysicsEngine() const;
    std::shared_ptr<AIEngine> getAIEngine() const;
    std::shared_ptr<NetworkManager> getNetworkManager() const;
    std::shared_ptr<UIManager> getUIManager() const;
    
    // 配置管理
    const GameConfig& getConfig() const;
    void setConfig(const GameConfig& config);
    
    // 统计信息
    const GameStats& getStats() const;
    void resetStats();
    
    // 工具方法
    double getDeltaTime() const;
    uint64_t getFrameCount() const;
    double getFPS() const;
    size_t getMemoryUsage() const;
    std::string getVersion() const;
    
    // 调试和性能
    void enableDebugMode(bool enable);
    bool isDebugMode() const;
    void enableProfiling(bool enable);
    bool isProfiling() const;
    void logPerformance(const std::string& operation, double timeMs);

private:
    GameEngine();
    ~GameEngine();
    
    // 内部实现
    class Impl;
    std::unique_ptr<Impl> pImpl_;
    
    // 私有方法
    void mainLoop();
    void updateSystems(double deltaTime);
    void renderSystems();
    void processEvents();
    void updateStats();
    void cleanupResources();
    
    // 系统初始化
    bool InitSystems();   //InitSystems
    bool InitResourceManager();
    bool InitConfigManager();
    bool InitDatabaseManager();
    bool InitEventSystem();
    bool InitInputManager();
    bool InitAudioManager();
    bool InitRenderManager();
    bool InitPhysicsEngine();
    bool InitAIEngine();
    bool InitNetworkManager();
    bool InitUIManager();
    
    // 系统清理
    void shutdownSystems();
    void shutdownResourceManager();
    void shutdownConfigManager();
    void shutdownDatabaseManager();
    void shutdownEventSystem();
    void shutdownInputManager();
    void shutdownAudioManager();
    void shutdownRenderManager();
    void shutdownPhysicsEngine();
    void shutdownAIEngine();
    void shutdownNetworkManager();
    void shutdownUIManager();
};

// 游戏引擎日志宏
#define GAME_LOG_TRACE(msg, ...) \
    FANTASY_LOG_TRACE("[GameEngine] " msg, ##__VA_ARGS__)

#define GAME_LOG_DEBUG(msg, ...) \
    FANTASY_LOG_DEBUG("[GameEngine] " msg, ##__VA_ARGS__)

#define GAME_LOG_INFO(msg, ...) \
    FANTASY_LOG_INFO("[GameEngine] " msg, ##__VA_ARGS__)

#define GAME_LOG_WARN(msg, ...) \
    FANTASY_LOG_WARN("[GameEngine] " msg, ##__VA_ARGS__)

#define GAME_LOG_ERROR(msg, ...) \
    FANTASY_LOG_ERROR("[GameEngine] " msg, ##__VA_ARGS__)

} // namespace Fantasy