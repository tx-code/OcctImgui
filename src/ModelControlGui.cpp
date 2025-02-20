#include "ModelControlGui.h"
#include "ModelTreeGui.h"

#include <DE_Wrapper.hxx>
#include <Message.hxx>
#include <STEPCAFControl_ConfigurationNode.hxx>
#include <V3d_View.hxx>
#include <imgui.h>
#include <nfd.h>

void ModelControlGui::Show(const Handle(AIS_InteractiveContext) & theContext,
                           std::vector<Handle(AIS_Shape)>& theShapes,
                           const Handle(V3d_View) & theView)
{
    ImGui::Begin("Model Control");

    if (ImGui::Button("Import STEP Model")) {
        LoadStepFile(theContext, theShapes, theView);
    }

    if (ImGui::Button("Toggle Model Tree")) {
        myModelTree.IsVisible() = !myModelTree.IsVisible();
    }

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
