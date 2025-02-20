#pragma once

#include <AIS_InteractiveContext.hxx>
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
              std::vector<Handle(AIS_Shape)>& theShapes);

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
    void ShowShapeNode(const Handle(AIS_InteractiveContext) & theContext,
                       const Handle(AIS_Shape) & theShape,
                       size_t theIndex);

    void ShowShapeProperties(const Handle(AIS_InteractiveContext) & theContext,
                             const Handle(AIS_Shape) & theShape);

    void ShowShapeVisibility(const Handle(AIS_InteractiveContext) & theContext,
                             const Handle(AIS_Shape) & theShape);

    void ShowDisplayMode(const Handle(AIS_InteractiveContext) & theContext,
                         const Handle(AIS_Shape) & theShape);

    bool myIsVisible;
    std::map<Handle(AIS_Shape), Standard_Integer> myDisplayModes;
};