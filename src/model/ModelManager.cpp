#include "ModelManager.h"
#include "../events/ModelEvents.h"
#include "../importers/IShapeImporter.h"

ModelManager::ModelManager(const Handle(AIS_InteractiveContext)& theContext)
    : myContext(theContext)
{}

ModelManager::~ModelManager() {}

bool ModelManager::importModel(const std::string& filePath,
                             std::shared_ptr<IShapeImporter> importer)
{
    if (!importer)
        return false;

    // 保存当前对象以便清理
    auto oldObjects = myObjects;
    myObjects.clear();

    // 使用导入器加载模型
    bool success = importer->Import(filePath.c_str(), myContext, myObjects);
    
    if (!success) {
        // 恢复原始对象列表
        myObjects = oldObjects;
        return false;
    }

    // 移除旧对象
    for (const auto& obj : oldObjects) {
        myContext->Remove(obj, true);
        
        // 触发对象移除事件
        ModelEventData eventData;
        eventData.type = ModelEventType::ObjectRemoved;
        eventData.object = obj;
        ModelEventSystem::getInstance().fireEvent(eventData);
    }

    // 触发新对象添加事件
    for (const auto& obj : myObjects) {
        ModelEventData eventData;
        eventData.type = ModelEventType::ObjectAdded;
        eventData.object = obj;
        ModelEventSystem::getInstance().fireEvent(eventData);
    }

    return true;
}

void ModelManager::setDisplayMode(const Handle(AIS_InteractiveObject)& theObject, int theMode)
{
    if (!theObject.IsNull() && myContext->IsDisplayed(theObject)) {
        myContext->SetDisplayMode(theObject, theMode, Standard_True);
        
        // 触发显示模式改变事件
        ModelEventData eventData;
        eventData.type = ModelEventType::DisplayModeChanged;
        eventData.object = theObject;
        eventData.intValue = theMode;
        ModelEventSystem::getInstance().fireEvent(eventData);
    }
}

void ModelManager::setVisibility(const Handle(AIS_InteractiveObject)& theObject, bool isVisible)
{
    if (!theObject.IsNull()) {
        if (isVisible) {
            myContext->Display(theObject, Standard_False);
        }
        else {
            myContext->Erase(theObject, Standard_False);
        }
        myContext->UpdateCurrentViewer();
        
        // 触发可见性改变事件
        ModelEventData eventData;
        eventData.type = ModelEventType::VisibilityChanged;
        eventData.object = theObject;
        eventData.boolValue = isVisible;
        ModelEventSystem::getInstance().fireEvent(eventData);
    }
}

void ModelManager::setSelectionMode(const Handle(AIS_InteractiveObject)& theObject, int theMode)
{
    if (!theObject.IsNull()) {
        // 清除所有选择模式
        myContext->Deactivate(theObject);

        // 设置新的选择模式
        if (theMode >= 0) {
            myContext->Activate(theObject, theMode);
        }
        
        // 触发对象修改事件
        ModelEventData eventData;
        eventData.type = ModelEventType::ObjectModified;
        eventData.object = theObject;
        eventData.intValue = theMode;
        ModelEventSystem::getInstance().fireEvent(eventData);
    }
}

void ModelManager::addObject(const Handle(AIS_InteractiveObject)& theObject)
{
    if (!theObject.IsNull()) {
        myObjects.push_back(theObject);
        
        // 触发对象添加事件
        ModelEventData eventData;
        eventData.type = ModelEventType::ObjectAdded;
        eventData.object = theObject;
        ModelEventSystem::getInstance().fireEvent(eventData);
    }
}

void ModelManager::removeObject(const Handle(AIS_InteractiveObject)& theObject)
{
    if (!theObject.IsNull()) {
        auto it = std::find(myObjects.begin(), myObjects.end(), theObject);
        if (it != myObjects.end()) {
            myObjects.erase(it);
            myContext->Remove(theObject, Standard_True);
            
            // 触发对象移除事件
            ModelEventData eventData;
            eventData.type = ModelEventType::ObjectRemoved;
            eventData.object = theObject;
            ModelEventSystem::getInstance().fireEvent(eventData);
        }
    }
}

void ModelManager::selectObject(const Handle(AIS_InteractiveObject)& theObject, bool clearFirst)
{
    if (!theObject.IsNull()) {
        if (clearFirst) {
            myContext->ClearSelected(Standard_False);
        }
        myContext->AddOrRemoveSelected(theObject, Standard_True);
        
        // 触发选择改变事件
        ModelEventData eventData;
        eventData.type = ModelEventType::SelectionChanged;
        eventData.object = theObject;
        ModelEventSystem::getInstance().fireEvent(eventData);
    }
}

void ModelManager::clearSelection()
{
    myContext->ClearSelected(Standard_True);
    
    // 触发选择改变事件
    ModelEventData eventData;
    eventData.type = ModelEventType::SelectionChanged;
    eventData.object = nullptr;
    ModelEventSystem::getInstance().fireEvent(eventData);
}