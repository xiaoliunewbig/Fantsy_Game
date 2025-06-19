/**
 * @file MessageHandler.h
 * @brief 网络消息处理接口定义 - 负责接收和分发网络消息
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 支持多种消息类型注册
 * - 支持异步消息处理
 * - 支持消息回调绑定
 * - 支持消息过滤与优先级排序（TODO）
 */
#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <string>
#include <functional>
#include <map>

class MessageHandler {
public:
    using MessageCallback = std::function<void(const std::string&)>;

    /**
     * @brief 注册一个消息类型及其处理函数
     * @param messageType 消息类型标识符
     * @param callback 处理函数
     */
    void RegisterMessageHandler(const std::string& messageType, const MessageCallback& callback);

    /**
     * @brief 取消注册某个消息类型
     * @param messageType 消息类型标识符
     */
    void UnregisterMessageHandler(const std::string& messageType);

    /**
     * @brief 接收到新消息时调用此方法
     * @param messageType 消息类型
     * @param messageContent 消息内容
     */
    void OnMessageReceived(const std::string& messageType, const std::string& messageContent);

    /**
     * @brief 获取已注册的消息类型数量
     * @return 数量
     */
    int GetMessageHandlerCount() const;

    /**
     * @brief 清除所有消息处理器
     */
    void ClearAllHandlers();
};

#endif // MESSAGE_HANDLER_H