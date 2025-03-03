#pragma once

#include "IView.h"
#include "ImGuiView.h"
#include "OcctView.h"
#include "../viewmodel/ViewModelManager.h"
#include "../GlfwOcctWindow.h"
#include "../utils/Logger.h"
#include <memory>
#include <map>
#include <string>
#include <vector>

struct GLFWwindow;

// 创建ViewManager日志记录器 - 使用函数确保安全初始化
inline std::shared_ptr<Utils::Logger>& getViewManagerLogger() {
    static std::shared_ptr<Utils::Logger> logger = Utils::Logger::getLogger("view.manager");
    return logger;
}

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
            getViewManagerLogger()->error("Failed to get ViewModel with ID: {}", viewModelId);
            return nullptr;
        }
        
        // 创建View
        auto view = std::make_shared<T>(viewModel);
        myViews[viewId] = view;
        getViewManagerLogger()->info("Created view with ID: {}", viewId);
        return view;
    }
    
    // 专门用于创建OcctView的工厂方法
    std::shared_ptr<OcctView> createOcctView(const std::string& viewId,
                                           const std::string& viewModelId,
                                           Handle(GlfwOcctWindow) window) {
        // 获取ViewModel
        auto& viewModelManager = ViewModelManager::instance();
        auto viewModel = viewModelManager.getViewModel<UnifiedViewModel>(viewModelId);
        
        if (!viewModel) {
            getViewManagerLogger()->error("Failed to get ViewModel with ID: {}", viewModelId);
            return nullptr;
        }
        
        // 创建OcctView
        auto view = std::make_shared<OcctView>(viewModel, window);
        myViews[viewId] = view;
        getViewManagerLogger()->info("Created OcctView with ID: {}", viewId);
        return view;
    }
    
    // 初始化所有视图
    void initializeAll(GLFWwindow* window) {
        LOG_FUNCTION_SCOPE(getViewManagerLogger(), "initializeAll");
        for (auto& pair : myViews) {
            pair.second->initialize(window);
        }
    }
    
    // 初始化特定的视图
    void initializeView(const std::string& viewId, GLFWwindow* window) {
        auto view = getView(viewId);
        if (view) {
            view->initialize(window);
            getViewManagerLogger()->info("Initialized view with ID: {}", viewId);
        } else {
            getViewManagerLogger()->warn("Cannot initialize view with ID: {}, view not found", viewId);
        }
    }
    
    // 渲染所有视图
    void renderAll() {
        for (auto& pair : myViews) {
            pair.second->newFrame();
            pair.second->render();
        }
    }
    
    // 按照指定的顺序渲染视图
    void renderInOrder(const std::vector<std::string>& viewIds) {
        for (const auto& viewId : viewIds) {
            auto view = getView(viewId);
            if (view) {
                view->newFrame();
                view->render();
                getViewManagerLogger()->debug("Rendered view with ID: {}", viewId);
            } else {
                getViewManagerLogger()->warn("Cannot render view with ID: {}, view not found", viewId);
            }
        }
    }
    
    // 关闭所有视图
    void shutdownAll() {
        LOG_FUNCTION_SCOPE(getViewManagerLogger(), "shutdownAll");
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
            getViewManagerLogger()->info("Removed view with ID: {}", viewId);
        } else {
            getViewManagerLogger()->warn("Cannot remove view with ID: {}, view not found", viewId);
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
    
    // 事件处理方法
    void handleResize(const std::string& occtViewId, int width, int height) {
        auto occtView = getView<OcctView>(occtViewId);
        if (occtView) {
            occtView->onResize(width, height);
            getViewManagerLogger()->debug("Handled resize event for view with ID: {}", occtViewId);
        }
    }
    
    void handleMouseScroll(const std::string& occtViewId, double offsetX, double offsetY) {
        if (!anyViewWantCaptureMouse()) {
            auto occtView = getView<OcctView>(occtViewId);
            if (occtView) {
                occtView->onMouseScroll(offsetX, offsetY);
                getViewManagerLogger()->debug("Handled mouse scroll event for view with ID: {}", occtViewId);
            }
        }
    }
    
    void handleMouseButton(const std::string& occtViewId, int button, int action, int mods) {
        if (!anyViewWantCaptureMouse()) {
            auto occtView = getView<OcctView>(occtViewId);
            if (occtView) {
                occtView->onMouseButton(button, action, mods);
                getViewManagerLogger()->debug("Handled mouse button event for view with ID: {}", occtViewId);
            }
        }
    }
    
    void handleMouseMove(const std::string& occtViewId, double posX, double posY) {
        if (!anyViewWantCaptureMouse()) {
            auto occtView = getView<OcctView>(occtViewId);
            if (occtView) {
                occtView->onMouseMove(posX, posY);
                getViewManagerLogger()->debug("Handled mouse move event for view with ID: {}", occtViewId);
            }
        }
    }
    
private:
    ViewManager() = default;
    ~ViewManager() = default;
    
    std::map<std::string, std::shared_ptr<IView>> myViews;
}; 