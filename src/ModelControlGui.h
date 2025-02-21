#pragma once

#include "importers/IShapeImporter.h"
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <memory>
#include <vector>


class ModelTreeGui;

class ModelControlGui
{
public:
    explicit ModelControlGui(ModelTreeGui& theModelTree);

    void Show(const Handle(AIS_InteractiveContext) & theContext,
              std::vector<Handle(AIS_Shape)>& theShapes,
              const Handle(V3d_View) & theView);

    void RegisterImporter(std::shared_ptr<IShapeImporter> importer);

    // Get current selection mode
    int GetSelectionMode() const
    {
        return myCurrentSelectionMode;
    }

private:
    void UpdateSelectionMode(const Handle(AIS_InteractiveContext) & theContext);
    void ShowSelectionInfo(const Handle(AIS_InteractiveContext) & theContext);
    std::string GetShapeTypeString(TopAbs_ShapeEnum theType) const;
    void ImportFile(const Handle(AIS_InteractiveContext) & context,
                    std::vector<Handle(AIS_Shape)>& shapes,
                    const Handle(V3d_View) & view);

private:
    ModelTreeGui& myModelTree;
    int myCurrentSelectionMode;  // 0: Neutral, 1: Vertex, 2: Edge, 3: Wire, 4: Face, 5: Shell, 6:
                                 // Solid
    std::vector<std::shared_ptr<IShapeImporter>> myImporters;
};