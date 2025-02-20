#ifndef _ModelTreeGui_Header
#define _ModelTreeGui_Header

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
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

private:
    void ShowShapeNode(const Handle(AIS_InteractiveContext) & theContext,
                       const Handle(AIS_Shape) & theShape,
                       size_t theIndex);

    bool myIsVisible;
};

#endif