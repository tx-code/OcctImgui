#include "StepImporter.h"

#include <AIS_Shape.hxx>
#include <Message.hxx>
#include <STEPCAFControl_ConfigurationNode.hxx>
#include <TopoDS_Shape.hxx>

bool StepImporter::Import(const char* filePath,
                         const Handle(AIS_InteractiveContext) & context,
                         std::vector<Handle(AIS_InteractiveObject)>& objects)
{
    Handle(DE_Wrapper) aOneTimeSession = DE_Wrapper::GlobalWrapper()->Copy();

    if (!ConfigureSession(aOneTimeSession)) {
        return false;
    }

    TopoDS_Shape aShRes;
    if (!aOneTimeSession->Read(filePath, aShRes)) {
        Message::SendFail() << "Error: Can't read file from " << filePath << "\n";
        return false;
    }

    Handle(AIS_Shape) aShape = new AIS_Shape(aShRes);
    context->Display(aShape, AIS_Shaded, 0, true);
    objects.push_back(aShape);

    // TODO: update view to fit the new shape

    return true;
}

bool StepImporter::ConfigureSession(Handle(DE_Wrapper) & session)
{
    TCollection_AsciiString aString = "global.priority.STEP :   OCC DTK\n"
                                      "global.general.length.unit : 1\n"
                                      "provider.STEP.OCC.read.precision.val : 0.\n";
    if (!session->Load(aString, Standard_True)) {
        Message::SendFail() << "Error: configuration is incorrect";
        return false;
    }

    auto aNode = new STEPCAFControl_ConfigurationNode;
    session->Bind(aNode);
    return true;
}
