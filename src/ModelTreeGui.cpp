#include "ModelTreeGui.h"
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <MeshVS_DataSource.hxx>
#include <MeshVS_Mesh.hxx>
#include <TopExp_Explorer.hxx>
#include <imgui.h>


void ModelTreeGui::Show(const Handle(AIS_InteractiveContext) & theContext,
                        std::vector<Handle(AIS_InteractiveObject)>& theObjects)
{
    if (!myIsVisible) {
        return;
    }

    ImGui::Begin("Model Tree", &myIsVisible);

    // CAD Models tree node
    if (ImGui::TreeNode("TopoShapes")) {
        for (size_t i = 0; i < theObjects.size(); i++) {
            if (theObjects[i]->DynamicType() == STANDARD_TYPE(AIS_Shape)) {
                ShowObjectNode(theContext, theObjects[i], i);
            }
        }
        ImGui::TreePop();
    }

    // Meshes tree node
    if (ImGui::TreeNode("Meshes")) {
        for (size_t i = 0; i < theObjects.size(); i++) {
            if (theObjects[i]->DynamicType() == STANDARD_TYPE(MeshVS_Mesh)) {
                ShowObjectNode(theContext, theObjects[i], i);
            }
        }
        ImGui::TreePop();
    }

    ImGui::End();
}

void ModelTreeGui::ShowObjectNode(const Handle(AIS_InteractiveContext) & theContext,
                                  const Handle(AIS_InteractiveObject) & theObject,
                                  size_t theIndex)
{
    TCollection_AsciiString label;
    label += "Object ";
    label += static_cast<Standard_Integer>(theIndex);

    // 根据对象类型添加不同的标签
    if (theObject->DynamicType() == STANDARD_TYPE(AIS_Shape)) {
        auto shape = Handle(AIS_Shape)::DownCast(theObject);
        switch (shape->Shape().ShapeType()) {
            case TopAbs_COMPOUND:
                label += " (Compound)";
                break;
            case TopAbs_COMPSOLID:
                label += " (CompSolid)";
                break;
            case TopAbs_SOLID:
                label += " (Solid)";
                break;
            case TopAbs_SHELL:
                label += " (Shell)";
                break;
            case TopAbs_FACE:
                label += " (Face)";
                break;
            case TopAbs_WIRE:
                label += " (Wire)";
                break;
            case TopAbs_EDGE:
                label += " (Edge)";
                break;
            case TopAbs_VERTEX:
                label += " (Vertex)";
                break;
            default:
                label += " (Other)";
                break;
        }
    }
    else if (theObject->DynamicType() == STANDARD_TYPE(MeshVS_Mesh)) {
        label += " (Mesh)";
    }

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
    if (theContext->IsSelected(theObject)) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    if (ImGui::TreeNodeEx(label.ToCString(), flags)) {
        if (ImGui::IsItemClicked()) {
            if (ImGui::GetIO().KeyCtrl) {
                theContext->AddOrRemoveSelected(theObject, true);
            }
            else {
                theContext->ClearSelected(false);
                theContext->AddOrRemoveSelected(theObject, true);
            }
        }

        ShowObjectProperties(theContext, theObject);
        ShowObjectVisibility(theContext, theObject);
        ShowDisplayMode(theContext, theObject);

        ImGui::TreePop();
    }
}

void ModelTreeGui::ShowObjectProperties(const Handle(AIS_InteractiveContext) & theContext,
                                        const Handle(AIS_InteractiveObject) & theObject)
{
    if (!ImGui::TreeNode("Properties")) {
        return;
    }

    if (theObject->DynamicType() == STANDARD_TYPE(AIS_Shape)) {
        auto shape = Handle(AIS_Shape)::DownCast(theObject);
        // 显示形状特有的属性
        TopExp_Explorer faceExp(shape->Shape(), TopAbs_FACE);
        TopExp_Explorer edgeExp(shape->Shape(), TopAbs_EDGE);
        TopExp_Explorer vertExp(shape->Shape(), TopAbs_VERTEX);
        int nFaces = 0, nEdges = 0, nVertices = 0;
        for (; faceExp.More(); faceExp.Next())
            nFaces++;
        for (; edgeExp.More(); edgeExp.Next())
            nEdges++;
        for (; vertExp.More(); vertExp.Next())
            nVertices++;

        ImGui::Text("Topology:");
        ImGui::Text("  Faces: %d", nFaces);
        ImGui::Text("  Edges: %d", nEdges);
        ImGui::Text("  Vertices: %d", nVertices);

        Bnd_Box bbox;
        BRepBndLib::Add(shape->Shape(), bbox);
        Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
        bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

        ImGui::Text("Dimensions:");
        ImGui::Text("  Size: %.2f x %.2f x %.2f", xmax - xmin, ymax - ymin, zmax - zmin);
        ImGui::Text("  Center: (%.2f, %.2f, %.2f)",
                    (xmax + xmin) / 2,
                    (ymax + ymin) / 2,
                    (zmax + zmin) / 2);
    }
    else if (theObject->DynamicType() == STANDARD_TYPE(MeshVS_Mesh)) {
        auto mesh = Handle(MeshVS_Mesh)::DownCast(theObject);
        auto dataSource = Handle(MeshVS_DataSource)::DownCast(mesh->GetDataSource());

        ImGui::Text("Mesh Statistics:");
        ImGui::Text("  Nodes: %d", dataSource->GetAllNodes().Extent());
        ImGui::Text("  Elements: %d", dataSource->GetAllElements().Extent());

        // 获取包围盒
        Bnd_Box bbox;
        mesh->BoundingBox(bbox);
        Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
        bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

        ImGui::Text("Dimensions:");
        ImGui::Text("  Size: %.2f x %.2f x %.2f", xmax - xmin, ymax - ymin, zmax - zmin);
        ImGui::Text("  Center: (%.2f, %.2f, %.2f)",
                    (xmax + xmin) / 2,
                    (ymax + ymin) / 2,
                    (zmax + zmin) / 2);
    }

    ImGui::TreePop();
}

void ModelTreeGui::ShowObjectVisibility(const Handle(AIS_InteractiveContext) & theContext,
                                        const Handle(AIS_InteractiveObject) & theObject)
{
    bool isVisible = theContext->IsDisplayed(theObject);

    if (ImGui::Checkbox("Visible", &isVisible)) {
        if (isVisible) {
            // 使用保存的显示模式显示对象
            theContext->Display(theObject, myDisplayModes[theObject], 0, true);
        }
        else {
            theContext->Erase(theObject, true);
        }
    }
}

void ModelTreeGui::ShowDisplayMode(const Handle(AIS_InteractiveContext) & theContext,
                                   const Handle(AIS_InteractiveObject) & theObject)
{
    if (theObject->DynamicType() == STANDARD_TYPE(AIS_Shape)) {
        const char* modes[] = {"Wireframe", "Shaded"};
        if (myDisplayModes.find(theObject) == myDisplayModes.end()) {
            myDisplayModes[theObject] = AIS_Shaded;
        }

        int currentMode = myDisplayModes[theObject] == AIS_WireFrame ? 0 : 1;
        if (ImGui::Combo("Display Mode", &currentMode, modes, IM_ARRAYSIZE(modes))) {
            Standard_Integer newMode = (currentMode == 0) ? AIS_WireFrame : AIS_Shaded;
            myDisplayModes[theObject] = newMode;
            if (theContext->IsDisplayed(theObject)) {
                theContext->SetDisplayMode(theObject, newMode, Standard_True);
            }
        }
    }
    else if (theObject->DynamicType() == STANDARD_TYPE(MeshVS_Mesh)) {
        const char* modes[] = {"Wireframe", "Shaded"};
        if (myDisplayModes.find(theObject) == myDisplayModes.end()) {
            myDisplayModes[theObject] = MeshVS_DMF_WireFrame | MeshVS_DMF_Shading;
        }

        int currentMode;
        if (myDisplayModes[theObject] == MeshVS_DMF_WireFrame)
            currentMode = 0;
        else if (myDisplayModes[theObject] == MeshVS_DMF_Shading)
            currentMode = 1;

        if (ImGui::Combo("Display Mode", &currentMode, modes, IM_ARRAYSIZE(modes))) {
            Standard_Integer newMode;
            switch (currentMode) {
                case 0:
                    newMode = MeshVS_DMF_WireFrame;
                    break;
                case 1:
                default:
                    newMode = MeshVS_DMF_Shading;
                    break;
            }
            myDisplayModes[theObject] = newMode;
            if (theContext->IsDisplayed(theObject)) {
                theContext->SetDisplayMode(theObject, newMode, Standard_True);
            }
        }
    }
}