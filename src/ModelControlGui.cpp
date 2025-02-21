#include "ModelControlGui.h"
#include "ModelTreeGui.h"
#include "importers/MeshImporter.h"
#include "importers/StepImporter.h"

#include <MeshVS_Mesh.hxx>
#include <MeshVS_SelectionModeFlags.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <V3d_View.hxx>
#include <filesystem>
#include <imgui.h>
#include <nfd.h>


ModelControlGui::ModelControlGui(ModelTreeGui& theModelTree)
    : myModelTree(theModelTree)
{
    // register all importers
    RegisterImporter(std::make_shared<StepImporter>());
    RegisterImporter(std::make_shared<MeshImporter>());
}

void ModelControlGui::Show(const Handle(AIS_InteractiveContext) & context,
                           std::vector<Handle(AIS_InteractiveObject)>& objects,
                           const Handle(V3d_View) & view)
{
    ImGui::Begin("Model Control");
    ImGui::SeparatorText("General");

    if (ImGui::Button("Import Model")) {
        ImportFile(context, objects, view);
    }

    if (ImGui::Button("Toggle Model Tree")) {
        myModelTree.IsVisible() = !myModelTree.IsVisible();
    }

    ImGui::SeparatorText("Selection");
    ShowSelectionControls(context, objects);
    ShowSelectionInfo(context);

    ImGui::End();
}

void ModelControlGui::ShowSelectionControls(const Handle(AIS_InteractiveContext) & context,
                                            std::vector<Handle(AIS_InteractiveObject)>& objects)
{
    bool hasTopoShapes = false;
    bool hasMeshes = false;

    // 检查是否存在不同类型的对象
    for (const auto& obj : objects) {
        if (obj->DynamicType() == STANDARD_TYPE(AIS_Shape)) {
            hasTopoShapes = true;
        }
        else if (obj->DynamicType() == STANDARD_TYPE(MeshVS_Mesh)) {
            hasMeshes = true;
        }
    }

    // TopoShape选择模式
    if (hasTopoShapes) {
        ImGui::Text("CAD Model Selection Mode:");
        static const char* topoItems[] =
            {"Neutral", "Vertex", "Edge", "Wire", "Face", "Shell", "Solid"};
        int previousMode = mySelectionModes.topoShapeMode;

        if (ImGui::Combo("##TopoMode",
                         &mySelectionModes.topoShapeMode,
                         topoItems,
                         IM_ARRAYSIZE(topoItems))) {
            if (previousMode != mySelectionModes.topoShapeMode) {
                for (const auto& obj : objects) {
                    if (obj->DynamicType() == STANDARD_TYPE(AIS_Shape)) {
                        UpdateSelectionMode(context, obj);
                    }
                }
            }
        }
    }

    // Mesh选择模式
    if (hasMeshes) {
        ImGui::Text("Mesh Selection Mode:");
        static const char* meshItems[] = {"Neutral"};
        int previousMode = mySelectionModes.meshMode;

        if (ImGui::Combo("##MeshMode",
                         &mySelectionModes.meshMode,
                         meshItems,
                         IM_ARRAYSIZE(meshItems))) {
            if (previousMode != mySelectionModes.meshMode) {
                for (const auto& obj : objects) {
                    if (obj->DynamicType() == STANDARD_TYPE(MeshVS_Mesh)) {
                        UpdateSelectionMode(context, obj);
                    }
                }
            }
        }
    }
}

void ModelControlGui::UpdateSelectionMode(const Handle(AIS_InteractiveContext) & context,
                                          const Handle(AIS_InteractiveObject) & object)
{
    context->ClearSelected(Standard_False);
    context->Deactivate(object);

    if (object->DynamicType() == STANDARD_TYPE(AIS_Shape)) {
        switch (mySelectionModes.topoShapeMode) {
            case 0:  // Neutral
                context->Activate(object, AIS_Shape::SelectionMode(TopAbs_SHAPE));
                break;
            case 1:  // Vertex
                context->Activate(object, AIS_Shape::SelectionMode(TopAbs_VERTEX));
                break;
            case 2:  // Edge
                context->Activate(object, AIS_Shape::SelectionMode(TopAbs_EDGE));
                break;
            case 3:  // Wire
                context->Activate(object, AIS_Shape::SelectionMode(TopAbs_WIRE));
                break;
            case 4:  // Face
                context->Activate(object, AIS_Shape::SelectionMode(TopAbs_FACE));
                break;
            case 5:  // Shell
                context->Activate(object, AIS_Shape::SelectionMode(TopAbs_SHELL));
                break;
            case 6:  // Solid
                context->Activate(object, AIS_Shape::SelectionMode(TopAbs_SOLID));
                break;
        }
    }
    else if (object->DynamicType() == STANDARD_TYPE(MeshVS_Mesh)) {
        auto mesh = Handle(MeshVS_Mesh)::DownCast(object);
        switch (mySelectionModes.meshMode) {
            case 0:  // Neutral
            default:
                context->Activate(object, MeshVS_SMF_Mesh);
                break;
        }
    }
}

void ModelControlGui::ImportFile(const Handle(AIS_InteractiveContext) & context,
                                 std::vector<Handle(AIS_InteractiveObject)>& objects,
                                 const Handle(V3d_View) & view)
{
    NFD_Init();

    std::vector<nfdu8filteritem_t> filters;
    std::vector<std::string> names, specs;

    names.push_back("All Supported Formats");
    specs.push_back("");
    for (const auto& importer : myImporters) {
        if (!specs.back().empty()) {
            specs.back() += ",";
        }
        specs.back() += importer->GetFileExtensions();
    }

    for (const auto& importer : myImporters) {
        names.push_back(importer->GetImporterName());
        specs.push_back(importer->GetFileExtensions());
    }

    for (size_t i = 0; i < names.size(); ++i) {
        filters.push_back({names[i].c_str(), specs[i].c_str()});
    }

    nfdu8char_t* outPath = nullptr;
    nfdresult_t result = NFD_OpenDialogU8(&outPath, filters.data(), filters.size(), nullptr);

    if (result == NFD_OKAY) {
        std::filesystem::path filePath(outPath);
        std::string ext = filePath.extension().string().substr(1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        for (const auto& importer : myImporters) {
            if (importer->GetFileExtensions().find(ext) != std::string::npos) {
                myModelTree.ClearDisplayModes();
                for (const auto& obj : objects) {
                    context->Remove(obj, true);
                }
                objects.clear();

                importer->Import(outPath, context, objects, view);
                break;
            }
        }
        NFD_FreePathU8(outPath);
    }

    NFD_Quit();
}

std::string ModelControlGui::GetShapeTypeString(TopAbs_ShapeEnum theType) const
{
    switch (theType) {
        case TopAbs_COMPOUND:
            return "Compound";
        case TopAbs_COMPSOLID:
            return "CompSolid";
        case TopAbs_SOLID:
            return "Solid";
        case TopAbs_SHELL:
            return "Shell";
        case TopAbs_FACE:
            return "Face";
        case TopAbs_WIRE:
            return "Wire";
        case TopAbs_EDGE:
            return "Edge";
        case TopAbs_VERTEX:
            return "Vertex";
        case TopAbs_SHAPE:
            return "Shape";
        default:
            return "Unknown";
    }
}

void ModelControlGui::ShowSelectionInfo(const Handle(AIS_InteractiveContext) & theContext)
{
    theContext->InitSelected();
    if (!theContext->MoreSelected()) {
        ImGui::Text("No shape selected");
        return;
    }

    // Show selection info directly in the Model Control window
    for (; theContext->MoreSelected(); theContext->NextSelected()) {
        // Get the root shape
        auto aRootShape = Handle(AIS_Shape)::DownCast(theContext->SelectedInteractive())->Shape();
        auto anOwner = theContext->SelectedOwner();
        if (auto aBrepOwner = Handle(StdSelect_BRepOwner)::DownCast(anOwner)) {
            const TopoDS_Shape& aSubShape = aBrepOwner->Shape();
            if (aSubShape == aRootShape) {
                ImGui::Text("%s", GetShapeTypeString(aSubShape.ShapeType()).c_str());
            }
            else if (aSubShape.ShapeType() == TopAbs_VERTEX) {
                TopTools_IndexedMapOfShape aVertexMap;
                TopExp::MapShapes(aRootShape, TopAbs_VERTEX, aVertexMap);
                ImGui::Text("Vertex %d", aVertexMap.FindIndex(aSubShape));
            }
            else if (aSubShape.ShapeType() == TopAbs_EDGE) {
                TopTools_IndexedMapOfShape aEdgeMap;
                TopExp::MapShapes(aRootShape, TopAbs_EDGE, aEdgeMap);
                ImGui::Text("Edge %d", aEdgeMap.FindIndex(aSubShape));
            }
            else if (aSubShape.ShapeType() == TopAbs_WIRE) {
                TopTools_IndexedMapOfShape aWireMap;
                TopExp::MapShapes(aRootShape, TopAbs_WIRE, aWireMap);
                ImGui::Text("Wire %d", aWireMap.FindIndex(aSubShape));
            }
            else if (aSubShape.ShapeType() == TopAbs_FACE) {
                TopTools_IndexedMapOfShape aFaceMap;
                TopExp::MapShapes(aRootShape, TopAbs_FACE, aFaceMap);
                ImGui::Text("Face %d", aFaceMap.FindIndex(aSubShape));
            }
            else if (aSubShape.ShapeType() == TopAbs_SHELL) {
                TopTools_IndexedMapOfShape aShellMap;
                TopExp::MapShapes(aRootShape, TopAbs_SHELL, aShellMap);
                ImGui::Text("Shell %d", aShellMap.FindIndex(aSubShape));
            }
            else if (aSubShape.ShapeType() == TopAbs_SOLID) {
                TopTools_IndexedMapOfShape aSolidMap;
                TopExp::MapShapes(aRootShape, TopAbs_SOLID, aSolidMap);
                ImGui::Text("Solid %d", aSolidMap.FindIndex(aSubShape));
            }
            else if (aSubShape.ShapeType() == TopAbs_COMPOUND) {
                TopTools_IndexedMapOfShape aCompoundMap;
                TopExp::MapShapes(aRootShape, TopAbs_COMPOUND, aCompoundMap);
                ImGui::Text("Compound %d", aCompoundMap.FindIndex(aSubShape));
            }
            else {
                ImGui::Text("Unknown");
            }
        }
    }
}

void ModelControlGui::RegisterImporter(std::shared_ptr<IShapeImporter> importer)
{
    if (importer) {
        // 检查是否已经注册了相同类型的导入器
        auto it = std::find_if(myImporters.begin(), myImporters.end(), [&](const auto& existing) {
            return existing->GetImporterName() == importer->GetImporterName();
        });

        if (it == myImporters.end()) {
            myImporters.push_back(importer);
        }
    }
}