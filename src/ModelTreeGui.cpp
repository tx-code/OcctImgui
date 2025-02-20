#include "ModelTreeGui.h"
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
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

    if (ImGui::TreeNode(label.ToCString())) {
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

        // 显示形状信息
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

        // 控制按钮
        if (ImGui::Button("Hide/Show")) {
            if (theContext->IsDisplayed(theShape)) {
                theContext->Erase(theShape, Standard_True);
            }
            else {
                theContext->Display(theShape, AIS_Shaded, 0, true);
            }
        }

        ImGui::TreePop();
    }
}