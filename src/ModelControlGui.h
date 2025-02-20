#ifndef _ModelControlGui_Header
#define _ModelControlGui_Header

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <vector>

class ModelTreeGui;

class ModelControlGui
{
public:
    ModelControlGui(ModelTreeGui& theModelTree)
        : myModelTree(theModelTree)
    {}

    void Show(const Handle(AIS_InteractiveContext) & theContext,
              std::vector<Handle(AIS_Shape)>& theShapes,
              const Handle(V3d_View) & theView);

    void LoadStepFile(const Handle(AIS_InteractiveContext) & theContext,
                      std::vector<Handle(AIS_Shape)>& theShapes,
                      const Handle(V3d_View) & theView);

private:
    ModelTreeGui& myModelTree;
};

#endif