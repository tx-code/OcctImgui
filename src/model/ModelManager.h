#pragma once

#include "../events/ModelEvents.h"
#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <memory>
#include <vector>

// Forward declarations
class IShapeImporter;

class ModelManager
{
public:
    ModelManager(const Handle(AIS_InteractiveContext) & theContext);
    ~ModelManager();

    // 模型操作方法
    bool importModel(const std::string& filePath, std::shared_ptr<IShapeImporter> importer);
    void setDisplayMode(const Handle(AIS_InteractiveObject) & theObject, int theMode);
    void setVisibility(const Handle(AIS_InteractiveObject) & theObject, bool isVisible);
    void setSelectionMode(const Handle(AIS_InteractiveObject) & theObject, int theMode);

    // 访问器
    const std::vector<Handle(AIS_InteractiveObject)>& getObjects() const
    {
        return myObjects;
    }
    Handle(AIS_InteractiveContext) getContext() const
    {
        return myContext;
    }

    // 添加和删除对象
    void addObject(const Handle(AIS_InteractiveObject) & theObject);
    void removeObject(const Handle(AIS_InteractiveObject) & theObject);

    // 选择对象
    void selectObject(const Handle(AIS_InteractiveObject) & theObject, bool clearFirst = true);
    void clearSelection();

private:
    Handle(AIS_InteractiveContext) myContext;
    std::vector<Handle(AIS_InteractiveObject)> myObjects;
};
