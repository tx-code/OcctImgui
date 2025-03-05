/**
 * @file ViewModelManager.h
 * @brief Manages the creation, retrieval, and lifecycle of ViewModel objects in the MVVM architecture.
 * 
 * The ViewModelManager is responsible for creating and managing viewmodel instances,
 * providing a centralized registry for all viewmodels in the application.
 * It coordinates with the ModelManager to ensure proper model-viewmodel relationships.
 */
#pragma once

#include "IViewModel.h"
#include "UnifiedViewModel.h"
#include "../model/ModelManager.h"
#include "../model/ModelImporter.h"
#include "../mvvm/MessageBus.h"
#include "../mvvm/GlobalSettings.h"
#include <memory>
#include <map>
#include <string>
#include <vector>
#include <AIS_InteractiveContext.hxx>

/**
 * @class ViewModelManager
 * @brief Manages the lifecycle and access to ViewModel objects in the application.
 * 
 * This class provides methods to create, retrieve, and remove viewmodel instances,
 * acting as a central registry for all viewmodels. Each viewmodel is identified by a unique string ID
 * and is associated with a model from the ModelManager.
 */
class ViewModelManager {
public:
    /**
     * @brief Constructor with dependency injection
     * @param modelManager Reference to the ModelManager for model access
     * @param messageBus Reference to the MessageBus for event communication
     * @param globalSettings Reference to the GlobalSettings for application-wide settings
     * @param modelImporter Reference to the ModelImporter for model import functionality
     */
    ViewModelManager(ModelManager& modelManager, 
                    MVVM::MessageBus& messageBus,
                    MVVM::GlobalSettings& globalSettings,
                    ModelImporter& modelImporter) 
        : myModelManager(modelManager)
        , myMessageBus(messageBus)
        , myGlobalSettings(globalSettings)
        , myModelImporter(modelImporter) {}
    
    /**
     * @brief Creates a new viewmodel of the specified type
     * @tparam T The viewmodel type to create (must inherit from IViewModel)
     * @tparam ModelT The model type required by the viewmodel
     * @param viewModelId Unique identifier for the viewmodel
     * @param modelId Identifier of the model to associate with the viewmodel
     * @param context The AIS_InteractiveContext for visualization
     * @return Shared pointer to the created viewmodel
     */
    template<typename T, typename ModelT>
    std::shared_ptr<T> createViewModel(const std::string& viewModelId, 
                                      const std::string& modelId,
                                      Handle(AIS_InteractiveContext) context) {
        // Get or create the model
        std::shared_ptr<ModelT> model = std::dynamic_pointer_cast<ModelT>(myModelManager.getModel(modelId));
        
        if (!model) {
            model = myModelManager.createModel<ModelT>(modelId);
        }
        
        // Create the ViewModel with ModelImporter
        auto viewModel = std::make_shared<T>(model, context, myGlobalSettings, 
                                            std::make_shared<ModelImporter>(myModelImporter));
        myViewModels[viewModelId] = viewModel;
        return viewModel;
    }
    
    /**
     * @brief Retrieves a viewmodel by its ID
     * @param viewModelId The ID of the viewmodel to retrieve
     * @return Shared pointer to the viewmodel, or nullptr if not found
     */
    std::shared_ptr<IViewModel> getViewModel(const std::string& viewModelId) {
        auto it = myViewModels.find(viewModelId);
        if (it != myViewModels.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    /**
     * @brief Retrieves a viewmodel by its ID with type casting
     * @tparam T The expected type of the viewmodel
     * @param viewModelId The ID of the viewmodel to retrieve
     * @return Shared pointer to the viewmodel of type T, or nullptr if not found or wrong type
     */
    template<typename T>
    std::shared_ptr<T> getViewModel(const std::string& viewModelId) {
        auto viewModel = getViewModel(viewModelId);
        return std::dynamic_pointer_cast<T>(viewModel);
    }
    
    /**
     * @brief Removes a viewmodel from the manager
     * @param viewModelId The ID of the viewmodel to remove
     */
    void removeViewModel(const std::string& viewModelId) {
        myViewModels.erase(viewModelId);
    }
    
    /**
     * @brief Gets the IDs of all registered viewmodels
     * @return Vector of viewmodel IDs
     */
    std::vector<std::string> getAllViewModelIds() const {
        std::vector<std::string> ids;
        ids.reserve(myViewModels.size());
        
        for (const auto& pair : myViewModels) {
            ids.push_back(pair.first);
        }
        
        return ids;
    }
    
    /**
     * @brief Gets the message bus instance
     * @return Reference to the message bus
     */
    MVVM::MessageBus& getMessageBus() const { return myMessageBus; }
    
    /**
     * @brief Gets the global settings instance
     * @return Reference to the global settings
     */
    MVVM::GlobalSettings& getGlobalSettings() const { return myGlobalSettings; }
    
private:
    /** Reference to the model manager */
    ModelManager& myModelManager;
    
    /** Reference to the message bus for event communication */
    MVVM::MessageBus& myMessageBus;
    
    /** Reference to the global settings */
    MVVM::GlobalSettings& myGlobalSettings;
    
    /** Reference to the model importer */
    ModelImporter& myModelImporter;
    
    /** Map of viewmodel IDs to viewmodel instances */
    std::map<std::string, std::shared_ptr<IViewModel>> myViewModels;
}; 