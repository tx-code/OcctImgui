#include "MeshImporter.h"
#include "ais/OCCMesh_DataSource.h"

#include <BRep_Tool.hxx>
#include <MeshVS_DataSource.hxx>
#include <MeshVS_Drawer.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_Mesh.hxx>
#include <MeshVS_MeshPrsBuilder.hxx>
#include <Message.hxx>
#include <Poly_Triangulation.hxx>
#include <RWObj.hxx>
#include <RWStl.hxx>
#include <TColStd_HPackedMapOfInteger.hxx>
#include <filesystem>

bool MeshImporter::Import(const char* filePath,
                          const Handle(AIS_InteractiveContext) & context,
                          std::vector<Handle(AIS_InteractiveObject)>& objects,
                          const Handle(V3d_View) & view)
{
    std::string ext = std::filesystem::path(filePath).extension().string().substr(1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    Handle(Poly_Triangulation) aMesh;
    if (ext == "stl") {
        aMesh = RWStl::ReadFile(filePath);
    }
    else if (ext == "obj") {
        aMesh = RWObj::ReadFile(filePath);
    }

    if (aMesh.IsNull()) {
        Message::SendFail() << "Error: Can't read file from " << filePath << "\n";
        return false;
    }

    Handle(OCCMesh_DataSource) aDataSource = new OCCMesh_DataSource(aMesh);
    Handle(MeshVS_Mesh) aMeshPrs = new MeshVS_Mesh();
    aMeshPrs->SetDataSource(aDataSource);

    Handle(MeshVS_MeshPrsBuilder) mainBuilder =
        new MeshVS_MeshPrsBuilder(aMeshPrs, MeshVS_DMF_WireFrame | MeshVS_DMF_Shading);
    aMeshPrs->AddBuilder(mainBuilder, true);
    aMeshPrs->GetDrawer()->SetColor(MeshVS_DA_EdgeColor, Quantity_NOC_YELLOW);

    // Hide all nodes by default
    Handle(TColStd_HPackedMapOfInteger) aNodes = new TColStd_HPackedMapOfInteger();
    Standard_Integer aLen = aDataSource->GetAllNodes().Extent();
    for (Standard_Integer anIndex = 1; anIndex <= aLen; anIndex++) {
        aNodes->ChangeMap().Add(anIndex);
    }
    aMeshPrs->SetHiddenNodes(aNodes);

    context->Display(aMeshPrs, AIS_Shaded, 0, true);
    objects.push_back(aMeshPrs);

    view->FitAll();
    view->ZFitAll();
    view->Redraw();

    return true;
}