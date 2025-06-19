/**
 * @file ConnectionPool.h
 * @brief 网络连接池 - 管理客户端与服务器的持久连接
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 支持连接复用
 * - 支持自动重连机制
 * - 支持连接超时控制
 * - 支持连接状态监控（TODO）
 * - 支持SSL/TLS加密连接（TODO）
 */
#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

class NetworkConnection;

class ConnectionPool {
public:
    /**
     * @brief 获取连接池单例实例
     * @return 单例引用
     */
    static ConnectionPool& GetInstance();

    /**
     * @brief 初始化连接池
     * @param maxConnections 最大连接数
     */
    void Init(int maxConnections);

    /**
     * @brief 获取一个可用连接
     * @param host 主机地址
     * @param port 端口号
     * @return 连接对象指针
     */
    std::shared_ptr<NetworkConnection> GetConnection(const std::string& host, int port);

    /**
     * @brief 释放指定连接
     * @param connection 要释放的连接
     */
    void ReleaseConnection(const std::shared_ptr<NetworkConnection>& connection);

    /**
     * @brief 关闭所有空闲连接
     */
    void CloseIdleConnections();

    /**
     * @brief 设置连接超时时间（毫秒）
     * @param timeoutMs 超时时间
     */
    void SetTimeout(int timeoutMs);

    /**
     * @brief 获取当前活跃连接数
     * @return 活跃连接数量
     */
    int GetActiveConnectionsCount() const;

    /**
     * @brief 获取当前空闲连接数
     * @return 空闲连接数量
     */
    int GetIdleConnectionsCount() const;

    /**
     * @brief 启用SSL/TLS加密连接
     * @param enable 是否启用
     * @todo 实现SSL支持
     */
    void EnableSSL(bool enable);

private:
    ConnectionPool() = default;
    ~ConnectionPool();

    struct ConnectionKey {
        std::string host;
        int port;
        bool operator==(const ConnectionKey&) const;
    };

    struct ConnectionHash {
        size_t operator()(const ConnectionKey&) const;
    };

    std::unordered_map<ConnectionKey, std::vector<std::shared_ptr<NetworkConnection>>, ConnectionHash> connections_;
    std::mutex mutex_;
    int maxConnections_;
    int timeoutMs_;
    bool sslEnabled_;

    // 禁止拷贝
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;
};

#endif // CONNECTION_POOL_H