#pragma once

#include "IView.h"
#include "ImGuiView.h"
#include "../viewmodel/ViewModelManager.h"
#include <memory>
#include <map>
#include <string>
#include <vector>

struct GLFWwindow;

class ViewManager {
public:
    static ViewManager& instance() {
        static ViewManager manager;
        return manager;
    }
    
    // 创建视图
    template<typename T>
    std::shared_ptr<T> createView(const std::string& viewId, 
                                 const std::string& viewModelId) {
        // 获取ViewModel
        auto& viewModelManager = ViewModelManager::instance();
        auto viewModel = viewModelManager.getViewModel(viewModelId);
        
        if (!viewModel) {
            return nullptr;
        }
        
        // 创建View
        auto view = std::make_shared<T>(viewModel);
        myViews[viewId] = view;
        return view;
    }
    
    // 初始化所有视图
    void initializeAll(GLFWwindow* window) {
        for (auto& pair : myViews) {
            pair.second->initialize(window);
        }
    }
    
    // 渲染所有视图
    void renderAll() {
        for (auto& pair : myViews) {
            pair.second->newFrame();
            pair.second->render();
        }
    }
    
    // 关闭所有视图
    void shutdownAll() {
        for (auto& pair : myViews) {
            pair.second->shutdown();
        }
        myViews.clear();
    }
    
    // 获取视图
    std::shared_ptr<IView> getView(const std::string& viewId) {
        auto it = myViews.find(viewId);
        if (it != myViews.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    // 获取特定类型的视图
    template<typename T>
    std::shared_ptr<T> getView(const std::string& viewId) {
        auto view = getView(viewId);
        return std::dynamic_pointer_cast<T>(view);
    }
    
    // 移除视图
    void removeView(const std::string& viewId) {
        auto it = myViews.find(viewId);
        if (it != myViews.end()) {
            it->second->shutdown();
            myViews.erase(it);
        }
    }
    
    // 获取所有视图ID
    std::vector<std::string> getAllViewIds() const {
        std::vector<std::string> ids;
        ids.reserve(myViews.size());
        
        for (const auto& pair : myViews) {
            ids.push_back(pair.first);
        }
        
        return ids;
    }
    
    // 检查是否有视图想要捕获鼠标
    bool anyViewWantCaptureMouse() const {
        for (const auto& pair : myViews) {
            if (pair.second->wantCaptureMouse()) {
                return true;
            }
        }
        return false;
    }
    
private:
    ViewManager() = default;
    ~ViewManager() = default;
    
    std::map<std::string, std::shared_ptr<IView>> myViews;
}; 