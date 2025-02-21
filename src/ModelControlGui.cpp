#include "ModelControlGui.h"
#include "ModelTreeGui.h"
#include "importers/MeshImporter.h"
#include "importers/StepImporter.h"

#include <StdSelect_BRepOwner.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <V3d_View.hxx>
#include <filesystem>
#include <imgui.h>
#include <nfd.h>


ModelControlGui::ModelControlGui(ModelTreeGui& theModelTree)
    : myModelTree(theModelTree)
    , myCurrentSelectionMode(0)
{
    // register all importers
    RegisterImporter(std::make_shared<StepImporter>());
    RegisterImporter(std::make_shared<MeshImporter>());
}

void ModelControlGui::Show(const Handle(AIS_InteractiveContext) & context,
                           std::vector<Handle(AIS_Shape)>& shapes,
                           const Handle(V3d_View) & view)
{
    ImGui::Begin("Model Control");
    ImGui::SeparatorText("General");

    if (ImGui::Button("Import Model")) {
        ImportFile(context, shapes, view);
    }

    if (ImGui::Button("Toggle Model Tree")) {
        myModelTree.IsVisible() = !myModelTree.IsVisible();
    }

    ImGui::SeparatorText("Selection");

    // Selection Mode ComboBox
    static const char* items[] = {"Neutral", "Vertex", "Edge", "Wire", "Face", "Shell", "Solid"};
    int previousMode = myCurrentSelectionMode;

    if (ImGui::Combo("Selection Mode", &myCurrentSelectionMode, items, IM_ARRAYSIZE(items))) {
        if (previousMode != myCurrentSelectionMode) {
            UpdateSelectionMode(context);
        }
    }

    ShowSelectionInfo(context);

    ImGui::End();
}

void ModelControlGui::ImportFile(const Handle(AIS_InteractiveContext) & context,
                                 std::vector<Handle(AIS_Shape)>& shapes,
                                 const Handle(V3d_View) & view)
{
    NFD_Init();

    // 构建文件过滤器
    std::vector<nfdu8filteritem_t> filters;
    // 保存字符串以确保其生命周期
    std::vector<std::string> names, specs;

    // 首先添加所有格式的过滤器
    names.push_back("All Supported Formats");
    specs.push_back("");
    for (const auto& importer : myImporters) {
        if (!specs.back().empty()) {
            specs.back() += ",";
        }
        specs.back() += importer->GetFileExtensions();
    }

    // 然后为每种格式添加单独的过滤器
    for (const auto& importer : myImporters) {
        names.push_back(importer->GetImporterName());
        specs.push_back(importer->GetFileExtensions());
    }

    // 构建过滤器列表
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
                // 清理现有形状
                myModelTree.ClearDisplayModes();
                for (const auto& shape : shapes) {
                    context->Remove(shape, true);
                }
                shapes.clear();

                // 导入新形状
                importer->Import(outPath, context, shapes, view);
                break;
            }
        }

        NFD_FreePathU8(outPath);
    }

    NFD_Quit();
}

void ModelControlGui::UpdateSelectionMode(const Handle(AIS_InteractiveContext) & theContext)
{
    // Clear previous selection
    theContext->ClearSelected(Standard_False);

    // Deactivate previous selection mode
    theContext->Deactivate();

    // Activate new selection mode
    switch (myCurrentSelectionMode) {
        case 0:  // Neutral - shape selection
            theContext->Activate(AIS_Shape::SelectionMode(TopAbs_SHAPE));
            break;
        case 1:  // Vertex
            theContext->Activate(AIS_Shape::SelectionMode(TopAbs_VERTEX));
            break;
        case 2:  // Edge
            theContext->Activate(AIS_Shape::SelectionMode(TopAbs_EDGE));
            break;
        case 3:  // Wire
            theContext->Activate(AIS_Shape::SelectionMode(TopAbs_WIRE));
            break;
        case 4:  // Face
            theContext->Activate(AIS_Shape::SelectionMode(TopAbs_FACE));
            break;
        case 5:  // Shell
            theContext->Activate(AIS_Shape::SelectionMode(TopAbs_SHELL));
            break;
        case 6:  // Solid
            theContext->Activate(AIS_Shape::SelectionMode(TopAbs_SOLID));
            break;
    }
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