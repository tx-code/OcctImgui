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
class MessageBus
{
public:
    // Constructor
    MessageBus() = default;

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


    template<typename Handler>
    void subscribe(MessageType type, Handler&& handler)
    {
        mySubscribers[type].push_back(std::forward<Handler>(handler));
    }

    void publish(const Message& message)
    {
        for (auto& handler : mySubscribers[message.type]) {
            handler(message);
        }
    }

private:
    std::map<MessageType, std::vector<std::function<void(const Message&)>>> mySubscribers;
};

}  // namespace MVVM