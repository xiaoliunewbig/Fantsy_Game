/**
 * @file Quest.cpp
 * @brief 任务系统实现
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#include "core/quests/Quest.h"
#include "core/characters/Character.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <chrono>

namespace Fantasy {

// 构造函数
Quest::Quest(const std::string& id, const std::string& name, QuestType type, const QuestConfig& config)
    : id_(id), name_(name), description_(config.description), type_(type), level_(config.level),
      state_(QuestState::NOT_STARTED), progress_(0.0), completionCount_(0), repeatable_(config.repeatable),
      maxCompletions_(config.maxCompletions), timeLimit_(config.timeLimit), rewardsClaimed_(false),
      config_(config), stats_(), character_(nullptr) {
    
    QUEST_LOG_INFO("创建任务: %s (%s)", name_.c_str(), id_.c_str());
    
    // 初始化目标
    objectives_ = config.objectives;
    
    // 初始化奖励
    rewards_ = config.rewards;
    
    // 初始化需求
    requirements_ = config.requirements;
    
    // 初始化统计
    stats_.creationTime = std::chrono::system_clock::now();
    
    initializeQuest();
}

// 析构函数
Quest::~Quest() {
    QUEST_LOG_INFO("销毁任务: %s", name_.c_str());
}

// 开始任务
bool Quest::start(std::shared_ptr<Character> character) {
    if (state_ != QuestState::NOT_STARTED) {
        QUEST_LOG_WARN("任务 %s 无法开始，当前状态: %d", name_.c_str(), static_cast<int>(state_));
        return false;
    }
    
    if (!canStart(character)) {
        QUEST_LOG_WARN("任务 %s 不满足开始条件", name_.c_str());
        return false;
    }
    
    character_ = character;
    state_ = QuestState::ACTIVE;
    startTime_ = std::chrono::system_clock::now();
    stats_.lastStartTime = startTime_;
    stats_.totalStarts++;
    
    QUEST_LOG_INFO("任务 %s 开始", name_.c_str());
    
    // 初始化目标
    initObjectives();
    
    // 发送事件
    emitEvent(QuestEventType::QUEST_STARTED, "QuestStarted");
    onStarted();
    
    return true;
}

// 完成任务
bool Quest::complete() {
    if (state_ != QuestState::ACTIVE) {
        QUEST_LOG_WARN("任务 %s 无法完成，当前状态: %d", name_.c_str(), static_cast<int>(state_));
        return false;
    }
    
    if (!areAllObjectivesCompleted()) {
        QUEST_LOG_WARN("任务 %s 目标未全部完成", name_.c_str());
        return false;
    }
    
    state_ = QuestState::COMPLETED;
    completionTime_ = std::chrono::system_clock::now();
    stats_.lastCompletionTime = completionTime_;
    stats_.totalCompletions++;
    completionCount_++;
    progress_ = 100.0;
    
    QUEST_LOG_INFO("任务 %s 完成", name_.c_str());
    
    // 发放奖励
    if (giveRewards()) {
        rewardsClaimed_ = true;
        emitEvent(QuestEventType::REWARD_GRANTED, "RewardGranted");
        onRewardsGranted();
    }
    
    // 发送事件
    emitEvent(QuestEventType::QUEST_COMPLETED, "QuestCompleted");
    onCompleted();
    
    return true;
}

// 任务失败
bool Quest::fail() {
    if (state_ != QuestState::ACTIVE) {
        QUEST_LOG_WARN("任务 %s 无法失败，当前状态: %d", name_.c_str(), static_cast<int>(state_));
        return false;
    }
    
    state_ = QuestState::FAILED;
    stats_.lastFailureTime = std::chrono::system_clock::now();
    stats_.totalFailures++;
    
    QUEST_LOG_INFO("任务 %s 失败", name_.c_str());
    
    // 发送事件
    emitEvent(QuestEventType::QUEST_FAILED, "QuestFailed");
    onFailed();
    
    return true;
}

// 放弃任务
bool Quest::abandon() {
    if (state_ != QuestState::ACTIVE) {
        QUEST_LOG_WARN("任务 %s 无法放弃，当前状态: %d", name_.c_str(), static_cast<int>(state_));
        return false;
    }
    
    state_ = QuestState::ABANDONED;
    stats_.lastAbandonTime = std::chrono::system_clock::now();
    stats_.totalAbandons++;
    
    QUEST_LOG_INFO("任务 %s 被放弃", name_.c_str());
    
    // 发送事件
    emitEvent(QuestEventType::QUEST_ABANDONED, "QuestAbandoned");
    onAbandoned();
    
    return true;
}

// 更新任务
void Quest::update(double deltaTime) {
    if (state_ != QuestState::ACTIVE) {
        return;
    }
    
    // 更新计时器
    updateTimers(deltaTime);
    
    // 检查时间限制
    checkTimeLimit();
    
    // 处理事件
    processEvents();
    
    // 更新统计
    stats_.timeSpent += deltaTime;
    stats_.totalEvents++;
    
    // 处理事件
    processEvents();
}

// 更新目标进度
bool Quest::updateObjective(const std::string& objectiveId, int progress) {
    if (state_ != QuestState::ACTIVE) {
        return false;
    }
    
    int index = getObjectiveIndex(objectiveId);
    if (index == -1) {
        QUEST_LOG_WARN("目标 %s 不存在", objectiveId.c_str());
        return false;
    }
    
    QuestObjective& objective = objectives_[index];
    objective.current = std::min(objective.current + progress, objective.required);
    
    if (objective.current >= objective.required && !objective.completed) {
        objective.completed = true;
        QUEST_LOG_INFO("目标 %s 完成", objectiveId.c_str());
        emitEvent(QuestEventType::OBJECTIVE_COMPLETED, "ObjectiveCompleted");
        onObjectiveCompleted(objectiveId);
    } else {
        QUEST_LOG_INFO("目标 %s 进度更新: %d/%d", objectiveId.c_str(), objective.current, objective.required);
        emitEvent(QuestEventType::OBJECTIVE_UPDATED, "ObjectiveUpdated");
        onObjectiveUpdated(objectiveId);
    }
    
    // 重新计算总体进度
    calculateProgress();
    
    return true;
}

// 完成目标
bool Quest::completeObjective(const std::string& objectiveId) {
    return updateObjective(objectiveId, 999999); // 设置一个很大的值来确保完成
}

// 检查目标是否完成
bool Quest::isObjectiveCompleted(const std::string& objectiveId) const {
    int index = getObjectiveIndex(objectiveId);
    if (index == -1) {
        return false;
    }
    return objectives_[index].completed;
}

// 获取目标
QuestObjective Quest::getObjective(const std::string& objectiveId) const {
    int index = getObjectiveIndex(objectiveId);
    if (index == -1) {
        return QuestObjective();
    }
    return objectives_[index];
}

// 获取所有目标
std::vector<QuestObjective> Quest::getObjectives() const {
    return objectives_;
}

// 检查所有目标是否完成
bool Quest::areAllObjectivesCompleted() const {
    for (const auto& objective : objectives_) {
        if (!objective.completed) {
            return false;
        }
    }
    return true;
}

// 检查是否可以开始
bool Quest::canStart(std::shared_ptr<Character> character) const {
    if (!character) {
        return false;
    }
    
    // 检查等级需求
    if (!checkLevelRequirement(character->getStats().level)) {
        return false;
    }
    
    // 检查其他需求
    return meetsRequirements();
}

// 检查是否满足需求
bool Quest::meetsRequirements() const {
    for (const auto& req : requirements_) {
        if (!checkRequirement(req)) {
            return false;
        }
    }
    return true;
}

// 检查等级需求
bool Quest::checkLevelRequirement(int level) const {
    return level >= level_;
}

// 检查任务需求
bool Quest::checkQuestRequirement(const std::string& questId, QuestState state) const {
    // 这里需要实现具体的任务状态检查逻辑
    return true; // 临时返回true
}

// 检查物品需求
bool Quest::checkItemRequirement(const std::string& itemId, int quantity) const {
    // 这里需要实现具体的物品检查逻辑
    return true; // 临时返回true
}

// 发放奖励
bool Quest::grantRewards() {
    if (rewardsClaimed_) {
        QUEST_LOG_WARN("奖励已经发放过了");
        return false;
    }
    
    bool success = true;
    for (const auto& reward : rewards_) {
        if (!giveReward(reward)) {
            success = false;
        }
    }
    
    if (success) {
        rewardsClaimed_ = true;
        QUEST_LOG_INFO("任务 %s 奖励发放成功", name_.c_str());
    } else {
        QUEST_LOG_ERROR("任务 %s 奖励发放失败", name_.c_str());
    }
    
    return success;
}

// 检查是否可以完成
bool Quest::canComplete() const {
    return state_ == QuestState::ACTIVE && areAllObjectivesCompleted();
}

// 检查是否可以放弃
bool Quest::canAbandon() const {
    return state_ == QuestState::ACTIVE;
}

// 检查是否可以重复
bool Quest::canRepeat() const {
    return repeatable_ && completionCount_ < maxCompletions_;
}

// 获取时间花费
double Quest::getTimeSpent() const {
    if (state_ == QuestState::NOT_STARTED) {
        return 0.0;
    }
    
    auto now = std::chrono::system_clock::now();
    auto endTime = (state_ == QuestState::COMPLETED) ? completionTime_ : now;
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime_);
    return duration.count() / 1000.0;
}

// 获取剩余时间
double Quest::getTimeRemaining() const {
    if (timeLimit_ <= 0) {
        return -1.0; // 无时间限制
    }
    
    double timeSpent = getTimeSpent();
    return std::max(0.0, timeLimit_ - timeSpent);
}

// 发送事件
void Quest::emitEvent(const QuestEvent& event) {
    eventQueue_.push_back(event);
    stats_.totalEvents++;
}

// 发送事件
void Quest::emitEvent(QuestEventType type, const std::string& name, const QuestEventData& data) {
    QuestEvent event(type, name, data);
    emitEvent(event);
}

// 订阅事件
void Quest::subscribeToEvent(QuestEventType type, QuestEventCallback callback) {
    if (eventCallbacks_.find(type) == eventCallbacks_.end()) {
        eventCallbacks_[type] = std::vector<QuestEventCallback>();
    }
    eventCallbacks_[type].push_back(callback);
}

// 取消订阅事件
void Quest::unsubscribeFromEvent(QuestEventType type, QuestEventCallback callback) {
    auto it = eventCallbacks_.find(type);
    if (it != eventCallbacks_.end()) {
        auto& callbacks = it->second;
        callbacks.erase(
            std::remove_if(callbacks.begin(), callbacks.end(),
                [&callback](const QuestEventCallback& cb) {
                    return &cb == &callback;
                }),
            callbacks.end()
        );
    }
}

// 事件回调方法
void Quest::onStarted() {
    QUEST_LOG_INFO("任务 %s 开始回调", name_.c_str());
}

void Quest::onCompleted() {
    QUEST_LOG_INFO("任务 %s 完成回调", name_.c_str());
}

void Quest::onFailed() {
    QUEST_LOG_INFO("任务 %s 失败回调", name_.c_str());
}

void Quest::onAbandoned() {
    QUEST_LOG_INFO("任务 %s 放弃回调", name_.c_str());
}

void Quest::onExpired() {
    QUEST_LOG_INFO("任务 %s 过期回调", name_.c_str());
}

void Quest::onObjectiveUpdated(const std::string& objectiveId) {
    QUEST_LOG_INFO("任务 %s 目标 %s 更新回调", name_.c_str(), objectiveId.c_str());
}

void Quest::onObjectiveCompleted(const std::string& objectiveId) {
    QUEST_LOG_INFO("任务 %s 目标 %s 完成回调", name_.c_str(), objectiveId.c_str());
}

void Quest::onRewardsGranted() {
    QUEST_LOG_INFO("任务 %s 奖励发放回调", name_.c_str());
}

void Quest::onChainStarted() {
    QUEST_LOG_INFO("任务 %s 链开始回调", name_.c_str());
}

void Quest::onChainCompleted() {
    QUEST_LOG_INFO("任务 %s 链完成回调", name_.c_str());
}

// 初始化任务
void Quest::initializeQuest() {
    QUEST_LOG_INFO("初始化任务: %s", name_.c_str());
    
    // 初始化目标
    initObjectives();
    
    // 计算奖励
    calculateRewards();
    
    // 计算进度
    calculateProgress();
}

void Quest::processEvents() {
    while (!eventQueue_.empty()) {
        QuestEvent event = eventQueue_.front();
        eventQueue_.erase(eventQueue_.begin());
        
        auto it = eventCallbacks_.find(event.type);
        if (it != eventCallbacks_.end()) {
            for (const auto& callback : it->second) {
                callback(event);
            }
        }
    }
}

void Quest::updateTimers(double deltaTime) {
    // 更新各种计时器
    if (state_ == QuestState::ACTIVE) {
        stats_.timeSpent += deltaTime;
    }
}

void Quest::checkTimeLimit() {
    if (timeLimit_ > 0 && getTimeSpent() >= timeLimit_) {
        state_ = QuestState::EXPIRED;
        QUEST_LOG_WARN("任务 %s 超时", name_.c_str());
        emitEvent(QuestEventType::QUEST_EXPIRED, "QuestExpired");
        onExpired();
    }
}

void Quest::updateObjectiveProgress(const std::string& objectiveId, int progress) {
    updateObjective(objectiveId, progress);
}

bool Quest::validateObjective(const std::string& objectiveId) const {
    return getObjectiveIndex(objectiveId) != -1;
}

int Quest::getObjectiveIndex(const std::string& objectiveId) const {
    for (size_t i = 0; i < objectives_.size(); ++i) {
        if (objectives_[i].id == objectiveId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void Quest::calculateRewards() {
    // 计算奖励逻辑
    QUEST_LOG_INFO("计算任务 %s 的奖励", name_.c_str());
}

bool Quest::checkPrerequisites() const {
    // 检查前置条件
    return true;
}

void Quest::triggerFollowUps() {
    // 触发后续任务
    QUEST_LOG_INFO("触发任务 %s 的后续任务", name_.c_str());
}

void Quest::initObjectives() {
    QUEST_LOG_INFO("初始化任务 %s 的目标", name_.c_str());
    for (auto& objective : objectives_) {
        objective.current = 0;
        objective.completed = false;
    }
}

bool Quest::giveRewards() {
    return grantRewards();
}

// 奖励发放方法
bool Quest::giveReward(const QuestReward& reward) {
    if (!character_) {
        QUEST_LOG_ERROR("没有角色对象，无法发放奖励");
        return false;
    }
    
    // 根据奖励类型分发
    switch (reward.type[0]) {
        case 'e': // experience
            return giveExperienceReward(reward);
        case 'g': // gold
            return giveGoldReward(reward);
        case 'i': // item
            return giveItemReward(reward);
        case 's': // skill
            return giveSkillReward(reward);
        case 'r': // reputation
            return giveReputationReward(reward);
        case 't': // title
            return giveTitleReward(reward);
        default:
            QUEST_LOG_WARN("未知的奖励类型: %s", reward.type.c_str());
            return false;
    }
}

bool Quest::giveExperienceReward(const QuestReward& reward) {
    if (!character_) {
        return false;
    }
    
    // 这里需要调用角色的经验值增加方法
    // character_->addExperience(reward.quantity);
    QUEST_LOG_INFO("给予经验奖励: %d", reward.quantity);
    return true;
}

bool Quest::giveGoldReward(const QuestReward& reward) {
    if (!character_) {
        return false;
    }
    
    // 这里需要调用角色的金币增加方法
    // character_->addGold(reward.quantity);
    QUEST_LOG_INFO("给予金币奖励: %d", reward.quantity);
    return true;
}

bool Quest::giveItemReward(const QuestReward& reward) {
    if (!character_) {
        return false;
    }
    
    // 这里需要调用角色的物品添加方法
    // character_->addItem(reward.targetId, reward.quantity);
    QUEST_LOG_INFO("给予物品奖励: %s x%d", reward.targetId.c_str(), reward.quantity);
    return true;
}

bool Quest::giveSkillReward(const QuestReward& reward) {
    if (!character_) {
        return false;
    }
    
    // 这里需要调用角色的技能学习方法
    // character_->learnSkill(reward.targetId);
    QUEST_LOG_INFO("给予技能奖励: %s", reward.targetId.c_str());
    return true;
}

bool Quest::giveReputationReward(const QuestReward& reward) {
    if (!character_) {
        return false;
    }
    
    // 这里需要调用角色的声望增加方法
    // character_->addReputation(reward.targetId, reward.quantity);
    QUEST_LOG_INFO("给予声望奖励: %s +%d", reward.targetId.c_str(), reward.quantity);
    return true;
}

bool Quest::giveTitleReward(const QuestReward& reward) {
    if (!character_) {
        return false;
    }
    
    (void)reward; // 避免未使用参数警告
    
    // 这里需要调用角色的称号添加方法
    // character_->addTitle(reward.targetId);
    QUEST_LOG_INFO("给予称号奖励: %s", reward.targetId.c_str());
    return true;
}

// 需求检查方法
bool Quest::checkRequirement(const QuestRequirement& req) const {
    if (req.type == "level") {
        return checkLevelRequirement(req.minValue);
    } else if (req.type == "quest") {
        return checkQuestRequirement(req.targetId, QuestState::COMPLETED);
    } else if (req.type == "item") {
        return checkItemRequirement(req.targetId, req.minValue);
    }
    return true;
}

void Quest::calculateProgress() {
    if (objectives_.empty()) {
        progress_ = 0.0;
        return;
    }
    
    int completedCount = 0;
    for (const auto& objective : objectives_) {
        if (objective.completed) {
            completedCount++;
        }
    }
    
    progress_ = (static_cast<double>(completedCount) / objectives_.size()) * 100.0;
}

} // namespace Fantasy 