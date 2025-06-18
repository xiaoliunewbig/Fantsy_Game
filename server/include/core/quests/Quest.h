/**
 * @file Quest.h
 * @brief 任务系统 - 游戏任务管理
 * @author [pengchengkang]
 * @date 2025.06.17
 * 
 * 功能特性:
 * - 任务类型管理
 * - 任务目标跟踪
 * - 任务奖励系统
 * - 任务链系统
 * - 任务状态管理
 * - 任务事件系统
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <functional>
#include <variant>

namespace Fantasy {

// 前向声明
class Character;

// 任务类型枚举
enum class QuestType {
    MAIN,           // 主线任务
    SIDE,           // 支线任务
    DAILY,          // 日常任务
    WEEKLY,         // 周常任务
    EVENT,          // 活动任务
    REPEATABLE,     // 可重复任务
    HIDDEN,         // 隐藏任务
    GUILD,          // 公会任务
    PVP,            // PVP任务
    ACHIEVEMENT     // 成就任务
};

// 任务状态枚举
enum class QuestState {
    NOT_STARTED,    // 未开始
    INACTIVE,       // 未激活
    ACTIVE,         // 进行中
    COMPLETED,      // 已完成
    FAILED,         // 失败
    ABANDONED,      // 放弃
    EXPIRED,        // 过期
    ERROR           // 错误
};

// 任务目标类型枚举
enum class QuestObjectiveType {
    KILL_ENEMIES,   // 击杀敌人
    COLLECT_ITEMS,  // 收集物品
    TALK_TO_NPC,    // 与NPC对话
    REACH_LOCATION, // 到达位置
    CRAFT_ITEMS,    // 制作物品
    COMPLETE_QUESTS, // 完成任务
    GAIN_LEVELS,    // 提升等级
    EARN_CURRENCY,  // 获得货币
    USE_SKILLS,     // 使用技能
    DEFEAT_BOSS     // 击败Boss
};

// 任务目标结构
struct QuestObjective {
    std::string id;
    std::string description;
    QuestObjectiveType type;
    std::string targetId;
    int required;
    int current;
    bool completed;
    std::vector<std::string> subObjectives;
    
    QuestObjective() : type(QuestObjectiveType::KILL_ENEMIES), required(1), current(0), completed(false) {}
    QuestObjective(const std::string& id, const std::string& desc, QuestObjectiveType type, 
                  const std::string& target, int req)
        : id(id), description(desc), type(type), targetId(target), required(req), current(0), completed(false) {}
};

// 任务奖励结构
struct QuestReward {
    std::string id;
    std::string type;  // "experience", "currency", "item", "skill", "title"
    std::string targetId;
    int quantity;
    bool isPercentage;
    std::string description;
    
    QuestReward() : quantity(0), isPercentage(false) {}
    QuestReward(const std::string& id, const std::string& type, const std::string& target, int qty)
        : id(id), type(type), targetId(target), quantity(qty), isPercentage(false) {}
};

// 任务需求结构
struct QuestRequirement {
    std::string type;  // "level", "quest", "item", "skill", "reputation"
    std::string targetId;
    int minValue;
    std::string operator_;  // ">=", "==", "<=", "!="
    
    QuestRequirement() : minValue(0), operator_(">=") {}
    QuestRequirement(const std::string& type, const std::string& target, int value, const std::string& op = ">=")
        : type(type), targetId(target), minValue(value), operator_(op) {}
};

// 任务事件类型
enum class QuestEventType {
    QUEST_STARTED,
    QUEST_COMPLETED,
    QUEST_FAILED,
    QUEST_ABANDONED,
    OBJECTIVE_UPDATED,
    OBJECTIVE_COMPLETED,
    REWARD_GRANTED,
    QUEST_EXPIRED,
    QUEST_CHAIN_STARTED,
    QUEST_CHAIN_COMPLETED
};

// 任务事件数据
using QuestEventData = std::unordered_map<std::string, std::string>;

// 任务事件结构
struct QuestEvent {
    QuestEventType type;
    std::string name;
    QuestEventData data;
    std::chrono::system_clock::time_point timestamp;
    
    QuestEvent(QuestEventType t, const std::string& n, const QuestEventData& d = QuestEventData{})
        : type(t), name(n), data(d), timestamp(std::chrono::system_clock::now()) {}
};

// 事件回调函数类型
using EventCallback = std::function<void(const QuestEvent&)>;

// 任务配置结构
struct QuestConfig {
    std::string id;
    std::string name;
    std::string description;
    QuestType type;
    int level;
    std::string giverId;
    std::string turnInId;
    std::vector<QuestRequirement> requirements;
    std::vector<QuestObjective> objectives;
    std::vector<QuestReward> rewards;
    std::vector<std::string> prerequisites;
    std::vector<std::string> followUps;
    int timeLimit;  // 时间限制（秒），0表示无限制
    bool repeatable;
    int maxCompletions;
    std::string category;
    
    QuestConfig() : type(QuestType::SIDE), level(1), timeLimit(0), repeatable(false), maxCompletions(1) {}
};

// 任务统计结构
struct QuestStats {
    std::chrono::system_clock::time_point creationTime;
    std::chrono::system_clock::time_point lastStartTime;
    std::chrono::system_clock::time_point lastCompletionTime;
    std::chrono::system_clock::time_point lastFailureTime;
    std::chrono::system_clock::time_point lastAbandonTime;
    int totalStarts;
    int totalCompletions;
    int totalFailures;
    int totalAbandons;
    int totalEvents;
    double timeSpent;  // 添加时间花费字段
    
    QuestStats() : totalStarts(0), totalCompletions(0), totalFailures(0), totalAbandons(0), totalEvents(0), timeSpent(0.0) {}
};

// 日志宏定义
#define QUEST_LOG_DEBUG(fmt, ...) 
#define QUEST_LOG_INFO(fmt, ...) 
#define QUEST_LOG_WARN(fmt, ...) 
#define QUEST_LOG_ERROR(fmt, ...) 

/**
 * @brief 任务类
 * 
 * 提供任务的基础功能：
 * - 任务配置管理
 * - 目标跟踪
 * - 奖励发放
 * - 状态管理
 * - 事件系统
 */
class Quest {
public:
    // 构造和析构
    Quest(const std::string& id, const std::string& name, QuestType type, const QuestConfig& config);
    virtual ~Quest();
    
    // 禁用拷贝
    Quest(const Quest&) = delete;
    Quest& operator=(const Quest&) = delete;
    
    // 任务生命周期
    virtual bool start(std::shared_ptr<Character> character);
    virtual bool complete();
    virtual bool fail();
    virtual bool abandon();
    virtual void update(double deltaTime);
    
    // 目标管理
    virtual bool updateObjective(const std::string& objectiveId, int progress);
    virtual bool completeObjective(const std::string& objectiveId);
    virtual bool isObjectiveCompleted(const std::string& objectiveId) const;
    virtual QuestObjective getObjective(const std::string& objectiveId) const;
    virtual std::vector<QuestObjective> getObjectives() const;
    virtual bool areAllObjectivesCompleted() const;
    
    // 需求检查
    virtual bool canStart(std::shared_ptr<Character> character) const;
    virtual bool meetsRequirements() const;
    virtual bool checkLevelRequirement(int level) const;
    virtual bool checkQuestRequirement(const std::string& questId, QuestState state) const;
    virtual bool checkItemRequirement(const std::string& itemId, int quantity) const;
    
    // 奖励管理
    virtual bool grantRewards();
    virtual bool areRewardsClaimed() const { return rewardsClaimed_; }
    virtual std::vector<QuestReward> getRewards() const { return rewards_; }
    
    // 事件系统
    virtual void emitEvent(const QuestEvent& event);
    virtual void emitEvent(QuestEventType type, const std::string& name, 
                          const QuestEventData& data = QuestEventData{});
    virtual void subscribeToEvent(QuestEventType type, EventCallback callback);
    virtual void unsubscribeFromEvent(QuestEventType type, EventCallback callback);
    
    // 状态检查
    virtual bool isStarted() const { return state_ == QuestState::ACTIVE; }
    virtual bool isCompleted() const { return state_ == QuestState::COMPLETED; }
    virtual bool isFailed() const { return state_ == QuestState::FAILED; }
    virtual bool isExpired() const { return state_ == QuestState::EXPIRED; }
    virtual bool isAbandoned() const { return state_ == QuestState::ABANDONED; }
    virtual bool canComplete() const;
    virtual bool canAbandon() const;
    virtual bool isRepeatable() const { return repeatable_; }
    virtual bool canRepeat() const;
    
    // 获取器
    const std::string& getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getDescription() const { return description_; }
    QuestType getType() const { return type_; }
    int getLevel() const { return level_; }
    QuestState getState() const { return state_; }
    double getProgress() const { return progress_; }
    int getCompletionCount() const { return completionCount_; }
    double getTimeSpent() const;
    double getTimeRemaining() const;
    std::shared_ptr<Character> getCharacter() const { return character_; }
    
    // 设置器
    virtual void setState(QuestState state) { state_ = state; }
    virtual void setProgress(double progress) { progress_ = progress; }
    virtual void setCompletionCount(int count) { completionCount_ = count; }
    
    // 事件回调
    virtual void onStarted();
    virtual void onCompleted();
    virtual void onFailed();
    virtual void onAbandoned();
    virtual void onExpired();
    virtual void onObjectiveUpdated(const std::string& objectiveId);
    virtual void onObjectiveCompleted(const std::string& objectiveId);
    virtual void onRewardsGranted();
    virtual void onChainStarted();
    virtual void onChainCompleted();

protected:
    // 内部方法
    void initializeQuest();
    void processEvents();
    void updateTimers(double deltaTime);
    void checkTimeLimit();
    void updateObjectiveProgress(const std::string& objectiveId, int progress);
    bool validateObjective(const std::string& objectiveId) const;
    int getObjectiveIndex(const std::string& objectiveId) const;
    void calculateRewards();
    bool checkPrerequisites() const;
    void triggerFollowUps();
    void initObjectives();
    bool giveRewards();
    
    // 奖励发放方法
    bool giveReward(const QuestReward& reward);
    bool giveExperienceReward(const QuestReward& reward);
    bool giveGoldReward(const QuestReward& reward);
    bool giveItemReward(const QuestReward& reward);
    bool giveSkillReward(const QuestReward& reward);
    bool giveReputationReward(const QuestReward& reward);
    bool giveTitleReward(const QuestReward& reward);
    
    // 需求检查方法
    bool checkRequirement(const QuestRequirement& req) const;
    void calculateProgress();

private:
    // 基础属性
    std::string id_;
    std::string name_;
    std::string description_;
    QuestType type_;
    int level_;
    QuestState state_;
    double progress_;
    int completionCount_;
    bool repeatable_;
    int maxCompletions_;  // 添加最大完成次数
    bool rewardsClaimed_;
    
    // 配置
    QuestConfig config_;
    std::vector<QuestObjective> objectives_;
    std::vector<QuestReward> rewards_;
    std::vector<QuestRequirement> requirements_;
    QuestStats stats_;
    
    // 时间管理
    std::chrono::system_clock::time_point startTime_;
    std::chrono::system_clock::time_point completionTime_;
    int timeLimit_;
    
    // 关联对象
    std::shared_ptr<Character> character_;
    
    // 事件系统
    std::vector<QuestEvent> eventQueue_;
    std::unordered_map<QuestEventType, std::vector<EventCallback>> eventCallbacks_;
};

} // namespace Fantasy 