#pragma once

#include <map>
#include <vector>
#include <functional>
#include <any>

namespace MVVM {

class MessageBus {
public:
    enum class MessageType {
        ModelChanged,
        SelectionChanged,
        ViewChanged,
        CommandExecuted
    };
    
    struct Message {
        MessageType type;
        std::any data;
    };
    
    void subscribe(MessageType type, std::function<void(const Message&)> handler) {
        mySubscribers[type].push_back(handler);
    }
    
    void publish(const Message& message) {
        for (auto& handler : mySubscribers[message.type]) {
            handler(message);
        }
    }
    
    // 单例实现
    static MessageBus& getInstance() {
        static MessageBus instance;
        return instance;
    }
    
private:
    MessageBus() = default;
    std::map<MessageType, std::vector<std::function<void(const Message&)>>> mySubscribers;
};

} // namespace MVVM 