#pragma once

#include <string>
#include <functional>
#include <vector>

class IModel {
public:
    virtual ~IModel() = default;
    
    // 通用模型操作
    virtual std::vector<std::string> getAllEntityIds() const = 0;
    virtual void removeEntity(const std::string& id) = 0;
    
    // 事件通知系统
    using ChangeListener = std::function<void(const std::string&)>;
    void addChangeListener(ChangeListener listener);
    
protected:
    void notifyChange(const std::string& entityId);
    
    std::vector<ChangeListener> myChangeListeners;
}; 