#pragma once

#include <memory>
#include <map>
#include <string>
#include <vector>
#include "IModel.h"

class ModelManager {
public:
    // Constructor
    ModelManager() = default;
    
    template<typename T>
    std::shared_ptr<T> createModel(const std::string& modelId) {
        auto model = std::make_shared<T>();
        myModels[modelId] = model;
        return model;
    }
    
    std::shared_ptr<IModel> getModel(const std::string& modelId) {
        auto it = myModels.find(modelId);
        if (it != myModels.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    void removeModel(const std::string& modelId) {
        myModels.erase(modelId);
    }
    
    std::vector<std::string> getAllModelIds() const {
        std::vector<std::string> ids;
        ids.reserve(myModels.size());
        
        for (const auto& pair : myModels) {
            ids.push_back(pair.first);
        }
        
        return ids;
    }
    
private:
    std::map<std::string, std::shared_ptr<IModel>> myModels;
}; 