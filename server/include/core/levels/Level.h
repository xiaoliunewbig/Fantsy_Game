/**
 * @file Level.h
 * @brief 关卡系统 - 游戏关卡管理
 * @author [pengchengkang]
 * @date 2025.06.17
 * 
 * 功能特性:
 * - 关卡加载和卸载
 * - 地图管理
 * - 实体管理
 * - 碰撞检测
 * - 事件系统
 * - 关卡状态管理
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <functional>
#include <variant>
#include <optional>
#include <cmath>

namespace Fantasy {

// 前向声明
class Character;

// 关卡类型枚举
enum class LevelType {
    TUTORIAL,       // 教程关卡
    VILLAGE,        // 村庄
    FOREST,         // 森林
    CAVE,           // 洞穴
    DUNGEON,        // 地下城
    BOSS_ARENA,     // Boss竞技场
    TOWN,           // 城镇
    CASTLE,         // 城堡
    WILDERNESS,     // 荒野
    SPECIAL_EVENT   // 特殊事件
};

// 关卡状态枚举
enum class LevelState {
    LOADING,        // 加载中
    LOADED,         // 已加载
    ACTIVE,         // 活跃
    PAUSED,         // 暂停
    COMPLETED,      // 完成
    FAILED,         // 失败
    TRANSITIONING   // 过渡中
};

// 地形类型枚举
enum class TerrainType {
    GRASS,          // 草地
    WATER,          // 水
    SAND,           // 沙子
    ROCK,           // 岩石
    ICE,            // 冰
    LAVA,           // 岩浆
    BRIDGE,         // 桥
    WALL,           // 墙
    DOOR,           // 门
    TELEPORTER      // 传送门
};

// 实体类型枚举
enum class EntityType {
    PLAYER,         // 玩家
    NPC,            // NPC
    ENEMY,          // 敌人
    BOSS,           // Boss
    ITEM,           // 物品
    TRIGGER,        // 触发器
    DECORATION,     // 装饰
    EFFECT,         // 特效
    PROJECTILE,     // 投射物
    COLLECTIBLE     // 收集品
};

// 位置结构
struct Vector2D {
    float x = 0.0f;
    float y = 0.0f;
    
    Vector2D() = default;
    Vector2D(float x, float y) : x(x), y(y) {}
    
    Vector2D operator+(const Vector2D& other) const { return Vector2D(x + other.x, y + other.y); }
    Vector2D operator-(const Vector2D& other) const { return Vector2D(x - other.x, y - other.y); }
    Vector2D operator*(float scalar) const { return Vector2D(x * scalar, y * scalar); }
    Vector2D operator/(float scalar) const { return Vector2D(x / scalar, y / scalar); }
    
    float length() const { return std::sqrt(x * x + y * y); }
    Vector2D normalized() const { float len = length(); return len > 0 ? *this / len : Vector2D(); }
    
    float distance(const Vector2D& other) const {
        Vector2D diff = *this - other;
        return diff.length();
    }
};

// 矩形结构
struct Rectangle {
    float x, y, width, height;
    
    Rectangle() : x(0), y(0), width(0), height(0) {}
    Rectangle(float x, float y, float w, float h) : x(x), y(y), width(w), height(h) {}
    
    bool contains(const Vector2D& point) const {
        return point.x >= x && point.x <= x + width && 
               point.y >= y && point.y <= y + height;
    }
    
    bool intersects(const Rectangle& other) const {
        return x < other.x + other.width && x + width > other.x &&
               y < other.y + other.height && y + height > other.y;
    }
};

// 地形块结构
struct TerrainTile {
    TerrainType type;
    Vector2D position;
    Rectangle bounds;
    bool walkable;
    bool swimable;
    bool flyable;
    float movementCost;
    
    TerrainTile() : type(TerrainType::GRASS), position(0, 0), bounds(0, 0, 32, 32), 
                   walkable(true), swimable(false), flyable(true), movementCost(1.0f) {}
    TerrainTile(TerrainType t, const Vector2D& pos, float w, float h)
        : type(t), position(pos), bounds(pos.x, pos.y, w, h), walkable(true), 
          swimable(false), flyable(true), movementCost(1.0f) {}
    TerrainTile(TerrainType t, const Vector2D& pos, float w, float h, bool walk, bool swim, bool fly, float cost)
        : type(t), position(pos), bounds(pos.x, pos.y, w, h), walkable(walk), 
          swimable(swim), flyable(fly), movementCost(cost) {}
};

// 实体基类
class Entity {
public:
    Entity(const std::string& id, EntityType type, const Vector2D& position);
    virtual ~Entity() = default;
    
    // 基础操作
    virtual void update(double deltaTime) = 0;
    virtual void render() = 0;
    
    // 获取器
    const std::string& getId() const { return id_; }
    EntityType getType() const { return type_; }
    const Vector2D& getPosition() const { return position_; }
    const Rectangle& getBounds() const { return bounds_; }
    bool isActive() const { return active_; }
    bool isVisible() const { return visible_; }
    
    // 设置器
    virtual void setPosition(const Vector2D& position);
    virtual void setActive(bool active) { active_ = active; }
    virtual void setVisible(bool visible) { visible_ = visible; }
    
protected:
    std::string id_;
    EntityType type_;
    Vector2D position_;
    Rectangle bounds_;
    bool active_;
    bool visible_;
};

// 关卡事件类型
enum class LevelEventType {
    LEVEL_LOADED,
    LEVEL_UNLOADED,
    ENTITY_SPAWNED,
    ENTITY_DESTROYED,
    PLAYER_ENTERED,
    PLAYER_LEFT,
    OBJECTIVE_COMPLETED,
    OBJECTIVE_FAILED,
    TRIGGER_ACTIVATED,
    COLLISION_DETECTED
};

// 关卡事件数据
using LevelEventData = std::unordered_map<std::string, std::string>;

// 关卡事件结构
struct LevelEvent {
    LevelEventType type;
    std::string name;
    LevelEventData data;
    std::chrono::system_clock::time_point timestamp;
    
    LevelEvent(LevelEventType t, const std::string& n, const LevelEventData& d = LevelEventData{})
        : type(t), name(n), data(d), timestamp(std::chrono::system_clock::now()) {}
};

// 事件回调函数类型
using LevelEventCallback = std::function<void(const LevelEvent&)>;

// 关卡配置结构
struct LevelConfig {
    std::string id;
    std::string name;
    std::string description;
    LevelType type;
    int width;
    int height;
    int tileSize;
    std::string backgroundMusic;
    std::string ambientSound;
    std::vector<std::string> requiredItems;
    std::vector<std::string> objectives;
    int timeLimit;  // 时间限制（秒），0表示无限制
    int maxPlayers;
    bool allowRespawn;
    
    LevelConfig() : type(LevelType::VILLAGE), width(100), height(100), tileSize(32),
                   timeLimit(0), maxPlayers(1), allowRespawn(true) {}
};

// 关卡统计结构
struct LevelStats {
    int entitiesSpawned;
    int entitiesDestroyed;
    int objectivesCompleted;
    int objectivesFailed;
    double timeSpent;
    int playerDeaths;
    int itemsCollected;
    int enemiesDefeated;
    int totalEvents;
    double updateTime;
    int totalUpdates;
    int totalEntities;
    
    LevelStats() : entitiesSpawned(0), entitiesDestroyed(0), objectivesCompleted(0),
                  objectivesFailed(0), timeSpent(0.0), playerDeaths(0), 
                  itemsCollected(0), enemiesDefeated(0), totalEvents(0),
                  updateTime(0.0), totalUpdates(0), totalEntities(0) {}
};

// 日志宏定义
#define LEVEL_LOG_DEBUG(fmt, ...) 
#define LEVEL_LOG_INFO(fmt, ...) 
#define LEVEL_LOG_WARN(fmt, ...) 
#define LEVEL_LOG_ERROR(fmt, ...) 

/**
 * @brief 关卡类
 * 
 * 提供关卡的基础功能：
 * - 关卡加载和卸载
 * - 实体管理
 * - 碰撞检测
 * - 事件系统
 * - 地形管理
 */
class Level {
public:
    // 构造和析构
    Level(const LevelConfig& config);
    virtual ~Level();
    
    // 禁用拷贝
    Level(const Level&) = delete;
    Level& operator=(const Level&) = delete;
    
    // 关卡生命周期
    virtual bool load();
    virtual void unload();
    virtual void update(double deltaTime);
    virtual void render();
    
    // 状态管理
    virtual void pause();
    virtual void resume();
    virtual void reset();
    virtual bool isCompleted() const;
    virtual bool isFailed() const;
    
    // 实体管理
    virtual bool addEntity(std::shared_ptr<Entity> entity);
    virtual bool removeEntity(const std::string& entityId);
    virtual std::shared_ptr<Entity> getEntity(const std::string& entityId) const;
    virtual std::vector<std::shared_ptr<Entity>> getEntitiesByType(EntityType type) const;
    virtual std::vector<std::shared_ptr<Entity>> getEntitiesInArea(const Rectangle& area) const;
    virtual void clearEntities();
    
    // 角色管理
    virtual bool addPlayer(std::shared_ptr<Character> player);
    virtual bool removePlayer(const std::string& playerId);
    virtual std::shared_ptr<Character> getPlayer(const std::string& playerId) const;
    virtual std::vector<std::shared_ptr<Character>> getAllPlayers() const;
    
    // 地形管理
    virtual void setTerrainTile(int x, int y, const TerrainTile& tile);
    virtual TerrainTile getTerrainTile(int x, int y) const;
    virtual bool isWalkable(const Vector2D& position) const;
    virtual bool isSwimable(const Vector2D& position) const;
    virtual bool isFlyable(const Vector2D& position) const;
    virtual float getMovementCost(const Vector2D& position) const;
    virtual TerrainType getTerrainType(const Vector2D& position) const;
    virtual void setTerrainType(const Vector2D& position, TerrainType type);
    
    // 碰撞检测
    virtual bool checkCollision(const Rectangle& bounds) const;
    virtual bool checkCollision(const Vector2D& position, float radius) const;
    virtual bool checkCollision(const Entity& entity1, const Entity& entity2) const;
    virtual std::vector<std::shared_ptr<Entity>> getCollidingEntities(const Rectangle& bounds) const;
    virtual std::vector<std::shared_ptr<Entity>> getCollidingEntities(const Vector2D& position, float radius) const;
    
    // 路径查找
    virtual std::vector<Vector2D> findPath(const Vector2D& start, const Vector2D& end) const;
    virtual bool hasLineOfSight(const Vector2D& start, const Vector2D& end) const;
    
    // 事件系统
    virtual void emitEvent(const LevelEvent& event);
    virtual void emitEvent(LevelEventType type, const std::string& name, const LevelEventData& data = LevelEventData{});
    virtual void subscribeToEvent(LevelEventType type, LevelEventCallback callback);
    virtual void unsubscribeFromEvent(LevelEventType type, LevelEventCallback callback);
    
    // 目标系统
    virtual bool addObjective(const std::string& objectiveId, const std::string& description);
    virtual bool completeObjective(const std::string& objectiveId);
    virtual bool failObjective(const std::string& objectiveId);
    virtual bool isObjectiveCompleted(const std::string& objectiveId) const;
    virtual std::vector<std::string> getActiveObjectives() const;
    
    // 触发器系统
    virtual bool addTrigger(const std::string& triggerId, const Rectangle& area, 
                          std::function<void()> callback);
    virtual bool removeTrigger(const std::string& triggerId);
    virtual void checkTriggers(const Vector2D& position);
    
    // 获取器
    const LevelConfig& getConfig() const { return config_; }
    const LevelStats& getStats() const { return stats_; }
    LevelState getState() const { return state_; }
    int getWidth() const { return config_.width; }
    int getHeight() const { return config_.height; }
    int getTileSize() const { return config_.tileSize; }
    const std::string& getId() const { return config_.id; }
    const std::string& getName() const { return config_.name; }
    const std::string& getDescription() const { return config_.description; }
    LevelType getType() const { return config_.type; }
    
    // 设置器
    virtual void setState(LevelState state) { state_ = state; }
    virtual void setConfig(const LevelConfig& config) { config_ = config; }

    // 生命周期事件
    virtual void onLoaded();
    virtual void onUnloaded();
    virtual void onEntitySpawned(std::shared_ptr<Entity> entity);
    virtual void onEntityDestroyed(const std::string& entityId);
    virtual void onPlayerEntered(std::shared_ptr<Character> player);
    virtual void onPlayerLeft(const std::string& playerId);
    virtual void onObjectiveCompleted(const std::string& objectiveId);
    virtual void onObjectiveFailed(const std::string& objectiveId);
    virtual void onTriggerActivated(const std::string& triggerId);

protected:
    // 私有方法
    void initializeTerrain();
    void updateEntities(double deltaTime);
    void updatePlayers(double deltaTime);
    void updateTriggers();
    void processEvents();
    void updateStats(double deltaTime);
    bool isValidPosition(const Vector2D& position) const;
    int getTileIndex(const Vector2D& position) const;
    
    // 新增方法
    bool initTerrain();
    void loadEntities();
    void initCollisionSystem();
    void updateCollisions();
    void cleanupEntities();
    void renderTerrain();
    void renderUI();
    bool isEntityInArea(const Entity& entity, const Rectangle& area) const;
    bool isTerrainWalkable(TerrainType type);
    void generateForestTerrain();
    void generateCaveTerrain();
    void generateDungeonTerrain();
    void generateCityTerrain();
    void generateDefaultTerrain();

private:
    // 配置和状态
    LevelConfig config_;
    LevelState state_;
    LevelStats stats_;
    
    // 地形系统
    std::vector<std::vector<TerrainTile>> terrain_;
    
    // 实体系统
    std::unordered_map<std::string, std::shared_ptr<Entity>> entities_;
    std::unordered_map<std::string, std::shared_ptr<Character>> players_;
    
    // 目标系统
    struct Objective {
        std::string description;
        bool completed;
        
        Objective() : description(""), completed(false) {}
        Objective(const std::string& desc, bool comp = false) 
            : description(desc), completed(comp) {}
    };
    std::unordered_map<std::string, Objective> objectives_;
    
    // 触发器系统
    struct Trigger {
        std::string id;
        Rectangle area;
        std::function<void()> callback;
        bool activated;
        
        Trigger() : id(""), area(0, 0, 0, 0), callback(nullptr), activated(false) {}
        Trigger(const std::string& id, const Rectangle& area, std::function<void()> callback)
            : id(id), area(area), callback(callback), activated(false) {}
    };
    std::unordered_map<std::string, Trigger> triggers_;
    
    // 事件系统
    std::unordered_map<LevelEventType, std::vector<LevelEventCallback>> eventCallbacks_;
    std::vector<LevelEvent> eventQueue_;
    
    // 内部状态
    bool initialized_;
    std::chrono::system_clock::time_point startTime_;
    std::chrono::system_clock::time_point lastUpdateTime_;
};

} // namespace Fantasy
