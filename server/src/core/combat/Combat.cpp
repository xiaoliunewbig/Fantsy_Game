/**
 * @file Combat.cpp
 * @brief 战斗系统实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "include/core/combat/Combat.h"
#include "include/core/characters/Character.h"
#include <algorithm>
#include <random>
#include <cmath>
#include <numeric>

namespace Fantasy {

// 构造函数
Combat::Combat(const CombatConfig& config)
    : config_(config), state_(CombatState::PREPARING), isPlayerTurn_(true), currentTurn_(0),
      combatTime_(0.0), turnTimer_(0.0), actionTimer_(0.0), initialized_(false) {
    
    COMBAT_LOG_INFO("创建战斗系统");
    initializeCombat();
}

// 析构函数
Combat::~Combat() {
    COMBAT_LOG_INFO("销毁战斗系统");
    end();
}

// 开始战斗
bool Combat::start(const std::vector<std::shared_ptr<Character>>& participants) {
    COMBAT_LOG_INFO("开始战斗");
    
    if (state_ != CombatState::PREPARING) {
        COMBAT_LOG_WARN("战斗已经进行中");
        return false;
    }
    
    if (participants.size() < 2) {
        COMBAT_LOG_ERROR("需要至少2个参与者才能开始战斗");
        return false;
    }
    
    try {
        // 设置参与者
        participants_ = participants;
        
        // 初始化战斗状态
        state_ = CombatState::ACTIVE;
        
        // 初始化参与者战斗状态
        if (!initParticipants()) {
            COMBAT_LOG_ERROR("初始化参与者失败");
            return false;
        }
        
        // 确定行动顺序
        determineTurnOrder();
        
        // 设置战斗状态
        currentTurn_ = 0;
        combatTime_ = 0.0;
        
        // 初始化统计信息
        stats_ = CombatStats{};
        startTime_ = std::chrono::system_clock::now();
        
        // 发送战斗开始事件
        emitEvent(CombatEventType::COMBAT_STARTED, "CombatStarted");
        
        COMBAT_LOG_INFO("战斗成功开始，参与者数量: {}", participants_.size());
        return true;
        
    } catch (const std::exception& e) {
        COMBAT_LOG_ERROR("战斗开始时发生异常: {}", e.what());
        state_ = CombatState::FINISHED;
        return false;
    }
}

// 结束战斗
bool Combat::end() {
    COMBAT_LOG_INFO("结束战斗");
    
    if (state_ == CombatState::PREPARING) {
        return true;
    }
    
    state_ = CombatState::FINISHED;
    
    try {
        // 确定胜利者
        determineWinner();
        
        // 清理战斗状态
        cleanupCombat();
        
        // 发送战斗结束事件
        emitEvent(CombatEventType::COMBAT_ENDED, "CombatEnded");
        
        COMBAT_LOG_INFO("战斗结束");
        return true;
        
    } catch (const std::exception& e) {
        COMBAT_LOG_ERROR("战斗结束时发生异常: {}", e.what());
        return false;
    }
}

// 更新战斗
void Combat::update(double deltaTime) {
    if (state_ != CombatState::ACTIVE) {
        return;
    }
    
    // 更新计时器
    updateTimers(deltaTime);
    
    // 处理事件
    processEvents();
    
    // 更新统计
    updateStats(deltaTime);
    
    // 检查战斗是否结束
    if (checkCombatEnd()) {
        end();
        return;
    }
    
    // 处理行动队列
    executeQueuedActions();
}

// 暂停战斗
void Combat::pause() {
    if (state_ == CombatState::ACTIVE) {
        state_ = CombatState::PAUSED;
        COMBAT_LOG_INFO("战斗暂停");
    }
}

// 恢复战斗
void Combat::resume() {
    if (state_ == CombatState::PAUSED) {
        state_ = CombatState::ACTIVE;
        lastUpdateTime_ = std::chrono::system_clock::now();
        COMBAT_LOG_INFO("战斗恢复");
    }
}

// 执行行动
bool Combat::executeAction(const CombatAction& action) {
    if (state_ != CombatState::ACTIVE) {
        COMBAT_LOG_ERROR("无法执行行动：战斗未激活");
        return false;
    }
    
    COMBAT_LOG_DEBUG("执行行动: {} by {}", action.id, action.actor->getName());
    
    try {
        // 验证行动
        if (!validateAction(action, action.actor)) {
            COMBAT_LOG_ERROR("行动验证失败: {}", action.id);
            return false;
        }
        
        // 执行行动
        CombatResult result = performAction(action, action.actor);
        
        // 处理结果
        processActionResult(result);
        
        // 检查战斗是否结束
        if (checkCombatEnd()) {
            end();
            return true;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        COMBAT_LOG_ERROR("执行行动时发生异常: {}", e.what());
        return false;
    }
}

// 队列行动
bool Combat::queueAction(const CombatAction& action) {
    if (state_ != CombatState::ACTIVE) {
        return false;
    }
    
    actionQueue_.push(action);
    COMBAT_LOG_DEBUG("行动已加入队列: {}", action.id);
    return true;
}

// 清空行动队列
void Combat::clearActionQueue() {
    while (!actionQueue_.empty()) {
        actionQueue_.pop();
    }
    COMBAT_LOG_DEBUG("行动队列已清空");
}

// 处理回合
void Combat::processTurn() {
    if (state_ != CombatState::ACTIVE) {
        return;
    }
    
    COMBAT_LOG_DEBUG("处理回合: {}", currentTurn_);
    
    // 处理玩家回合
    if (isPlayerTurn_) {
        // 等待玩家输入
        return;
    }
    
    // 处理敌人回合
    processEnemyTurns();
}

// 处理行动
void Combat::processActions(double deltaTime) {
    if (state_ != CombatState::ACTIVE) {
        return;
    }
    
    actionTimer_ += deltaTime;
    
    // 处理行动队列
    while (!actionQueue_.empty() && actionTimer_ >= 0.1) { // 每0.1秒处理一个行动
        CombatAction action = actionQueue_.front();
        actionQueue_.pop();
        
        executeAction(action);
        actionTimer_ = 0.0;
    }
}

// 玩家攻击
bool Combat::playerAttack(int targetIndex) {
    if (!isPlayerTurn_ || state_ != CombatState::ACTIVE) {
        return false;
    }
    
    if (targetIndex < 0 || targetIndex >= enemies_.size()) {
        return false;
    }
    
    auto target = enemies_[targetIndex];
    if (!target || !target->isAlive()) {
        return false;
    }
    
    CombatAction action(ActionType::ATTACK, player_, target);
    action.id = "player_attack";
    
    return executeAction(action);
}

// 玩家使用技能
bool Combat::playerUseSkill(const std::string& skillId, int targetIndex) {
    if (!isPlayerTurn_ || state_ != CombatState::ACTIVE) {
        return false;
    }
    
    CombatAction action(ActionType::SKILL, player_);
    action.id = "player_skill";
    action.skillId = skillId;
    
    if (targetIndex >= 0 && targetIndex < enemies_.size()) {
        action.target = enemies_[targetIndex];
    }
    
    return executeAction(action);
}

// 玩家使用物品
bool Combat::playerUseItem(const std::string& itemId, int targetIndex) {
    if (!isPlayerTurn_ || state_ != CombatState::ACTIVE) {
        return false;
    }
    
    CombatAction action(ActionType::ITEM, player_);
    action.id = "player_item";
    action.itemId = itemId;
    
    if (targetIndex >= 0 && targetIndex < participants_.size()) {
        action.target = participants_[targetIndex];
    }
    
    return executeAction(action);
}

// 玩家防御
bool Combat::playerDefend() {
    if (!isPlayerTurn_ || state_ != CombatState::ACTIVE) {
        return false;
    }
    
    CombatAction action(ActionType::DEFEND, player_);
    action.id = "player_defend";
    
    return executeAction(action);
}

// 玩家逃跑
bool Combat::playerFlee() {
    if (!isPlayerTurn_ || state_ != CombatState::ACTIVE) {
        return false;
    }
    
    CombatAction action(ActionType::FLEE, player_);
    action.id = "player_flee";
    
    return executeAction(action);
}

// 处理敌人回合
void Combat::processEnemyTurns() {
    for (auto& enemy : enemies_) {
        if (enemy && enemy->isAlive()) {
            CombatAction action = generateAIAction(enemy);
            executeAction(action);
        }
    }
}

// 生成AI行动
CombatAction Combat::generateAIAction(std::shared_ptr<Character> enemy) {
    // 简单的AI逻辑：随机选择行动
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 3);
    
    int actionType = dis(gen);
    
    switch (actionType) {
        case 0: {
            // 攻击
            auto target = selectTarget(enemy, {player_});
            CombatAction action(ActionType::ATTACK, enemy, target);
            action.id = "enemy_attack";
            return action;
        }
        case 1: {
            // 使用技能
            CombatAction action(ActionType::SKILL, enemy);
            action.id = "enemy_skill";
            action.skillId = "basic_attack"; // 默认技能
            auto target = selectTarget(enemy, {player_});
            action.target = target;
            return action;
        }
        case 2: {
            // 防御
            CombatAction action(ActionType::DEFEND, enemy);
            action.id = "enemy_defend";
            return action;
        }
        default: {
            // 等待
            CombatAction action(ActionType::WAIT, enemy);
            action.id = "enemy_wait";
            return action;
        }
    }
}

// 选择目标
std::shared_ptr<Character> Combat::selectTarget(std::shared_ptr<Character> attacker, 
                        const std::vector<std::shared_ptr<Character>>& targets) {
    if (targets.empty()) {
        return nullptr;
    }
    
    // 简单的目标选择：选择第一个活着的目标
    for (const auto& target : targets) {
        if (target && target->isAlive()) {
            return target;
        }
    }
    
    return targets[0]; // 如果没有活着的目标，返回第一个
}

// 计算伤害
CombatResult Combat::calculateDamage(std::shared_ptr<Character> attacker, 
                                    std::shared_ptr<Character> target,
                                    const std::string& skillId) {
    CombatResult result;
    
    // 计算命中率
    float hitChance = calculateHitChance(attacker, target);
    float dodgeChance = calculateDodgeChance(target);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    if (dis(gen) > hitChance || dis(gen) < dodgeChance) {
        result.hit = false;
        return result;
    }
    
    result.hit = true;
    
    // 计算基础伤害
    result.damage = calculateBaseDamage(attacker, target);
    result.damageType = DamageType::PHYSICAL;
    
    // 计算暴击
    float critChance = calculateCriticalChance(attacker);
    if (dis(gen) < critChance) {
        result.critical = true;
        result.damage = static_cast<int>(result.damage * calculateCriticalMultiplier(attacker));
    }
    
    return result;
}

// 计算基础伤害
int Combat::calculateBaseDamage(std::shared_ptr<Character> attacker, 
                               std::shared_ptr<Character> target) {
    int attack = attacker->getStats().attack;
    int defense = target->getStats().defense;
    
    int damage = std::max(1, attack - defense);
    
    // 添加随机性
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(80, 120); // 80%-120%的伤害
    
    return static_cast<int>(damage * dis(gen) / 100.0f);
}

// 计算暴击率
float Combat::calculateCriticalChance(std::shared_ptr<Character> attacker) {
    return attacker->getStats().criticalRate / 100.0f;
}

// 计算暴击倍数
float Combat::calculateCriticalMultiplier(std::shared_ptr<Character> attacker) {
    return attacker->getStats().criticalDamage / 100.0f;
}

// 计算命中率
float Combat::calculateHitChance(std::shared_ptr<Character> attacker, 
                                std::shared_ptr<Character> target) {
    // 基础命中率90%
    float baseHitChance = 0.9f;
    
    // 根据等级差异调整
    int levelDiff = attacker->getStats().level - target->getStats().level;
    float levelBonus = levelDiff * 0.02f; // 每级差异2%
    
    return std::min(0.95f, baseHitChance + levelBonus);
}

// 计算闪避率
float Combat::calculateDodgeChance(std::shared_ptr<Character> target) {
    return target->getStats().dodgeRate / 100.0f;
}

// 检查战斗结束
bool Combat::checkCombatEnd() {
    // 检查玩家是否死亡
    if (!player_ || !player_->isAlive()) {
        return true;
    }
    
    // 检查所有敌人是否死亡
    bool allEnemiesDead = true;
    for (const auto& enemy : enemies_) {
        if (enemy && enemy->isAlive()) {
            allEnemiesDead = false;
            break;
        }
    }
    
    return allEnemiesDead;
}

// 检查玩家胜利
bool Combat::isPlayerVictory() const {
    if (!player_ || !player_->isAlive()) {
        return false;
    }
    
    for (const auto& enemy : enemies_) {
        if (enemy && enemy->isAlive()) {
            return false;
        }
    }
    
    return true;
}

// 检查玩家失败
bool Combat::isPlayerDefeat() const {
    return !player_ || !player_->isAlive();
}

// 检查是否可以逃跑
bool Combat::canFlee() const {
    return config_.allowFlee;
}

// 检查是否激活
bool Combat::isActive() const {
    return state_ == CombatState::ACTIVE;
}

// 检查是否暂停
bool Combat::isPaused() const {
    return state_ == CombatState::PAUSED;
}

// 发送事件
void Combat::emitEvent(const CombatEvent& event) {
    eventQueue_.push_back(event);
    stats_.totalEvents++;
}

// 发送事件
void Combat::emitEvent(CombatEventType type, const std::string& name, 
                      const CombatEventData& data) {
    CombatEvent event(type, name, data);
    emitEvent(event);
}

// 订阅事件
void Combat::subscribeToEvent(CombatEventType type, EventCallback callback) {
    eventCallbacks_[type].push_back(callback);
}

// 取消订阅事件
void Combat::unsubscribeFromEvent(CombatEventType type, EventCallback callback) {
    auto& callbacks = eventCallbacks_[type];
    callbacks.erase(
        std::remove_if(callbacks.begin(), callbacks.end(),
            [&callback](const EventCallback& cb) {
                return cb.target_type() == callback.target_type() && 
                       cb.target<void(*)(const CombatEvent&)>() == callback.target<void(*)(const CombatEvent&)>();
            }),
        callbacks.end()
    );
}

// 获取参与者
std::vector<std::shared_ptr<Character>> Combat::getParticipants() const {
    return participants_;
}

// 获取存活的参与者
std::vector<std::shared_ptr<Character>> Combat::getAliveParticipants() const {
    std::vector<std::shared_ptr<Character>> alive;
    for (const auto& participant : participants_) {
        if (participant && participant->isAlive()) {
            alive.push_back(participant);
        }
    }
    return alive;
}

// 获取当前行动者
std::shared_ptr<Character> Combat::getCurrentActor() const {
    if (isPlayerTurn_) {
        return player_;
    }
    
    // 返回第一个存活的敌人
    for (const auto& enemy : enemies_) {
        if (enemy && enemy->isAlive()) {
            return enemy;
        }
    }
    
    return nullptr;
}

// 获取胜利者
std::shared_ptr<Character> Combat::getWinner() const {
    if (isPlayerVictory()) {
        return player_;
    }
    
    // 返回第一个存活的敌人
    for (const auto& enemy : enemies_) {
        if (enemy && enemy->isAlive()) {
            return enemy;
        }
    }
    
    return nullptr;
}

// 验证行动
bool Combat::validateAction(const CombatAction& action, std::shared_ptr<Character> actor) {
    if (!actor || !actor->isAlive()) {
        return false;
    }
    
    switch (action.type) {
        case ActionType::ATTACK:
            return validateAttackAction(action, actor);
        case ActionType::SKILL:
            return validateSkillAction(action, actor);
        case ActionType::ITEM:
            return validateItemAction(action, actor);
        case ActionType::DEFEND:
            return validateDefendAction(action, actor);
        case ActionType::FLEE:
            return validateFleeAction(action, actor);
        case ActionType::WAIT:
            return true;
        default:
            return false;
    }
}

// 验证攻击行动
bool Combat::validateAttackAction(const CombatAction& action, std::shared_ptr<Character> actor) {
    if (!action.target) {
        return false;
    }
    
    // 检查目标是否有效
    for (const auto& targetId : {action.target->getName()}) {
        bool found = false;
        for (const auto& participant : participants_) {
            if (participant && participant->getName() == targetId) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    
    return true;
}

// 验证技能行动
bool Combat::validateSkillAction(const CombatAction& action, std::shared_ptr<Character> actor) {
    if (!config_.allowSkills) {
        return false;
    }
    
    if (action.skillId.empty()) {
        return false;
    }
    
    // 这里应该检查角色是否拥有该技能
    return true;
}

// 验证物品行动
bool Combat::validateItemAction(const CombatAction& action, std::shared_ptr<Character> actor) {
    if (!config_.allowItems) {
        return false;
    }
    
    if (action.itemId.empty()) {
        return false;
    }
    
    // 这里应该检查角色是否拥有该物品
    return true;
}

// 验证防御行动
bool Combat::validateDefendAction(const CombatAction& action, std::shared_ptr<Character> actor) {
    return true;
}

// 验证逃跑行动
bool Combat::validateFleeAction(const CombatAction& action, std::shared_ptr<Character> actor) {
    if (!config_.allowFlee) {
        return false;
    }
    
    return true;
}

// 执行行动
CombatResult Combat::performAction(const CombatAction& action, std::shared_ptr<Character> actor) {
    switch (action.type) {
        case ActionType::ATTACK:
            return performAttackAction(action, actor);
        case ActionType::SKILL:
            return performSkillAction(action, actor);
        case ActionType::ITEM:
            return performItemAction(action, actor);
        case ActionType::DEFEND:
            return performDefendAction(action, actor);
        case ActionType::FLEE:
            return performFleeAction(action, actor);
        case ActionType::WAIT:
            return CombatResult{};
        default:
            COMBAT_LOG_ERROR("未知的行动类型");
            return CombatResult{};
    }
}

// 执行攻击行动
CombatResult Combat::performAttackAction(const CombatAction& action, std::shared_ptr<Character> actor) {
    CombatResult result;
    
    if (!action.target) {
        return result;
    }
    
    // 计算伤害
    result = calculateDamage(actor, action.target);
    
    if (result.hit) {
        // 应用伤害
        applyDamage(action.target, result.damage);
        
        COMBAT_LOG_INFO("{} 对 {} 造成 {} 点伤害", 
                       actor->getName(), action.target->getName(), result.damage);
    } else {
        COMBAT_LOG_INFO("{} 的攻击被 {} 闪避了", 
                       actor->getName(), action.target->getName());
    }
    
    return result;
}

// 执行技能行动
CombatResult Combat::performSkillAction(const CombatAction& action, std::shared_ptr<Character> actor) {
    CombatResult result;
    
    COMBAT_LOG_INFO("{} 使用技能: {}", actor->getName(), action.skillId);
    
    // 这里应该实现技能效果
    stats_.skillsUsed++;
    
    return result;
}

// 执行物品行动
CombatResult Combat::performItemAction(const CombatAction& action, std::shared_ptr<Character> actor) {
    CombatResult result;
    
    COMBAT_LOG_INFO("{} 使用物品: {}", actor->getName(), action.itemId);
    
    // 这里应该实现物品效果
    stats_.itemsUsed++;
    
    return result;
}

// 执行防御行动
CombatResult Combat::performDefendAction(const CombatAction& action, std::shared_ptr<Character> actor) {
    CombatResult result;
    
    COMBAT_LOG_INFO("{} 进入防御状态", actor->getName());
    
    // 这里应该实现防御效果
    
    return result;
}

// 执行逃跑行动
CombatResult Combat::performFleeAction(const CombatAction& action, std::shared_ptr<Character> actor) {
    CombatResult result;
    
    double fleeChance = calculateFleeChance(actor);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0.0, 1.0);
    
    if (dis(gen) < fleeChance) {
        COMBAT_LOG_INFO("{} 成功逃跑", actor->getName());
        // 这里应该实现逃跑逻辑
    } else {
        COMBAT_LOG_INFO("{} 逃跑失败", actor->getName());
    }
    
    return result;
}

// 计算逃跑概率
double Combat::calculateFleeChance(std::shared_ptr<Character> actor) {
    // 基础逃跑概率30%
    double baseChance = 0.3;
    
    // 根据速度调整
    double speedBonus = actor->getStats().speed * 0.01;
    
    return std::min(0.8, baseChance + speedBonus);
}

// 应用伤害
void Combat::applyDamage(std::shared_ptr<Character> target, int damage) {
    if (!target || !target->isAlive()) {
        return;
    }
    
    // 这里应该调用角色的受伤方法
    // target->takeDamage(damage);
    
    stats_.totalDamageDealt += damage;
    
    if (!target->isAlive()) {
        stats_.totalKills++;
        COMBAT_LOG_INFO("{} 被击败了", target->getName());
    }
}

// 处理行动结果
void Combat::processActionResult(const CombatResult& result) {
    // 发送行动执行事件
    CombatEventData data = {
        {"hit", result.hit ? "true" : "false"},
        {"critical", result.critical ? "true" : "false"},
        {"damage", std::to_string(result.damage)}
    };
    
    emitEvent(CombatEventType::ACTION_EXECUTED, "ActionExecuted", data);
    
    if (result.hit) {
        stats_.totalActions++;
        if (result.critical) {
            stats_.criticalHits++;
        }
    }
}

// 下一回合
void Combat::nextTurn() {
    currentTurn_++;
    isPlayerTurn_ = !isPlayerTurn_;
    
    COMBAT_LOG_DEBUG("进入回合: {}, 玩家回合: {}", currentTurn_, isPlayerTurn_);
    
    // 发送回合开始事件
    emitEvent(CombatEventType::TURN_STARTED, "TurnStarted");
}

// 结束回合
void Combat::endTurn() {
    // 发送回合结束事件
    emitEvent(CombatEventType::TURN_ENDED, "TurnEnded");
    
    // 切换到下一回合
    nextTurn();
}

// 确定胜利者
void Combat::determineWinner() {
    if (isPlayerVictory()) {
        state_ = CombatState::VICTORY;
        COMBAT_LOG_INFO("玩家胜利");
        emitEvent(CombatEventType::VICTORY, "Victory");
    } else if (isPlayerDefeat()) {
        state_ = CombatState::DEFEAT;
        COMBAT_LOG_INFO("玩家失败");
        emitEvent(CombatEventType::DEFEAT, "Defeat");
    }
    
    // 计算战斗持续时间
    auto endTime = std::chrono::system_clock::now();
    stats_.combatDuration = std::chrono::duration<double>(endTime - startTime_).count();
}

// 清理战斗
void Combat::cleanupCombat() {
    // 清空行动队列
    clearActionQueue();
    
    // 清理参与者状态
    for (auto& participant : participants_) {
        if (participant) {
            // 这里应该清理角色的战斗状态
        }
    }
    
    COMBAT_LOG_DEBUG("战斗清理完成");
}

// 查找参与者
std::shared_ptr<Character> Combat::findParticipant(const std::string& id) const {
    for (const auto& participant : participants_) {
        if (participant && participant->getName() == id) {
            return participant;
        }
    }
    return nullptr;
}

// 初始化战斗
void Combat::initializeCombat() {
    COMBAT_LOG_DEBUG("初始化战斗系统");
    initialized_ = true;
}

// 初始化参与者
bool Combat::initParticipants() {
    if (participants_.empty()) {
        return false;
    }
    
    // 分离玩家和敌人
    player_ = participants_[0]; // 假设第一个是玩家
    enemies_.clear();
    
    for (size_t i = 1; i < participants_.size(); ++i) {
        enemies_.push_back(participants_[i]);
    }
    
    COMBAT_LOG_DEBUG("初始化参与者完成，玩家: {}, 敌人: {}", 
                    player_->getName(), enemies_.size());
    
    return true;
}

// 确定行动顺序
void Combat::determineTurnOrder() {
    COMBAT_LOG_DEBUG("确定行动顺序");
    
    // 简单的行动顺序：玩家先行动
    isPlayerTurn_ = true;
}

// 更新计时器
void Combat::updateTimers(double deltaTime) {
    combatTime_ += deltaTime;
    turnTimer_ += deltaTime;
    actionTimer_ += deltaTime;
}

// 处理事件
void Combat::processEvents() {
    while (!eventQueue_.empty()) {
        const CombatEvent& event = eventQueue_.front();
        
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
void Combat::updateStats(double deltaTime) {
    stats_.turnsTaken = currentTurn_;
}

// 执行队列中的行动
void Combat::executeQueuedActions() {
    // 在processActions中已经处理
}

// 解析行动
void Combat::resolveAction(const CombatAction& action) {
    // 在performAction中已经处理
}

// 应用伤害
void Combat::applyDamage(std::shared_ptr<Character> target, const CombatResult& result) {
    if (result.hit) {
        applyDamage(target, result.damage);
    }
}

// 检查角色状态
void Combat::checkCharacterStatus(std::shared_ptr<Character> character) {
    if (!character->isAlive()) {
        onCharacterDefeated(character);
    }
}

// 移除被击败的角色
void Combat::removeDefeatedCharacters() {
    // 在checkCombatEnd中已经处理
}

// 切换回合
void Combat::switchTurn() {
    nextTurn();
}

// 生成敌人行动
void Combat::generateEnemyActions() {
    processEnemyTurns();
}

// 获取存活的敌人
std::vector<std::shared_ptr<Character>> Combat::getAliveEnemies() const {
    std::vector<std::shared_ptr<Character>> alive;
    for (const auto& enemy : enemies_) {
        if (enemy && enemy->isAlive()) {
            alive.push_back(enemy);
        }
    }
    return alive;
}

// 检查目标是否有效
bool Combat::isValidTarget(std::shared_ptr<Character> target) const {
    if (!target) {
        return false;
    }
    
    for (const auto& participant : participants_) {
        if (participant == target) {
            return true;
        }
    }
    
    return false;
}

// 获取参与者索引
int Combat::getParticipantIndex(std::shared_ptr<Character> participant) const {
    for (size_t i = 0; i < participants_.size(); ++i) {
        if (participants_[i] == participant) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// 事件回调方法
void Combat::onCombatStarted() {
    COMBAT_LOG_INFO("战斗开始回调");
}

void Combat::onCombatEnded() {
    COMBAT_LOG_INFO("战斗结束回调");
}

void Combat::onTurnStarted() {
    COMBAT_LOG_DEBUG("回合开始回调");
}

void Combat::onTurnEnded() {
    COMBAT_LOG_DEBUG("回合结束回调");
}

void Combat::onActionExecuted(const CombatAction& action, const CombatResult& result) {
    COMBAT_LOG_DEBUG("行动执行回调: {}", action.id);
}

void Combat::onDamageDealt(std::shared_ptr<Character> target, int damage, DamageType type) {
    COMBAT_LOG_DEBUG("造成伤害回调: {} 对 {} 造成 {} 点伤害", 
                    target->getName(), damage);
}

void Combat::onDamageTaken(std::shared_ptr<Character> target, int damage, DamageType type) {
    COMBAT_LOG_DEBUG("受到伤害回调: {} 受到 {} 点伤害", 
                    target->getName(), damage);
}

void Combat::onCharacterDefeated(std::shared_ptr<Character> character) {
    COMBAT_LOG_INFO("角色被击败回调: {}", character->getName());
}

void Combat::onVictory() {
    COMBAT_LOG_INFO("胜利回调");
}

void Combat::onDefeat() {
    COMBAT_LOG_INFO("失败回调");
}

} // namespace Fantasy

