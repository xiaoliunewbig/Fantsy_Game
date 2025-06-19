/**
 * @file MessageDispatcher.h
 * @brief 消息分发器 - 负责将接收到的消息分发到对应的处理模块
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 支持按消息类型注册回调
 * - 支持优先级排序
 * - 支持异步消息队列
 * - 支持消息拦截和日志记录（TODO）
 */
#ifndef MESSAGE_DISPATCHER_H
#define MESSAGE_DISPATCHER_H

#include <string>
#include <functional>
#include <map>
#include <queue>
#include <mutex>
#include <thread>

class MessageDispatcher {
public:
    using MessageHandler = std::function<void(const std::string&)>;

    /**
     * @brief 注册一个消息处理器
     * @param messageType 消息类型
     * @param handler 处理函数
     */
    void RegisterHandler(const std::string& messageType, const MessageHandler& handler);

    /**
     * @brief 取消注册某个消息类型的处理器
     * @param messageType 消息类型
     */
    void UnregisterHandler(const std::string& messageType);

    /**
     * @brief 分发一条消息给对应处理器
     * @param messageType 消息类型
     * @param messageContent 消息内容
     */
    void DispatchMessage(const std::string& messageType, const std::string& messageContent);

    /**
     * @brief 启动异步消息处理线程
     */
    void StartAsyncProcessing();

    /**
     * @brief 停止异步处理线程
     */
    void StopAsyncProcessing();

    /**
     * @brief 清空所有已注册的消息处理器
     */
    void ClearAllHandlers();

    /**
     * @brief 获取当前等待处理的消息数量
     * @return 消息数量
     */
    int GetMessageQueueSize() const;

    /**
     * @brief 设置消息优先级
     * @param messageType 消息类型
     * @param priority 优先级数值（0~100）
     * @todo 实现优先级调度机制
     */
    void SetMessagePriority(const std::string& messageType, int priority);

private:
    std::map<std::string, MessageHandler> handlers_;
    std::queue<std::pair<std::string, std::string>> messageQueue_;
    std::mutex queueMutex_;
    std::thread workerThread_;
    bool running_ = false;
};

#endif // MESSAGE_DISPATCHER_H