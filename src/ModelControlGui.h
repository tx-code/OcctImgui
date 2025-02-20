#ifndef _ModelControlGui_Header
#define _ModelControlGui_Header

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <vector>


class ModelTreeGui;

class ModelControlGui
{
public:
    explicit ModelControlGui(ModelTreeGui& theModelTree)
        : myModelTree(theModelTree)
        , myCurrentSelectionMode(0)  // Default to neutral selection mode
    {}

    void Show(const Handle(AIS_InteractiveContext) & theContext,
              std::vector<Handle(AIS_Shape)>& theShapes,
              const Handle(V3d_View) & theView);

    void LoadStepFile(const Handle(AIS_InteractiveContext) & theContext,
                      std::vector<Handle(AIS_Shape)>& theShapes,
                      const Handle(V3d_View) & theView);

    // Get current selection mode
    int GetSelectionMode() const
    {
        return myCurrentSelectionMode;
    }

private:
    void UpdateSelectionMode(const Handle(AIS_InteractiveContext) & theContext);
    void ShowSelectionInfo(const Handle(AIS_InteractiveContext) & theContext);
    std::string GetShapeTypeString(TopAbs_ShapeEnum theType) const;

private:
    ModelTreeGui& myModelTree;
    int myCurrentSelectionMode;  // 0: Neutral, 1: Vertex, 2: Edge, 3: Wire, 4: Face, 5: Shell, 6:
                                 // Solid
};

#endif