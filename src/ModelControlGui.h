#pragma once

#include "importers/IShapeImporter.h"
#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <memory>
#include <vector>


class ModelTreeGui;

class ModelControlGui
{
public:
    // 为不同类型的对象定义选择模式
    struct SelectionModes
    {
        int topoShapeMode =
            0;             // 0: Neutral, 1: Vertex, 2: Edge, 3: Wire, 4: Face, 5: Shell, 6: Solid
        int meshMode = 0;  // 0: Neutral
    };

    explicit ModelControlGui(ModelTreeGui& theModelTree);

    void Show(const Handle(AIS_InteractiveContext) & theContext,
              std::vector<Handle(AIS_InteractiveObject)>& objects,
              const Handle(V3d_View) & theView);

    void RegisterImporter(std::shared_ptr<IShapeImporter> importer);

    const SelectionModes& GetSelectionModes() const
    {
        return mySelectionModes;
    }

private:
    void UpdateSelectionMode(const Handle(AIS_InteractiveContext) & theContext,
                             const Handle(AIS_InteractiveObject) & theObject);
    void ShowSelectionInfo(const Handle(AIS_InteractiveContext) & theContext);
    std::string GetShapeTypeString(TopAbs_ShapeEnum theType) const;
    void ImportFile(const Handle(AIS_InteractiveContext) & context,
                    std::vector<Handle(AIS_InteractiveObject)>& objects,
                    const Handle(V3d_View) & view);
    void ShowSelectionControls(const Handle(AIS_InteractiveContext) & theContext,
                               std::vector<Handle(AIS_InteractiveObject)>& objects);

private:
    ModelTreeGui& myModelTree;
    SelectionModes mySelectionModes;
    std::vector<std::shared_ptr<IShapeImporter>> myImporters;
};