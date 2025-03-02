#pragma once

#include "../model/IModel.h"
#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <vector>
#include <string>
#include <memory>

class IViewModel {
public:
    virtual ~IViewModel() = default;
    
    // 通用UI操作
    virtual void deleteSelectedObjects() = 0;
    
    // UI状态
    virtual bool hasSelection() const = 0;
    virtual std::vector<std::string> getSelectedObjects() const = 0;
    
    // 选择处理
    virtual void processSelection(const Handle(AIS_InteractiveObject)& obj, bool isSelected) = 0;
    virtual void clearSelection() = 0;
    
    // 访问器
    virtual Handle(AIS_InteractiveContext) getContext() const = 0;
    virtual std::shared_ptr<IModel> getModel() const = 0;
}; 