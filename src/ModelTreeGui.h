#pragma once

#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <AIS_Shape.hxx>
#include <map>
#include <vector>

class ModelTreeGui
{
public:
    ModelTreeGui()
        : myIsVisible(true)
    {}

    void Show(const Handle(AIS_InteractiveContext) & theContext,
              std::vector<Handle(AIS_InteractiveObject)>& theObjects);

    bool& IsVisible()
    {
        return myIsVisible;
    }

    // 清理显示模式记录
    void ClearDisplayModes()
    {
        myDisplayModes.clear();
    }

private:
    void ShowObjectNode(const Handle(AIS_InteractiveContext) & theContext,
                        const Handle(AIS_InteractiveObject) & theObject,
                        size_t theIndex);

    void ShowObjectProperties(const Handle(AIS_InteractiveContext) & theContext,
                              const Handle(AIS_InteractiveObject) & theObject);

    void ShowObjectVisibility(const Handle(AIS_InteractiveContext) & theContext,
                              const Handle(AIS_InteractiveObject) & theObject);

    void ShowDisplayMode(const Handle(AIS_InteractiveContext) & theContext,
                         const Handle(AIS_InteractiveObject) & theObject);

    bool myIsVisible;
    std::map<Handle(AIS_InteractiveObject), Standard_Integer> myDisplayModes;
};