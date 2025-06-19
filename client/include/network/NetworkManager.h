/**
 * @file NetWorkManager.h
 * @brief 网络连接管理器接口定义 - 支持多种协议和连接池
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - TCP/UDP 支持
 * - 多连接管理
 * - 心跳包检测
 * - 自动重连机制
 * - 消息队列缓冲
 * - 异步收发支持
 * - 协议解析插件（TODO）
 */
#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <string>
#include <functional>

class NetworkManager {
public:
    /**
     * @brief 连接到指定服务器
     * @param host 主机地址
     * @param port 端口号
     * @return 是否连接成功
     */
    bool Connect(const std::string& host, int port);

    /**
     * @brief 断开当前连接
     */
    void Disconnect();

    /**
     * @brief 发送网络消息
     * @param message 消息内容
     * @return 是否发送成功
     */
    bool SendMessage(const std::string& message);

    /**
     * @brief 注册接收消息回调
     * @param callback 接收到消息时触发的回调
     */
    void OnMessageReceived(const std::function<void(const std::string&)>& callback);

    /**
     * @brief 检查当前是否连接
     * @return 是否连接
     */
    bool IsConnected() const;

    /**
     * @brief 获取当前连接状态
     * @return 状态字符串
     * @todo 实现更详细的连接状态枚举
     */
    std::string GetConnectionStatus() const;
};

#endif // NETWORK_MANAGER_H