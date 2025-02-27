#pragma once

#include <AIS_InteractiveObject.hxx>
#include <functional>
#include <map>
#include <string>
#include <vector>

// 模型事件类型
enum class ModelEventType {
    ObjectAdded,
    ObjectRemoved,
    ObjectModified,
    SelectionChanged,
    DisplayModeChanged,
    VisibilityChanged
};

// 模型事件数据
struct ModelEventData {
    ModelEventType type;
    Handle(AIS_InteractiveObject) object;
    int intValue = 0;  // 通用整数值字段（如显示模式）
    bool boolValue = false;  // 通用布尔值字段（如可见性）
};

// 事件监听器基类
class IModelEventListener {
public:
    virtual ~IModelEventListener() = default;
    virtual void onModelEvent(const ModelEventData& eventData) = 0;
};

// 事件系统
class ModelEventSystem {
public:
    static ModelEventSystem& getInstance() {
        static ModelEventSystem instance;
        return instance;
    }

    void registerListener(IModelEventListener* listener, const std::string& id) {
        listeners[id] = listener;
    }

    void unregisterListener(const std::string& id) {
        listeners.erase(id);
    }

    void fireEvent(const ModelEventData& eventData) {
        for (auto& pair : listeners) {
            if (pair.second) {
                pair.second->onModelEvent(eventData);
            }
        }
    }

private:
    ModelEventSystem() = default;
    std::map<std::string, IModelEventListener*> listeners;
}; 