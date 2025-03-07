/**
 * @brief MessageBus - 集中式消息分发系统
 *
 * 特点:
 * - 集中式通信：所有消息通过一个中央"总线"传递
 * - 基于类型的消息：使用预定义的MessageType枚举
 * - 松散耦合：发送者不需要知道接收者，接收者只需订阅感兴趣的消息类型
 * - 数据传递：通过std::any可以传递任意类型的数据
 * - 简单实现：使用std::map和std::vector实现，没有依赖外部库
 *
 * @note MessageBus与Signal的区别:
 * - MessageBus：适用于系统级别的通信，如模型变更通知、视图状态改变等。
 *   它是一种"广播"机制，一条消息可以被多个不相关的组件接收。
 * - Signal：适用于组件内部或紧密相关组件之间的通信，如UI控件的事件处理。
 *   它提供更精细的类型安全和资源管理。
 *
 * @see Signal.h
 *
 */

#pragma once

#include "SelectionInfo.h"

#include <AIS_InteractiveObject.hxx>
#include <TopoDS_Shape.hxx>
#include <any>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace MVVM
{

// 前向声明
class MessageBus;

/**
 * @brief 订阅管理类，用于自动管理订阅的生命周期
 *
 * 当Subscription对象被销毁时，会自动取消所有订阅
 */
class Subscription
{
public:
    Subscription() = default;

    // 单个令牌的构造函数
    Subscription(MessageBus& bus, size_t token)
        : myBus(&bus)
    {
        myTokens.push_back(token);
    }

    // 多个令牌的构造函数
    Subscription(MessageBus& bus, const std::vector<size_t>& tokens)
        : myBus(&bus)
        , myTokens(tokens)
    {}

    // 移动构造函数
    Subscription(Subscription&& other) noexcept
        : myBus(other.myBus)
        , myTokens(std::move(other.myTokens))
    {
        other.myTokens.clear();
    }

    // 移动赋值操作符
    Subscription& operator=(Subscription&& other) noexcept
    {
        if (this != &other) {
            unsubscribe();
            myBus = other.myBus;
            myTokens = std::move(other.myTokens);
            other.myTokens.clear();
        }
        return *this;
    }

    // 禁用拷贝
    Subscription(const Subscription&) = delete;
    Subscription& operator=(const Subscription&) = delete;

    // 析构函数 - 自动取消所有订阅
    ~Subscription()
    {
        unsubscribe();
    }

    // 添加新的订阅令牌
    void addToken(size_t token)
    {
        if (myBus) {
            myTokens.push_back(token);
        }
    }

    // 手动取消所有订阅
    void unsubscribe();

    // 取消特定的订阅
    bool unsubscribeToken(size_t token);

    // 检查是否有活跃的订阅
    bool hasActiveSubscriptions() const
    {
        return !myTokens.empty();
    }

    // 获取所有令牌
    const std::vector<size_t>& getTokens() const
    {
        return myTokens;
    }

private:
    MessageBus* myBus = nullptr;
    std::vector<size_t> myTokens;
};

class MessageBus
{
public:
    // 获取单例实例
    static MessageBus& getInstance()
    {
        static MessageBus instance;
        return instance;
    }

    // 删除拷贝构造函数和赋值操作符
    MessageBus(const MessageBus&) = delete;
    MessageBus& operator=(const MessageBus&) = delete;

    enum class MessageType
    {
        ModelChanged,
        SelectionChanged,
        ViewChanged,
        CommandExecuted
    };

    struct Message
    {
        MessageType type;
        std::any data;
    };

    // 订阅令牌类型，用于标识订阅并允许取消订阅
    using SubscriptionToken = size_t;

    /**
     * @brief 订阅指定类型的消息
     * @param type 消息类型
     * @param handler 消息处理函数
     * @return 订阅令牌，可用于取消订阅
     */
    template<typename Handler>
    SubscriptionToken subscribe(MessageType type, Handler&& handler)
    {
        auto token = myNextToken++;
        mySubscribers[type].push_back({token, std::forward<Handler>(handler)});
        return token;
    }

    /**
     * @brief 订阅指定类型的消息，返回Subscription对象
     * @param type 消息类型
     * @param handler 消息处理函数
     * @return Subscription对象，可自动管理订阅生命周期
     */
    template<typename Handler>
    Subscription subscribeWithManager(MessageType type, Handler&& handler)
    {
        auto token = subscribe(type, std::forward<Handler>(handler));
        return Subscription(*this, token);
    }

    /**
     * @brief 订阅指定类型的消息，并将其添加到Subscription对象中
     * @param subscription 要添加订阅的Subscription对象
     * @param type 消息类型
     * @param handler 消息处理函数
     */
    template<typename Handler>
    void subscribeWithManager(Subscription& subscription, MessageType type, Handler&& handler)
    {
        subscription.addToken(subscribe(type, std::forward<Handler>(handler)));
    }

    /**
     * @brief 订阅多个消息类型，对每种类型使用相同的处理函数
     * @param types 消息类型列表
     * @param handler 消息处理函数
     * @return Subscription对象，可自动管理所有订阅的生命周期
     */
    template<typename Handler>
    Subscription subscribeMultiple(const std::vector<MessageType>& types, Handler&& handler)
    {
        std::vector<SubscriptionToken> tokens;
        tokens.reserve(types.size());

        for (const auto& type : types) {
            tokens.push_back(subscribe(type, handler));
        }

        return Subscription(*this, tokens);
    }

    /**
     * @brief 取消订阅
     * @param token 订阅时返回的令牌
     * @return 是否成功取消订阅
     */
    bool unsubscribe(SubscriptionToken token)
    {
        for (auto& [type, handlers] : mySubscribers) {
            auto it = std::find_if(handlers.begin(), handlers.end(), [token](const auto& entry) {
                return entry.first == token;
            });
            if (it != handlers.end()) {
                handlers.erase(it);
                return true;
            }
        }
        return false;
    }

    /**
     * @brief 发布消息
     * @param message 要发布的消息
     */
    void publish(const Message& message)
    {
        for (auto& handler : mySubscribers[message.type]) {
            handler.second(message);
        }
    }

    /**
     * @brief 清除所有订阅
     */
    void clearAllSubscriptions()
    {
        mySubscribers.clear();
    }

private:
    // 私有构造函数
    MessageBus()
        : myNextToken(1)
    {}

    // 订阅者存储结构：消息类型 -> {令牌, 处理函数}的列表
    std::map<MessageType,
             std::vector<std::pair<SubscriptionToken, std::function<void(const Message&)>>>>
        mySubscribers;

    // 下一个可用的订阅令牌
    SubscriptionToken myNextToken;
};

// Subscription类的unsubscribe方法实现
inline void Subscription::unsubscribe()
{
    if (!myTokens.empty() && myBus) {
        for (size_t token : myTokens) {
            myBus->unsubscribe(token);
        }
        myTokens.clear();
    }
}

// Subscription类的unsubscribeToken方法实现
inline bool Subscription::unsubscribeToken(size_t token)
{
    if (myBus) {
        auto it = std::find(myTokens.begin(), myTokens.end(), token);
        if (it != myTokens.end()) {
            bool result = myBus->unsubscribe(token);
            if (result) {
                myTokens.erase(it);
            }
            return result;
        }
    }
    return false;
}

}  // namespace MVVM