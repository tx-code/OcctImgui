#include "ModelControlGui.h"
#include "ModelTreeGui.h"

#include <DE_Wrapper.hxx>
#include <Message.hxx>
#include <STEPCAFControl_ConfigurationNode.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <V3d_View.hxx>
#include <imgui.h>
#include <nfd.h>


void ModelControlGui::Show(const Handle(AIS_InteractiveContext) & theContext,
                           std::vector<Handle(AIS_Shape)>& theShapes,
                           const Handle(V3d_View) & theView)
{
    ImGui::Begin("Model Control");

    ImGui::SeparatorText("General");

    if (ImGui::Button("Import STEP Model")) {
        LoadStepFile(theContext, theShapes, theView);
    }

    if (ImGui::Button("Toggle Model Tree")) {
        myModelTree.IsVisible() = !myModelTree.IsVisible();
    }

    ImGui::SeparatorText("Selection");

    // Selection Mode ComboBox
    static const char* items[] =
        {"Neutral", "Vertex", "Edge", "Wire", "Face", "Shell", "Solid"};
    int previousMode = myCurrentSelectionMode;

    if (ImGui::Combo("Selection Mode", &myCurrentSelectionMode, items, IM_ARRAYSIZE(items))) {
        if (previousMode != myCurrentSelectionMode) {
            UpdateSelectionMode(theContext);
        }
    }

    ShowSelectionInfo(theContext);

    ImGui::End();
}

void ModelControlGui::LoadStepFile(const Handle(AIS_InteractiveContext) & theContext,
                                   std::vector<Handle(AIS_Shape)>& theShapes,
                                   const Handle(V3d_View) & theView)
{
    NFD_Init();

    nfdu8char_t* aPath;
    nfdu8filteritem_t filters[] = {{"STEP file", "stp,step"}};
    nfdopendialogu8args_t args = {0};
    args.filterList = filters;
    args.filterCount = 1;
    auto aResult = NFD_OpenDialogU8_With(&aPath, &args);

    if (aResult == NFD_OKAY) {
        // Getting a DE session
        Handle(DE_Wrapper) aOneTimeSession = DE_Wrapper::GlobalWrapper()->Copy();

        // Loading configuration resources
        TCollection_AsciiString aString = "global.priority.STEP :   OCC DTK\n"
                                          "global.general.length.unit : 1\n"
                                          "provider.STEP.OCC.read.precision.val : 0.\n";
        Standard_Boolean aIsRecursive = Standard_True;
        if (!aOneTimeSession->Load(aString, aIsRecursive)) {
            Message::SendFail() << "Error: configuration is incorrect";
            NFD_FreePathU8(aPath);
            NFD_Quit();
            return;
        }

        // Registering providers
        auto aNode = new STEPCAFControl_ConfigurationNode;
        aOneTimeSession->Bind(aNode);

        // Transfer of CAD models
        TopoDS_Shape aShRes;
        if (!aOneTimeSession->Read(aPath, aShRes)) {
            Message::SendFail() << "Error: Can't read file from " << aPath << "\n";
            NFD_FreePathU8(aPath);
            NFD_Quit();
            return;
        }

        // Clear existing shapes
        myModelTree.ClearDisplayModes();
        for (const auto& shape : theShapes) {
            theContext->Remove(shape, true);
        }
        theShapes.clear();

        // Display the new shape
        Handle(AIS_Shape) aShape = new AIS_Shape(aShRes);
        theContext->Display(aShape, AIS_Shaded, 0, true);
        theShapes.push_back(aShape);

        // Fit view
        theView->FitAll();
        theView->ZFitAll();
        theView->Redraw();

        NFD_FreePathU8(aPath);
    }
    else if (aResult != NFD_CANCEL) {
        Message::DefaultMessenger()->Send(TCollection_AsciiString("Error: ") + NFD_GetError(),
                                          Message_Fail);
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