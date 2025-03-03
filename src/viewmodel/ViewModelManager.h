#pragma once

#include "IViewModel.h"
#include "UnifiedViewModel.h"
#include "../model/ModelManager.h"
#include "../mvvm/MessageBus.h"
#include "../mvvm/GlobalSettings.h"
#include <memory>
#include <map>
#include <string>
#include <vector>
#include <AIS_InteractiveContext.hxx>

class ViewModelManager {
public:
    // Constructor with dependency injection
    ViewModelManager(ModelManager& modelManager, 
                    MVVM::MessageBus& messageBus,
                    MVVM::GlobalSettings& globalSettings) 
        : myModelManager(modelManager)
        , myMessageBus(messageBus)
        , myGlobalSettings(globalSettings) {}
    
    // 创建特定类型的ViewModel
    template<typename T, typename ModelT>
    std::shared_ptr<T> createViewModel(const std::string& viewModelId, 
                                      const std::string& modelId,
                                      Handle(AIS_InteractiveContext) context) {
        // 获取或创建模型
        std::shared_ptr<ModelT> model = std::dynamic_pointer_cast<ModelT>(myModelManager.getModel(modelId));
        
        if (!model) {
            model = myModelManager.createModel<ModelT>(modelId);
        }
        
        // 创建ViewModel
        auto viewModel = std::make_shared<T>(model, context, myGlobalSettings);
        myViewModels[viewModelId] = viewModel;
        return viewModel;
    }
    
    // 获取ViewModel
    std::shared_ptr<IViewModel> getViewModel(const std::string& viewModelId) {
        auto it = myViewModels.find(viewModelId);
        if (it != myViewModels.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    // 获取特定类型的ViewModel
    template<typename T>
    std::shared_ptr<T> getViewModel(const std::string& viewModelId) {
        auto viewModel = getViewModel(viewModelId);
        return std::dynamic_pointer_cast<T>(viewModel);
    }
    
    // 移除ViewModel
    void removeViewModel(const std::string& viewModelId) {
        myViewModels.erase(viewModelId);
    }
    
    // 获取所有ViewModel ID
    std::vector<std::string> getAllViewModelIds() const {
        std::vector<std::string> ids;
        ids.reserve(myViewModels.size());
        
        for (const auto& pair : myViewModels) {
            ids.push_back(pair.first);
        }
        
        return ids;
    }
    
    // 获取依赖的服务
    MVVM::MessageBus& getMessageBus() const { return myMessageBus; }
    MVVM::GlobalSettings& getGlobalSettings() const { return myGlobalSettings; }
    
private:
    ModelManager& myModelManager;
    MVVM::MessageBus& myMessageBus;
    MVVM::GlobalSettings& myGlobalSettings;
    std::map<std::string, std::shared_ptr<IViewModel>> myViewModels;
}; 