/**
 * @file Level.cpp
 * @brief 关卡系统实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "core/levels/Level.h"
#include "core/characters/Character.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <random>
#include <cmath>

namespace Fantasy {

// Entity构造函数
Entity::Entity(const std::string& id, EntityType type, const Vector2D& position)
    : id_(id), type_(type), position_(position), active_(true), visible_(true) {
    bounds_ = Rectangle(position.x, position.y, 32.0f, 32.0f); // 默认大小
}

// Entity设置位置
void Entity::setPosition(const Vector2D& position) {
    position_ = position;
    bounds_.x = position.x;
    bounds_.y = position.y;
}

// Level构造函数
Level::Level(const LevelConfig& config)
    : config_(config), state_(LevelState::LOADING), initialized_(false) {
    
    LEVEL_LOG_INFO("创建关卡: %s", config_.name.c_str());
    
    // 初始化地形
    terrain_.resize(config_.height);
    for (auto& row : terrain_) {
        row.resize(config_.width);
    }
    
    // 初始化统计
    stats_ = LevelStats{};
    startTime_ = std::chrono::system_clock::now();
    
    // 初始化地形
    initializeTerrain();
}

// Level析构函数
Level::~Level() {
    LEVEL_LOG_INFO("销毁关卡: %s", config_.name.c_str());
    unload();
}

// 加载关卡
bool Level::load() {
    LEVEL_LOG_INFO("加载关卡: %s", config_.name.c_str());
    
    if (state_ != LevelState::LOADING) {
        LEVEL_LOG_WARN("关卡已经加载或正在加载");
        return false;
    }
    
    try {
        state_ = LevelState::LOADING;
        
        // 初始化地形
        if (!initTerrain()) {
            LEVEL_LOG_ERROR("初始化地形失败");
            return false;
        }
        
        // 加载实体
        loadEntities();
        
        // 初始化碰撞系统
        initCollisionSystem();
        
        // 设置状态
        state_ = LevelState::LOADED;
        initialized_ = true;
        
        // 发送事件
        emitEvent(LevelEventType::LEVEL_LOADED, "LevelLoaded");
        
        LEVEL_LOG_INFO("关卡加载成功: %s", config_.name.c_str());
        return true;
        
    } catch (const std::exception& e) {
        LEVEL_LOG_ERROR("加载关卡时发生异常: %s", e.what());
        state_ = LevelState::FAILED;
        return false;
    }
}

// 卸载关卡
void Level::unload() {
    LEVEL_LOG_INFO("卸载关卡: %s", config_.name.c_str());
    
    if (state_ == LevelState::LOADING) {
        return;
    }
    
    try {
        // 清理实体
        cleanupEntities();
        
        // 清理触发器
        triggers_.clear();
        
        // 清理事件
        eventQueue_.clear();
        eventCallbacks_.clear();
        
        // 重置状态
        state_ = LevelState::LOADING;
        initialized_ = false;
        
        // 发送事件
        emitEvent(LevelEventType::LEVEL_UNLOADED, "LevelUnloaded");
        
        LEVEL_LOG_INFO("关卡卸载完成: %s", config_.name.c_str());
        
    } catch (const std::exception& e) {
        LEVEL_LOG_ERROR("卸载关卡时发生异常: %s", e.what());
    }
}

// 更新关卡
void Level::update(double deltaTime) {
    if (state_ != LevelState::ACTIVE && state_ != LevelState::LOADED) {
        return;
    }
    
    // 更新统计
    stats_.timeSpent += deltaTime;
    stats_.totalEvents++;
    
    // 更新实体
    updateEntities(deltaTime);
    
    // 更新玩家
    updatePlayers(deltaTime);
    
    // 更新触发器
    updateTriggers();
    
    // 更新碰撞
    updateCollisions();
    
    // 处理事件
    processEvents();
    
    // 更新统计
    updateStats(deltaTime);
    
    lastUpdateTime_ = std::chrono::system_clock::now();
}

// 渲染关卡
void Level::render() {
    if (!initialized_) {
        return;
    }
    
    // 渲染地形
    renderTerrain();
    
    // 渲染实体
    for (const auto& pair : entities_) {
        if (pair.second && pair.second->isVisible()) {
            pair.second->render();
        }
    }
    
    // 渲染UI
    renderUI();
}

// 暂停关卡
void Level::pause() {
    if (state_ == LevelState::ACTIVE) {
        state_ = LevelState::PAUSED;
        LEVEL_LOG_INFO("关卡暂停: %s", config_.name.c_str());
    }
}

// 恢复关卡
void Level::resume() {
    if (state_ == LevelState::PAUSED) {
        state_ = LevelState::ACTIVE;
        LEVEL_LOG_INFO("关卡恢复: %s", config_.name.c_str());
    }
}

// 重置关卡
void Level::reset() {
    LEVEL_LOG_INFO("重置关卡: %s", config_.name.c_str());
    
    // 卸载当前关卡
    unload();
    
    // 重新加载
    load();
}

// 检查是否完成
bool Level::isCompleted() const {
    return state_ == LevelState::COMPLETED;
}

// 检查是否失败
bool Level::isFailed() const {
    return state_ == LevelState::FAILED;
}

// 添加实体
bool Level::addEntity(std::shared_ptr<Entity> entity) {
    if (!entity) {
        return false;
    }
    
    if (entities_.find(entity->getId()) != entities_.end()) {
        LEVEL_LOG_WARN("实体已存在: %s", entity->getId().c_str());
        return false;
    }
    
    entities_[entity->getId()] = entity;
    stats_.entitiesSpawned++;
    
    LEVEL_LOG_DEBUG("添加实体: %s", entity->getId().c_str());
    
    // 发送事件
    emitEvent(LevelEventType::ENTITY_SPAWNED, "EntitySpawned", 
              {{"entityId", entity->getId()}});
    
    onEntitySpawned(entity);
    
    return true;
}

// 移除实体
bool Level::removeEntity(const std::string& entityId) {
    auto it = entities_.find(entityId);
    if (it == entities_.end()) {
        return false;
    }
    
    entities_.erase(it);
    stats_.entitiesDestroyed++;
    
    LEVEL_LOG_DEBUG("移除实体: %s", entityId.c_str());
    
    // 发送事件
    emitEvent(LevelEventType::ENTITY_DESTROYED, "EntityDestroyed", 
              {{"entityId", entityId}});
    
    onEntityDestroyed(entityId);
    
    return true;
}

// 获取实体
std::shared_ptr<Entity> Level::getEntity(const std::string& entityId) const {
    auto it = entities_.find(entityId);
    return it != entities_.end() ? it->second : nullptr;
}

// 按类型获取实体
std::vector<std::shared_ptr<Entity>> Level::getEntitiesByType(EntityType type) const {
    std::vector<std::shared_ptr<Entity>> result;
    
    for (const auto& pair : entities_) {
        if (pair.second && pair.second->getType() == type) {
            result.push_back(pair.second);
        }
    }
    
    return result;
}

// 获取区域内的实体
std::vector<std::shared_ptr<Entity>> Level::getEntitiesInArea(const Rectangle& area) const {
    std::vector<std::shared_ptr<Entity>> result;
    
    for (const auto& pair : entities_) {
        if (pair.second && isEntityInArea(*pair.second, area)) {
            result.push_back(pair.second);
        }
    }
    
    return result;
}

// 清空实体
void Level::clearEntities() {
    entities_.clear();
    LEVEL_LOG_DEBUG("清空所有实体");
}

// 添加玩家
bool Level::addPlayer(std::shared_ptr<Character> player) {
    if (!player) {
        return false;
    }
    
    if (players_.find(player->getName()) != players_.end()) {
        LEVEL_LOG_WARN("玩家已存在: %s", player->getName().c_str());
        return false;
    }
    
    players_[player->getName()] = player;
    
    LEVEL_LOG_DEBUG("添加玩家: %s", player->getName().c_str());
    
    // 发送事件
    emitEvent(LevelEventType::PLAYER_ENTERED, "PlayerEntered", 
              {{"playerId", player->getName()}});
    
    onPlayerEntered(player);
    
    return true;
}

// 移除玩家
bool Level::removePlayer(const std::string& playerId) {
    auto it = players_.find(playerId);
    if (it == players_.end()) {
        return false;
    }
    
    players_.erase(it);
    
    LEVEL_LOG_DEBUG("移除玩家: %s", playerId.c_str());
    
    // 发送事件
    emitEvent(LevelEventType::PLAYER_LEFT, "PlayerLeft", 
              {{"playerId", playerId}});
    
    onPlayerLeft(playerId);
    
    return true;
}

// 获取玩家
std::shared_ptr<Character> Level::getPlayer(const std::string& playerId) const {
    auto it = players_.find(playerId);
    return it != players_.end() ? it->second : nullptr;
}

// 获取所有玩家
std::vector<std::shared_ptr<Character>> Level::getAllPlayers() const {
    std::vector<std::shared_ptr<Character>> result;
    
    for (const auto& pair : players_) {
        if (pair.second) {
            result.push_back(pair.second);
        }
    }
    
    return result;
}

// 设置地形块
void Level::setTerrainTile(int x, int y, const TerrainTile& tile) {
    if (x >= 0 && x < config_.width && y >= 0 && y < config_.height) {
        terrain_[y][x] = tile;
    }
}

// 获取地形块
TerrainTile Level::getTerrainTile(int x, int y) const {
    if (x >= 0 && x < config_.width && y >= 0 && y < config_.height) {
        return terrain_[y][x];
    }
    return TerrainTile();
}

// 检查是否可步行
bool Level::isWalkable(const Vector2D& position) const {
    int x = static_cast<int>(position.x / config_.tileSize);
    int y = static_cast<int>(position.y / config_.tileSize);
    
    if (x < 0 || x >= config_.width || y < 0 || y >= config_.height) {
        return false;
    }
    
    return terrain_[y][x].walkable;
}

// 检查是否可游泳
bool Level::isSwimable(const Vector2D& position) const {
    int x = static_cast<int>(position.x / config_.tileSize);
    int y = static_cast<int>(position.y / config_.tileSize);
    
    if (x < 0 || x >= config_.width || y < 0 || y >= config_.height) {
        return false;
    }
    
    return terrain_[y][x].swimable;
}

// 检查是否可飞行
bool Level::isFlyable(const Vector2D& position) const {
    int x = static_cast<int>(position.x / config_.tileSize);
    int y = static_cast<int>(position.y / config_.tileSize);
    
    if (x < 0 || x >= config_.width || y < 0 || y >= config_.height) {
        return false;
    }
    
    return terrain_[y][x].flyable;
}

// 获取移动成本
float Level::getMovementCost(const Vector2D& position) const {
    int x = static_cast<int>(position.x / config_.tileSize);
    int y = static_cast<int>(position.y / config_.tileSize);
    
    if (x < 0 || x >= config_.width || y < 0 || y >= config_.height) {
        return 999.0f; // 不可移动
    }
    
    return terrain_[y][x].movementCost;
}

// 获取地形类型
TerrainType Level::getTerrainType(const Vector2D& position) const {
    int x = static_cast<int>(position.x / config_.tileSize);
    int y = static_cast<int>(position.y / config_.tileSize);
    
    if (x < 0 || x >= config_.width || y < 0 || y >= config_.height) {
        return TerrainType::ROCK; // 边界返回岩石
    }
    
    return terrain_[y][x].type;
}

// 设置地形类型
void Level::setTerrainType(const Vector2D& position, TerrainType type) {
    int x = static_cast<int>(position.x / config_.tileSize);
    int y = static_cast<int>(position.y / config_.tileSize);
    
    if (x >= 0 && x < config_.width && y >= 0 && y < config_.height) {
        terrain_[y][x].type = type;
        terrain_[y][x].walkable = isTerrainWalkable(type);
        
        // 发送事件
        emitEvent(LevelEventType::TRIGGER_ACTIVATED, "TerrainChanged",
                  {{"x", std::to_string(x)}, {"y", std::to_string(y)}, 
                   {"type", std::to_string(static_cast<int>(type))}});
    }
}

// 检查碰撞
bool Level::checkCollision(const Rectangle& bounds) const {
    // 检查地形碰撞
    if (!isWalkable(Vector2D(bounds.x, bounds.y)) ||
        !isWalkable(Vector2D(bounds.x + bounds.width, bounds.y)) ||
        !isWalkable(Vector2D(bounds.x, bounds.y + bounds.height)) ||
        !isWalkable(Vector2D(bounds.x + bounds.width, bounds.y + bounds.height))) {
        return true;
    }
    
    // 检查实体碰撞
    for (const auto& pair : entities_) {
        if (pair.second && bounds.intersects(pair.second->getBounds())) {
            return true;
        }
    }
    
    return false;
}

// 检查碰撞（圆形）
bool Level::checkCollision(const Vector2D& position, float radius) const {
    Rectangle bounds(position.x - radius, position.y - radius, radius * 2, radius * 2);
    return checkCollision(bounds);
}

// 检查实体碰撞
bool Level::checkCollision(const Entity& entity1, const Entity& entity2) const {
    return entity1.getBounds().intersects(entity2.getBounds());
}

// 获取碰撞实体
std::vector<std::shared_ptr<Entity>> Level::getCollidingEntities(const Rectangle& bounds) const {
    std::vector<std::shared_ptr<Entity>> result;
    
    for (const auto& pair : entities_) {
        if (pair.second && bounds.intersects(pair.second->getBounds())) {
            result.push_back(pair.second);
        }
    }
    
    return result;
}

// 获取碰撞实体（圆形）
std::vector<std::shared_ptr<Entity>> Level::getCollidingEntities(const Vector2D& position, float radius) const {
    Rectangle bounds(position.x - radius, position.y - radius, radius * 2, radius * 2);
    return getCollidingEntities(bounds);
}

// 查找路径
std::vector<Vector2D> Level::findPath(const Vector2D& start, const Vector2D& end) const {
    // 简单的直线路径查找
    std::vector<Vector2D> path;
    
    if (hasLineOfSight(start, end)) {
        path.push_back(start);
        path.push_back(end);
    }
    
    return path;
}

// 检查视线
bool Level::hasLineOfSight(const Vector2D& start, const Vector2D& end) const {
    Vector2D direction = end - start;
    float distance = direction.length();
    Vector2D normalized = direction.normalized();
    
    for (float d = 0; d <= distance; d += config_.tileSize / 2) {
        Vector2D point = start + normalized * d;
        if (!isWalkable(point)) {
            return false;
        }
    }
    
    return true;
}

// 发送事件
void Level::emitEvent(const LevelEvent& event) {
    eventQueue_.push_back(event);
    stats_.totalEvents++;
}

// 发送事件
void Level::emitEvent(LevelEventType type, const std::string& name, 
                     const LevelEventData& data) {
    LevelEvent event(type, name, data);
    emitEvent(event);
}

// 订阅事件
void Level::subscribeToEvent(LevelEventType type, EventCallback callback) {
    eventCallbacks_[type].push_back(callback);
}

// 取消订阅事件
void Level::unsubscribeFromEvent(LevelEventType type, EventCallback callback) {
    auto& callbacks = eventCallbacks_[type];
    callbacks.erase(
        std::remove_if(callbacks.begin(), callbacks.end(),
            [&callback](const EventCallback& cb) {
                return cb.target_type() == callback.target_type() && 
                       cb.target<void(*)(const LevelEvent&)>() == callback.target<void(*)(const LevelEvent&)>();
            }),
        callbacks.end()
    );
}

// 添加目标
bool Level::addObjective(const std::string& objectiveId, const std::string& description) {
    objectives_[objectiveId] = {description, false};
    LEVEL_LOG_DEBUG("添加目标: %s", objectiveId.c_str());
    return true;
}

// 完成目标
bool Level::completeObjective(const std::string& objectiveId) {
    auto it = objectives_.find(objectiveId);
    if (it == objectives_.end()) {
        return false;
    }
    
    it->second.completed = true;
    stats_.objectivesCompleted++;
    
    LEVEL_LOG_INFO("完成目标: %s", objectiveId.c_str());
    
    onObjectiveCompleted(objectiveId);
    
    return true;
}

// 失败目标
bool Level::failObjective(const std::string& objectiveId) {
    auto it = objectives_.find(objectiveId);
    if (it == objectives_.end()) {
        return false;
    }
    
    stats_.objectivesFailed++;
    
    LEVEL_LOG_WARN("目标失败: %s", objectiveId.c_str());
    
    onObjectiveFailed(objectiveId);
    
    return true;
}

// 检查目标是否完成
bool Level::isObjectiveCompleted(const std::string& objectiveId) const {
    auto it = objectives_.find(objectiveId);
    return it != objectives_.end() && it->second.completed;
}

// 获取活跃目标
std::vector<std::string> Level::getActiveObjectives() const {
    std::vector<std::string> result;
    
    for (const auto& pair : objectives_) {
        if (!pair.second.completed) {
            result.push_back(pair.first);
        }
    }
    
    return result;
}

// 添加触发器
bool Level::addTrigger(const std::string& triggerId, const Rectangle& area, 
                      std::function<void()> callback) {
    triggers_[triggerId] = Trigger(triggerId, area, callback);
    LEVEL_LOG_DEBUG("添加触发器: %s", triggerId.c_str());
    return true;
}

// 移除触发器
bool Level::removeTrigger(const std::string& triggerId) {
    auto it = triggers_.find(triggerId);
    if (it == triggers_.end()) {
        return false;
    }
    
    triggers_.erase(it);
    LEVEL_LOG_DEBUG("移除触发器: %s", triggerId.c_str());
    return true;
}

// 检查触发器
void Level::checkTriggers(const Vector2D& position) {
    for (auto& pair : triggers_) {
        Trigger& trigger = pair.second;
        if (!trigger.activated && trigger.area.contains(position)) {
            trigger.activated = true;
            trigger.callback();
            
            LEVEL_LOG_DEBUG("激活触发器: %s", trigger.id.c_str());
            
            emitEvent(LevelEventType::TRIGGER_ACTIVATED, "TriggerActivated",
                      {{"triggerId", trigger.id}});
            
            onTriggerActivated(trigger.id);
        }
    }
}

// 初始化地形
void Level::initializeTerrain() {
    LEVEL_LOG_DEBUG("初始化地形");
    
    // 根据关卡类型生成地形
    switch (config_.type) {
        case LevelType::FOREST:
            generateForestTerrain();
            break;
        case LevelType::CAVE:
            generateCaveTerrain();
            break;
        case LevelType::DUNGEON:
            generateDungeonTerrain();
            break;
        case LevelType::TOWN:
            generateCityTerrain();
            break;
        default:
            generateDefaultTerrain();
            break;
    }
}

// 初始化地形
bool Level::initTerrain() {
    LEVEL_LOG_DEBUG("初始化地形");
    
    // 根据关卡类型生成地形
    switch (config_.type) {
        case LevelType::FOREST:
            generateForestTerrain();
            break;
        case LevelType::CAVE:
            generateCaveTerrain();
            break;
        case LevelType::DUNGEON:
            generateDungeonTerrain();
            break;
        case LevelType::TOWN:
            generateCityTerrain();
            break;
        default:
            generateDefaultTerrain();
            break;
    }
    
    return true;
}

// 生成森林地形
void Level::generateForestTerrain() {
    LEVEL_LOG_DEBUG("生成森林地形");
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 100);
    
    for (int y = 0; y < config_.height; ++y) {
        for (int x = 0; x < config_.width; ++x) {
            int rand = dis(gen);
            Vector2D pos(x * config_.tileSize, y * config_.tileSize);
            
            if (rand < 70) {
                terrain_[y][x] = TerrainTile(TerrainType::GRASS, pos, config_.tileSize, config_.tileSize, true, false, true, 1.0f);
            } else if (rand < 85) {
                terrain_[y][x] = TerrainTile(TerrainType::ROCK, pos, config_.tileSize, config_.tileSize, false, false, true, 2.0f);
            } else {
                terrain_[y][x] = TerrainTile(TerrainType::WATER, pos, config_.tileSize, config_.tileSize, false, true, true, 1.5f);
            }
        }
    }
}

// 生成洞穴地形
void Level::generateCaveTerrain() {
    LEVEL_LOG_DEBUG("生成洞穴地形");
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 100);
    
    for (int y = 0; y < config_.height; ++y) {
        for (int x = 0; x < config_.width; ++x) {
            int rand = dis(gen);
            Vector2D pos(x * config_.tileSize, y * config_.tileSize);
            
            if (rand < 80) {
                terrain_[y][x] = TerrainTile(TerrainType::ROCK, pos, config_.tileSize, config_.tileSize, true, false, true, 1.0f);
            } else if (rand < 95) {
                terrain_[y][x] = TerrainTile(TerrainType::ROCK, pos, config_.tileSize, config_.tileSize, false, false, true, 999.0f);
            } else {
                terrain_[y][x] = TerrainTile(TerrainType::LAVA, pos, config_.tileSize, config_.tileSize, false, false, true, 999.0f);
            }
        }
    }
}

// 生成地下城地形
void Level::generateDungeonTerrain() {
    LEVEL_LOG_DEBUG("生成地下城地形");
    
    for (int y = 0; y < config_.height; ++y) {
        for (int x = 0; x < config_.width; ++x) {
            Vector2D pos(x * config_.tileSize, y * config_.tileSize);
            
            if (x == 0 || x == config_.width - 1 || y == 0 || y == config_.height - 1) {
                terrain_[y][x] = TerrainTile(TerrainType::WALL, pos, config_.tileSize, config_.tileSize, false, false, true, 999.0f);
            } else {
                terrain_[y][x] = TerrainTile(TerrainType::ROCK, pos, config_.tileSize, config_.tileSize, true, false, true, 1.0f);
            }
        }
    }
}

// 生成城市地形
void Level::generateCityTerrain() {
    LEVEL_LOG_DEBUG("生成城市地形");
    
    for (int y = 0; y < config_.height; ++y) {
        for (int x = 0; x < config_.width; ++x) {
            Vector2D pos(x * config_.tileSize, y * config_.tileSize);
            terrain_[y][x] = TerrainTile(TerrainType::ROCK, pos, config_.tileSize, config_.tileSize, true, false, true, 1.0f);
        }
    }
}

// 生成默认地形
void Level::generateDefaultTerrain() {
    LEVEL_LOG_DEBUG("生成默认地形");
    
    for (int y = 0; y < config_.height; ++y) {
        for (int x = 0; x < config_.width; ++x) {
            Vector2D pos(x * config_.tileSize, y * config_.tileSize);
            terrain_[y][x] = TerrainTile(TerrainType::GRASS, pos, config_.tileSize, config_.tileSize, true, false, true, 1.0f);
        }
    }
}

// 加载实体
void Level::loadEntities() {
    LEVEL_LOG_DEBUG("加载实体");
    
    // 这里应该从配置文件或数据库加载实体
    // 暂时为空实现
}

// 初始化碰撞系统
void Level::initCollisionSystem() {
    LEVEL_LOG_DEBUG("初始化碰撞系统");
    // 暂时为空实现
}

// 更新实体
void Level::updateEntities(double deltaTime) {
    for (auto& pair : entities_) {
        if (pair.second && pair.second->isActive()) {
            pair.second->update(deltaTime);
        }
    }
}

// 更新玩家
void Level::updatePlayers(double deltaTime) {
    for (auto& pair : players_) {
        if (pair.second) {
            pair.second->update(deltaTime);
        }
    }
}

// 更新触发器
void Level::updateTriggers() {
    // 检查所有玩家的触发器
    for (const auto& pair : players_) {
        if (pair.second) {
            checkTriggers(pair.second->getPosition());
        }
    }
}

// 处理事件
void Level::processEvents() {
    while (!eventQueue_.empty()) {
        const LevelEvent& event = eventQueue_.front();
        
        auto it = eventCallbacks_.find(event.type);
        if (it != eventCallbacks_.end()) {
            for (const auto& callback : it->second) {
                callback(event);
            }
        }
        
        eventQueue_.erase(eventQueue_.begin());
    }
}

// 更新统计
void Level::updateStats(double deltaTime) {
    // 统计更新逻辑
}

// 更新碰撞
void Level::updateCollisions() {
    // 碰撞检测逻辑
}

// 清理实体
void Level::cleanupEntities() {
    entities_.clear();
    players_.clear();
}

// 渲染地形
void Level::renderTerrain() {
    // 地形渲染逻辑
}

// 渲染UI
void Level::renderUI() {
    // UI渲染逻辑
}

// 检查实体是否在区域内
bool Level::isEntityInArea(const Entity& entity, const Rectangle& area) const {
    return area.intersects(entity.getBounds());
}

// 检查地形是否可步行
bool Level::isTerrainWalkable(TerrainType type) {
    switch (type) {
        case TerrainType::GRASS:
        case TerrainType::SAND:
        case TerrainType::BRIDGE:
        case TerrainType::ROCK:
            return true;
        case TerrainType::WATER:
        case TerrainType::LAVA:
        case TerrainType::WALL:
        case TerrainType::ICE:
            return false;
        default:
            return true;
    }
}

// 检查位置是否有效
bool Level::isValidPosition(const Vector2D& position) const {
    return position.x >= 0 && position.x < config_.width * config_.tileSize &&
           position.y >= 0 && position.y < config_.height * config_.tileSize;
}

// 获取瓦片索引
int Level::getTileIndex(const Vector2D& position) const {
    int x = static_cast<int>(position.x / config_.tileSize);
    int y = static_cast<int>(position.y / config_.tileSize);
    return y * config_.width + x;
}

// 事件回调方法
void Level::onLoaded() {
    LEVEL_LOG_INFO("关卡加载回调: %s", config_.name.c_str());
}

void Level::onUnloaded() {
    LEVEL_LOG_INFO("关卡卸载回调: %s", config_.name.c_str());
}

void Level::onEntitySpawned(std::shared_ptr<Entity> entity) {
    LEVEL_LOG_DEBUG("实体生成回调: %s", entity->getId().c_str());
}

void Level::onEntityDestroyed(const std::string& entityId) {
    LEVEL_LOG_DEBUG("实体销毁回调: %s", entityId.c_str());
}

void Level::onPlayerEntered(std::shared_ptr<Character> player) {
    LEVEL_LOG_INFO("玩家进入回调: %s", player->getName().c_str());
}

void Level::onPlayerLeft(const std::string& playerId) {
    LEVEL_LOG_INFO("玩家离开回调: %s", playerId.c_str());
}

void Level::onObjectiveCompleted(const std::string& objectiveId) {
    LEVEL_LOG_INFO("目标完成回调: %s", objectiveId.c_str());
}

void Level::onObjectiveFailed(const std::string& objectiveId) {
    LEVEL_LOG_WARN("目标失败回调: %s", objectiveId.c_str());
}

void Level::onTriggerActivated(const std::string& triggerId) {
    LEVEL_LOG_DEBUG("触发器激活回调: %s", triggerId.c_str());
}

} // namespace Fantasy
