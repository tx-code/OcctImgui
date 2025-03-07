#pragma once

#include "model/IModel.h"
#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <memory>
#include <string>
#include <vector>

class IViewModel
{
public:
    virtual ~IViewModel() = default;

    // 通用UI操作
    virtual void deleteSelectedObjects() = 0;

    // 访问器
    virtual Handle(AIS_InteractiveContext) getContext() const = 0;
    virtual std::shared_ptr<IModel> getModel() const = 0;
};