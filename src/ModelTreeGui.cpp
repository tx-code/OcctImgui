#include "ModelTreeGui.h"
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <Quantity_Color.hxx>
#include <TopExp_Explorer.hxx>
#include <imgui.h>


void ModelTreeGui::Show(const Handle(AIS_InteractiveContext) & theContext,
                        std::vector<Handle(AIS_Shape)>& theShapes)
{
    if (!myIsVisible) {
        return;
    }

    ImGui::Begin("Model Tree", &myIsVisible);

    if (ImGui::TreeNode("Shapes in Scene")) {
        for (size_t i = 0; i < theShapes.size(); i++) {
            ShowShapeNode(theContext, theShapes[i], i);
        }
        ImGui::TreePop();
    }

    ImGui::End();
}

void ModelTreeGui::ShowShapeNode(const Handle(AIS_InteractiveContext) & theContext,
                                 const Handle(AIS_Shape) & theShape,
                                 size_t theIndex)
{
    // 创建唯一的标签
    TCollection_AsciiString label;
    label += "Shape ";
    label += static_cast<Standard_Integer>(theIndex);

    // 获取形状类型
    TopAbs_ShapeEnum shapeType = theShape->Shape().ShapeType();
    switch (shapeType) {
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

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;

    // 如果形状被选中，添加选中标记
    if (theContext->IsSelected(theShape)) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    if (ImGui::TreeNodeEx(label.ToCString(), flags)) {
        // 处理选择
        if (ImGui::IsItemClicked()) {
            if (ImGui::GetIO().KeyCtrl) {
                // Ctrl+点击 多选
                theContext->AddOrRemoveSelected(theShape, true);
            }
            else {
                // 单击 单选
                theContext->ClearSelected(false);
                theContext->AddOrRemoveSelected(theShape, true);
            }
        }

        // 显示形状属性
        ShowShapeProperties(theContext, theShape);

        // 显示可见性控制
        ShowShapeVisibility(theContext, theShape);

        // 显示模式控制
        ShowDisplayMode(theContext, theShape);

        ImGui::TreePop();
    }
}

void ModelTreeGui::ShowShapeProperties(const Handle(AIS_InteractiveContext) & theContext,
                                       const Handle(AIS_Shape) & theShape)
{
    // 获取形状的拓扑信息
    TopExp_Explorer faceExp(theShape->Shape(), TopAbs_FACE);
    TopExp_Explorer edgeExp(theShape->Shape(), TopAbs_EDGE);
    TopExp_Explorer vertExp(theShape->Shape(), TopAbs_VERTEX);
    int nFaces = 0, nEdges = 0, nVertices = 0;
    for (; faceExp.More(); faceExp.Next())
        nFaces++;
    for (; edgeExp.More(); edgeExp.Next())
        nEdges++;
    for (; vertExp.More(); vertExp.Next())
        nVertices++;

    // 获取包围盒
    Bnd_Box bbox;
    BRepBndLib::Add(theShape->Shape(), bbox);
    Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
    bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    if (ImGui::TreeNode("Properties")) {
        ImGui::Text("Topology:");
        ImGui::Text("  Faces: %d", nFaces);
        ImGui::Text("  Edges: %d", nEdges);
        ImGui::Text("  Vertices: %d", nVertices);

        ImGui::Text("Dimensions:");
        ImGui::Text("  Size: %.2f x %.2f x %.2f", xmax - xmin, ymax - ymin, zmax - zmin);
        ImGui::Text("  Center: (%.2f, %.2f, %.2f)",
                    (xmax + xmin) / 2,
                    (ymax + ymin) / 2,
                    (zmax + zmin) / 2);
        ImGui::TreePop();
    }
}

void ModelTreeGui::ShowShapeVisibility(const Handle(AIS_InteractiveContext) & theContext,
                                       const Handle(AIS_Shape) & theShape)
{
    bool isVisible = theContext->IsDisplayed(theShape);

    if (ImGui::Checkbox("Visible", &isVisible)) {
        if (isVisible) {
            // 使用保存的显示模式显示形状
            theContext->Display(theShape, myDisplayModes[theShape], 0, true);
        }
        else {
            theContext->Erase(theShape, true);
        }
    }
}

void ModelTreeGui::ShowDisplayMode(const Handle(AIS_InteractiveContext) & theContext,
                                   const Handle(AIS_Shape) & theShape)
{
    const char* modes[] = {"Wireframe", "Shaded"};

    // 如果是新形状，设置默认显示模式为 Shaded
    if (myDisplayModes.find(theShape) == myDisplayModes.end()) {
        myDisplayModes[theShape] = AIS_Shaded;
    }

    int currentMode = myDisplayModes[theShape] == AIS_WireFrame ? 0 : 1;

    // 显示模式下拉菜单
    if (ImGui::Combo("Display Mode", &currentMode, modes, IM_ARRAYSIZE(modes))) {
        Standard_Integer newMode = (currentMode == 0) ? AIS_WireFrame : AIS_Shaded;
        myDisplayModes[theShape] = newMode;

        if (theContext->IsDisplayed(theShape)) {
            theContext->SetDisplayMode(theShape, newMode, Standard_True);
        }
    }
}