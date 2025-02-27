#include "ModelManager.h"
#include "importers/IShapeImporter.h"

ModelManager::ModelManager(const Handle(AIS_InteractiveContext) & theContext)
    : myContext(theContext)
{}

bool ModelManager::importModel(const std::string& filePath,
                               std::shared_ptr<IShapeImporter> importer)
{
    if (!importer)
        return false;

    // 使用导入器加载模型
    bool success = importer->Import(filePath.c_str(), myContext, myObjects);
    if (!success)
        return false;

    return true;
}

void ModelManager::setDisplayMode(const Handle(AIS_InteractiveObject) & theObject, int theMode)
{
    if (!theObject.IsNull() && myContext->IsDisplayed(theObject)) {
        myContext->SetDisplayMode(theObject, theMode, Standard_True);
    }
}

void ModelManager::setVisibility(const Handle(AIS_InteractiveObject) & theObject, bool isVisible)
{
    if (!theObject.IsNull()) {
        if (isVisible) {
            myContext->Display(theObject, Standard_False);
        }
        else {
            myContext->Erase(theObject, Standard_False);
        }
        myContext->UpdateCurrentViewer();
    }
}

void ModelManager::setSelectionMode(const Handle(AIS_InteractiveObject) & theObject, int theMode)
{
    if (!theObject.IsNull()) {
        // 清除所有选择模式
        myContext->AddOrRemoveSelected(theObject, Standard_True);

        // 设置新的选择模式
        if (theMode > 0) {
            myContext->Activate(theObject, theMode);
        }
    }
}

void ModelManager::addObject(const Handle(AIS_InteractiveObject) & theObject)
{
    if (!theObject.IsNull()) {
        myObjects.push_back(theObject);
    }
}

void ModelManager::removeObject(const Handle(AIS_InteractiveObject) & theObject)
{
    if (!theObject.IsNull()) {
        auto it = std::find(myObjects.begin(), myObjects.end(), theObject);
        if (it != myObjects.end()) {
            myContext->Remove(theObject, Standard_True);
            myObjects.erase(it);
        }
    }
}