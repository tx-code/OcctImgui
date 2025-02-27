#pragma once

#include <AIS_InteractiveContext.hxx>
#include "../events/ModelEvents.h"

// GUI组件接口
class IGUIComponent : public IModelEventListener {
public:
    virtual ~IGUIComponent() = default;
    
    // 渲染接口
    virtual void render() = 0;
    
    // 获取可见性状态
    virtual bool isVisible() const = 0;
    
    // 设置可见性状态
    virtual void setVisible(bool visible) = 0;
    
    // 实现事件监听接口
    virtual void onModelEvent(const ModelEventData& eventData) override = 0;
}; 