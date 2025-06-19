/**
 * @file EventBus.h
 * @brief 事件总线类定义 - 支持任意类型的事件广播和订阅
 * @author [pengchengkang]
 * @date 2025.06.19
 *
 * 功能特性:
 * - 支持多线程安全订阅与发布
 * - 支持泛型事件类型
 * - 提供订阅句柄用于取消订阅
 * - 支持事件过滤机制（TODO）
 */
#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <vector>
#include <functional>
#include <mutex>

template<typename EventType>
class EventBus {
public:
    /**
     * @brief 订阅一个事件处理函数
     * @param handler 事件处理函数
     * @return 返回一个用于取消订阅的句柄
     * @todo 后续支持基于标签或组的订阅过滤机制
     */
    int Subscribe(std::function<void(const EventType&)> handler);

    /**
     * @brief 发布一个事件，触发所有订阅者
     * @param event 要发布的事件对象
     */
    void Publish(const EventType& event);

    /**
     * @brief 取消某个订阅
     * @param handle 订阅句柄
     * @todo 实现取消订阅功能
     */
    void Unsubscribe(int handle);
};

#endif // EVENT_BUS_H