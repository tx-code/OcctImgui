/**
 * @file ModelManager.h
 * @brief Manages the creation, retrieval, and lifecycle of Model objects in the MVVM architecture.
 * 
 * The ModelManager is responsible for creating and managing model instances,
 * providing a centralized registry for all models in the application.
 */
#pragma once

#include <memory>
#include <map>
#include <string>
#include <vector>
#include "IModel.h"

/**
 * @class ModelManager
 * @brief Manages the lifecycle and access to Model objects in the application.
 * 
 * This class provides methods to create, retrieve, and remove model instances,
 * acting as a central registry for all models. Each model is identified by a unique string ID.
 */
class ModelManager {
public:
    /**
     * @brief Default constructor
     */
    ModelManager() = default;
    
    /**
     * @brief Creates a new model of the specified type
     * @tparam T The model type to create (must inherit from IModel)
     * @param modelId Unique identifier for the model
     * @return Shared pointer to the created model
     */
    template<typename T>
    std::shared_ptr<T> createModel(const std::string& modelId) {
        auto model = std::make_shared<T>();
        myModels[modelId] = model;
        return model;
    }
    
    /**
     * @brief Retrieves a model by its ID
     * @param modelId The ID of the model to retrieve
     * @return Shared pointer to the model, or nullptr if not found
     */
    std::shared_ptr<IModel> getModel(const std::string& modelId) {
        auto it = myModels.find(modelId);
        if (it != myModels.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    /**
     * @brief Removes a model from the manager
     * @param modelId The ID of the model to remove
     */
    void removeModel(const std::string& modelId) {
        myModels.erase(modelId);
    }
    
    /**
     * @brief Gets the IDs of all registered models
     * @return Vector of model IDs
     */
    std::vector<std::string> getAllModelIds() const {
        std::vector<std::string> ids;
        ids.reserve(myModels.size());
        
        for (const auto& pair : myModels) {
            ids.push_back(pair.first);
        }
        
        return ids;
    }
    
private:
    /** Map of model IDs to model instances */
    std::map<std::string, std::shared_ptr<IModel>> myModels;
}; 