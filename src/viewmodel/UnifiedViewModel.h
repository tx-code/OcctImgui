/**
 * @file UnifiedViewModel.h
 * @brief Defines the UnifiedViewModel class which connects the UnifiedModel with the view layer.
 * 
 * The UnifiedViewModel provides a unified interface for interacting with the UnifiedModel,
 * handling the presentation of model data in the view, and processing user interactions.
 */
#pragma once

#include "IViewModel.h"
#include "../model/UnifiedModel.h"
#include "../model/ModelImporter.h"
#include "../mvvm/Property.h"
#include "../mvvm/GlobalSettings.h"

#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <AIS_Shape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <memory>
#include <set>
#include <string>
#include <map>

/**
 * @class UnifiedViewModel
 * @brief ViewModel that connects the UnifiedModel with the view layer.
 * 
 * This class implements the IViewModel interface and provides methods for creating,
 * manipulating, and visualizing geometric objects. It maintains the connection between
 * the model data and its visual representation in the OCCT context.
 */
class UnifiedViewModel : public IViewModel {
public:
    /**
     * @brief Constructor
     * @param model The UnifiedModel to connect with
     * @param context The OCCT interactive context for visualization
     * @param globalSettings Reference to the application's global settings
     * @param modelImporter The ModelImporter to use for importing models
     */
    UnifiedViewModel(std::shared_ptr<UnifiedModel> model, 
                    Handle(AIS_InteractiveContext) context,
                    MVVM::GlobalSettings& globalSettings,
                    std::shared_ptr<ModelImporter> modelImporter);
    
    /**
     * @brief Virtual destructor
     */
    ~UnifiedViewModel() override = default;
    
    /**
     * @brief Creates a box shape at the specified location with the given dimensions
     * @param location The location of the box
     * @param sizeX The width of the box
     * @param sizeY The depth of the box
     * @param sizeZ The height of the box
     */
    void createBox(const gp_Pnt& location, double sizeX, double sizeY, double sizeZ);
    
    /**
     * @brief Creates a cone shape at the specified location with the given dimensions
     * @param location The location of the cone
     * @param radius The radius of the cone base
     * @param height The height of the cone
     */
    void createCone(const gp_Pnt& location, double radius, double height);
    
    /**
     * @brief Creates a mesh (placeholder for future implementation)
     */
    void createMesh(/* Mesh creation parameters */); // Specific parameters to be defined based on requirements
    
    /**
     * @brief Imports a model from a file
     * @param filePath The path to the model file
     * @param modelId The ID to assign to the imported model (if empty, the filename will be used)
     * @return True if import was successful, false otherwise
     */
    bool importModel(const std::string& filePath, const std::string& modelId = "");
    
    /**
     * @brief Deletes the currently selected objects
     */
    void deleteSelectedObjects() override;
    
    /**
     * @brief Checks if there are any selected objects
     * @return True if there are selected objects, false otherwise
     */
    bool hasSelection() const override;
    
    /**
     * @brief Gets the IDs of the currently selected objects
     * @return Vector of selected object IDs
     */
    std::vector<std::string> getSelectedObjects() const override;
    
    /**
     * @brief Processes selection/deselection of an interactive object
     * @param obj The interactive object that was selected or deselected
     * @param isSelected True if the object was selected, false if deselected
     */
    void processSelection(const Handle(AIS_InteractiveObject)& obj, bool isSelected) override;
    
    /**
     * @brief Clears the current selection
     */
    void clearSelection() override;
    
    /**
     * @brief Gets the OCCT interactive context
     * @return The interactive context
     */
    Handle(AIS_InteractiveContext) getContext() const override { return myContext; }
    
    /**
     * @brief Gets the model
     * @return Shared pointer to the model
     */
    std::shared_ptr<IModel> getModel() const override { return myModel; }
    
    /**
     * @brief Property for the display mode
     * 
     * Values:
     * - 0: Shaded
     * - 1: Wireframe
     * - 2: Vertices
     * - etc.
     */
    MVVM::Property<int> displayMode{0};
    
    /**
     * @brief Property for selection state
     */
    MVVM::Property<bool> hasSelectionProperty{false};
    
    /**
     * @brief Property for the number of selected objects
     */
    MVVM::Property<int> selectionCountProperty{0};
    
    /**
     * @brief Sets the color of the selected objects
     * @param color The color to set
     */
    void setSelectedColor(const Quantity_Color& color);
    
    /**
     * @brief Gets the color of the selected objects
     * @return The color of the selected objects
     */
    Quantity_Color getSelectedColor() const;
    
    /**
     * @brief Gets the UnifiedModel with type information preserved
     * @return Shared pointer to the UnifiedModel
     */
    std::shared_ptr<UnifiedModel> getUnifiedModel() const { return myModel; }
    
    /**
     * @brief Gets the OCCT viewer
     * @return The OCCT viewer
     */
    Handle(V3d_Viewer) getViewer() const { return myContext->CurrentViewer(); }
    
    /**
     * @brief Gets the global settings
     * @return Reference to the global settings
     */
    MVVM::GlobalSettings& getGlobalSettings() const { return myGlobalSettings; }
    
    /**
     * @brief Connection tracker for property bindings
     */
    MVVM::ConnectionTracker connections;
    
private:
    /** The model */
    std::shared_ptr<UnifiedModel> myModel;
    
    /** The OCCT interactive context */
    Handle(AIS_InteractiveContext) myContext;
    
    /** Set of selected object IDs */
    std::set<std::string> mySelectedObjects;
    
    /** Reference to the global settings */
    MVVM::GlobalSettings& myGlobalSettings;
    
    /** The model importer */
    std::shared_ptr<ModelImporter> myModelImporter;
    
    /**
     * @brief Updates the visual presentation of a geometry
     * @param id The ID of the geometry to update
     */
    void updatePresentation(const std::string& id);
    
    /** Map from OCCT objects to model IDs */
    std::map<Handle(AIS_InteractiveObject), std::string> myObjectToIdMap;
    
    /** Map from model IDs to OCCT objects */
    std::map<std::string, Handle(AIS_InteractiveObject)> myIdToObjectMap;
    
    /**
     * @brief Creates an appropriate AIS object for a geometry
     * @param id The ID of the geometry
     * @param data The geometry data
     * @return The created AIS object
     */
    Handle(AIS_InteractiveObject) createPresentationForGeometry(
        const std::string& id, const UnifiedModel::GeometryData* data);
    
    /**
     * @brief Callback for model changes
     * @param id The ID of the changed geometry
     */
    void onModelChanged(const std::string& id);
    
    /**
     * @brief Updates selection properties
     */
    void updateSelectionProperties();
}; 