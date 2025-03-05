#include "UnifiedViewModel.h"
#include "ais/Mesh_DataSource.h"
#include "../utils/Logger.h"
#include <AIS_Shape.hxx>
#include <AIS_Triangulation.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <MeshVS_Drawer.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_Mesh.hxx>
#include <MeshVS_MeshPrsBuilder.hxx>
#include <MeshVS_DisplayModeFlags.hxx>
#include <TColStd_HPackedMapOfInteger.hxx>
#include <Precision.hxx>
#include <TopoDS_Builder.hxx>
#include <algorithm>
#include <random>
#include <iostream>

// 创建ViewModel日志记录器
static std::shared_ptr<Utils::Logger>& getViewModelLogger() {
    static std::shared_ptr<Utils::Logger> logger = Utils::Logger::getLogger("viewmodel");
    return logger;
}

// Constructor
UnifiedViewModel::UnifiedViewModel(std::shared_ptr<UnifiedModel> model,
                                   Handle(AIS_InteractiveContext) context,
                                   MVVM::GlobalSettings& globalSettings,
                                   std::shared_ptr<ModelImporter> modelImporter)
    : myModel(model)
    , myContext(context)
    , myGlobalSettings(globalSettings)
    , myModelImporter(modelImporter)
{

    // Register model change listener
    model->addChangeListener([this](const std::string& id) {
        this->onModelChanged(id);
    });

    // Initialize display of existing geometries
    for (const std::string& id : model->getAllEntityIds()) {
        updatePresentation(id);
    }

    // Bind display mode property to global settings
    // Create a scoped connection and track it
    auto connection = displayMode.bindTo(globalSettings.displayMode);
    connections.track(connection);

    // Initialize selection properties
    updateSelectionProperties();
}

// Command - CAD geometry operations
void UnifiedViewModel::createBox(const gp_Pnt& location, double sizeX, double sizeY, double sizeZ)
{
    // Generate unique ID
    static int boxCounter = 0;
    std::string id = "box_" + std::to_string(++boxCounter);

    // Create box geometry
    BRepPrimAPI_MakeBox boxMaker(location, sizeX, sizeY, sizeZ);
    TopoDS_Shape boxShape = boxMaker.Shape();

    // Add to model
    myModel->addShape(id, boxShape);
}

void UnifiedViewModel::createCone(const gp_Pnt& location, double radius, double height)
{
    // Generate unique ID
    static int coneCounter = 0;
    std::string id = "cone_" + std::to_string(++coneCounter);

    // Create cone geometry
    gp_Ax2 axis(location, gp_Dir(0, 0, 1));
    BRepPrimAPI_MakeCone coneMaker(axis, radius, 0, height);
    TopoDS_Shape coneShape = coneMaker.Shape();

    // Add to model
    myModel->addShape(id, coneShape);
}

void UnifiedViewModel::createMesh(/* Mesh creation parameters */)
{
    // This method needs to be implemented based on the actual mesh creation requirements
    // Below is example code, should be modified based on actual situation

    /*
    // Generate unique ID
    static int meshCounter = 0;
    std::string id = "mesh_" + std::to_string(++meshCounter);

    // Create triangle mesh (example)
    Handle(Poly_Triangulation) mesh = new Poly_Triangulation(numVertices, numTriangles, false);

    // Set vertices and triangles
    // ... fill mesh data ...

    // Add to model
    myModel->addMesh(id, mesh);
    */
}

bool UnifiedViewModel::importModel(const std::string& filePath, const std::string& modelId)
{
    LOG_FUNCTION_SCOPE(getViewModelLogger(), "importModel");
    getViewModelLogger()->info("Importing model from '{}'", filePath);
    
    if (!myModelImporter) {
        getViewModelLogger()->error("ModelImporter is not available");
        return false;
    }
    
    // 使用注入的 ModelImporter 导入模型
    bool result = myModelImporter->importModel(filePath, *myModel, modelId);
    
    if (result) {
        getViewModelLogger()->info("Model imported successfully");
    } else {
        getViewModelLogger()->error("Failed to import model");
    }
    
    return result;
}

// IViewModel interface implementation
void UnifiedViewModel::deleteSelectedObjects()
{
    std::vector<std::string> objectsToDelete(mySelectedObjects.begin(), mySelectedObjects.end());

    for (const std::string& id : objectsToDelete) {
        myModel->removeEntity(id);
    }

    mySelectedObjects.clear();
}

bool UnifiedViewModel::hasSelection() const
{
    return !mySelectedObjects.empty();
}

std::vector<std::string> UnifiedViewModel::getSelectedObjects() const
{
    return std::vector<std::string>(mySelectedObjects.begin(), mySelectedObjects.end());
}

void UnifiedViewModel::processSelection(const Handle(AIS_InteractiveObject) & obj, bool isSelected)
{
    auto it = myObjectToIdMap.find(obj);
    if (it != myObjectToIdMap.end()) {
        if (isSelected) {
            mySelectedObjects.insert(it->second);
        }
        else {
            mySelectedObjects.erase(it->second);
        }

        // Update selection properties
        updateSelectionProperties();
    }
}

void UnifiedViewModel::clearSelection()
{
    mySelectedObjects.clear();
    myContext->ClearSelected(Standard_True);

    // Update selection properties
    updateSelectionProperties();
}

// New method to update selection properties
void UnifiedViewModel::updateSelectionProperties()
{
    hasSelectionProperty.set(!mySelectedObjects.empty());
    selectionCountProperty.set(static_cast<int>(mySelectedObjects.size()));
}

// Attribute access and modification
void UnifiedViewModel::setSelectedColor(const Quantity_Color& color)
{
    for (const std::string& id : mySelectedObjects) {
        myModel->setColor(id, color);
    }
}

Quantity_Color UnifiedViewModel::getSelectedColor() const
{
    if (mySelectedObjects.empty()) {
        return Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB);  // Default gray
    }

    // Return color of the first selected object
    return myModel->getColor(*mySelectedObjects.begin());
}

// Private methods
void UnifiedViewModel::updatePresentation(const std::string& id)
{
    // Delete existing representation
    auto it = myIdToObjectMap.find(id);
    if (it != myIdToObjectMap.end()) {
        myContext->Remove(it->second, false);
        myObjectToIdMap.erase(it->second);
        myIdToObjectMap.erase(it);
    }

    // Get geometry data
    const UnifiedModel::GeometryData* data = myModel->getGeometryData(id);
    if (!data) {
        return;
    }

    // Create new representation
    Handle(AIS_InteractiveObject) aisObj = createPresentationForGeometry(id, data);
    if (aisObj.IsNull()) {
        return;
    }

    // Display object
    myContext->Display(aisObj, false);

    // Update mapping
    myIdToObjectMap[id] = aisObj;
    myObjectToIdMap[aisObj] = id;
}

Handle(AIS_InteractiveObject)
    UnifiedViewModel::createPresentationForGeometry(const std::string& id,
                                                    const UnifiedModel::GeometryData* data)
{

    if (!data) {
        return nullptr;
    }

    Handle(AIS_InteractiveObject) aisObj;

    // Create appropriate AIS object based on geometry type
    if (data->type == UnifiedModel::GeometryType::SHAPE) {
        // Create AIS_Shape for CAD shape
        const TopoDS_Shape& shape = std::get<TopoDS_Shape>(data->geometry);
        Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
        aisShape->SetColor(data->color);

        // Set display mode based on displayMode
        switch (displayMode.get()) {
            case 0:  // Shaded
                aisShape->SetDisplayMode(AIS_Shaded);
                break;
            case 1:  // Wireframe
                aisShape->SetDisplayMode(AIS_WireFrame);
                break;
                // Can add more display modes
        }

        aisObj = aisShape;
    }
    else if (data->type == UnifiedModel::GeometryType::MESH) {
        // Get the mesh data from the geometry
        const UnifiedModel::MeshData& meshData = std::get<UnifiedModel::MeshData>(data->geometry);
        Handle(Mesh_DataSource) meshDataSource = new Mesh_DataSource(meshData.vertices,
                                                                     meshData.faces,
                                                                     meshData.normals);

        Handle(MeshVS_Mesh) meshObj = new MeshVS_Mesh;
        meshObj->SetDataSource(meshDataSource);

        Handle(MeshVS_MeshPrsBuilder) mainBuilder =
            new MeshVS_MeshPrsBuilder(meshObj, MeshVS_DMF_WireFrame | MeshVS_DMF_Shading);
        meshObj->AddBuilder(mainBuilder, true);
        meshObj->GetDrawer()->SetColor(MeshVS_DA_EdgeColor, data->color);

        // Hide all nodes by default
        Handle(TColStd_HPackedMapOfInteger) aNodes = new TColStd_HPackedMapOfInteger(meshDataSource->GetAllNodes());
        meshObj->SetHiddenNodes(aNodes);

        meshObj->SetDisplayMode(MeshVS_DMF_Shading);

        aisObj = meshObj;
    }

    return aisObj;
}

void UnifiedViewModel::onModelChanged(const std::string& id)
{
    updatePresentation(id);
}