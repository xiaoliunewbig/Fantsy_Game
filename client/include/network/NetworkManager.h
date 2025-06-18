/**
 * @file NetworkManager.h
 * @brief 网络管理器
 * @author [pengchengkang]
 * @date 2025.06.17
 */

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>

namespace Fantasy {

/**
 * @brief 网络管理器类
 * 
 * 负责处理网络通信
 */
class NetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkManager(QObject* parent = nullptr);
    ~NetworkManager();

    // 网络控制
    void connectToServer(const QString& address, int port);
    void disconnectFromServer();
    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void error(const QString& error);

private:
    bool m_isConnected;  ///< 是否已连接
};

} // namespace Fantasy

#endif // NETWORKMANAGER_H 