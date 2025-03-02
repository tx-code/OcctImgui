#pragma once

#include "IViewModel.h"
#include "CadViewModel.h"
#include "PolyViewModel.h"
#include "../model/ModelManager.h"
#include <memory>
#include <map>
#include <string>
#include <vector>
#include <AIS_InteractiveContext.hxx>

class ViewModelManager {
public:
    static ViewModelManager& instance() {
        static ViewModelManager manager;
        return manager;
    }
    
    // 创建特定类型的ViewModel
    template<typename T, typename ModelT>
    std::shared_ptr<T> createViewModel(const std::string& viewModelId, 
                                      const std::string& modelId,
                                      Handle(AIS_InteractiveContext) context) {
        // 获取或创建模型
        auto& modelManager = ModelManager::instance();
        std::shared_ptr<ModelT> model = std::dynamic_pointer_cast<ModelT>(modelManager.getModel(modelId));
        
        if (!model) {
            model = modelManager.createModel<ModelT>(modelId);
        }
        
        // 创建ViewModel
        auto viewModel = std::make_shared<T>(model, context);
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
    
private:
    ViewModelManager() = default;
    ~ViewModelManager() = default;
    
    std::map<std::string, std::shared_ptr<IViewModel>> myViewModels;
}; 