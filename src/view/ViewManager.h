/**
 * @file ViewManager.h
 * @brief Manages the creation, retrieval, and lifecycle of View objects in the MVVM architecture.
 * 
 * The ViewManager is responsible for creating and managing view instances,
 * providing a centralized registry for all views in the application.
 * It coordinates with the ViewModelManager to ensure proper view-viewmodel relationships.
 */
#pragma once

#include "IView.h"
#include "ImGuiView.h"
#include "OcctView.h"
#include "../viewmodel/ViewModelManager.h"
#include "../GlfwOcctWindow.h"
#include "../utils/Logger.h"
#include "../mvvm/MessageBus.h"
#include <memory>
#include <map>
#include <string>
#include <vector>

struct GLFWwindow;

/**
 * @brief Creates and returns the ViewManager logger
 * @return Reference to the ViewManager logger instance
 */
inline std::shared_ptr<Utils::Logger>& getViewManagerLogger() {
    static std::shared_ptr<Utils::Logger> logger = Utils::Logger::getLogger("view.manager");
    return logger;
}

/**
 * @class ViewManager
 * @brief Manages the lifecycle and access to View objects in the application.
 * 
 * This class provides methods to create, retrieve, and remove view instances,
 * acting as a central registry for all views. Each view is identified by a unique string ID
 * and is associated with a viewmodel from the ViewModelManager.
 */
class ViewManager {
public:
    /**
     * @brief Constructor with dependency injection
     * @param viewModelManager Reference to the ViewModelManager for viewmodel access
     * @param messageBus Reference to the MessageBus for event communication
     */
    ViewManager(ViewModelManager& viewModelManager, MVVM::MessageBus& messageBus) 
        : myViewModelManager(viewModelManager), myMessageBus(messageBus) {}
    
    /**
     * @brief Creates a new view of the specified type
     * @tparam T The view type to create (must inherit from IView)
     * @param viewId Unique identifier for the view
     * @param viewModelId Identifier of the viewmodel to associate with the view
     * @return Shared pointer to the created view
     */
    template<typename T>
    std::shared_ptr<T> createView(const std::string& viewId, 
                                 const std::string& viewModelId) {
        // Get the ViewModel
        auto viewModel = myViewModelManager.getViewModel(viewModelId);
        
        if (!viewModel) {
            getViewManagerLogger()->error("Failed to get ViewModel with ID: {}", viewModelId);
            return nullptr;
        }
        
        // Create the View
        auto view = std::make_shared<T>(viewModel);
        myViews[viewId] = view;
        getViewManagerLogger()->info("Created view with ID: {}", viewId);
        return view;
    }
    
    /**
     * @brief Factory method specifically for creating OcctView instances
     * @param viewId Unique identifier for the view
     * @param viewModelId Identifier of the viewmodel to associate with the view
     * @param window The GLFW OCCT window for rendering
     * @return Shared pointer to the created OcctView
     */
    std::shared_ptr<OcctView> createOcctView(const std::string& viewId,
                                           const std::string& viewModelId,
                                           Handle(GlfwOcctWindow) window) {
        // Get the ViewModel
        auto viewModel = myViewModelManager.getViewModel<UnifiedViewModel>(viewModelId);
        
        if (!viewModel) {
            getViewManagerLogger()->error("Failed to get ViewModel with ID: {}", viewModelId);
            return nullptr;
        }
        
        // Create OcctView
        auto view = std::make_shared<OcctView>(viewModel, window, myMessageBus);
        myViews[viewId] = view;
        getViewManagerLogger()->info("Created OcctView with ID: {}", viewId);
        return view;
    }
    
    /**
     * @brief Initializes all registered views
     * @param window The GLFW window to initialize views with
     */
    void initializeAll(GLFWwindow* window) {
        LOG_FUNCTION_SCOPE(getViewManagerLogger(), "initializeAll");
        for (auto& pair : myViews) {
            pair.second->initialize(window);
        }
    }
    
    /**
     * @brief Initializes a specific view
     * @param viewId The ID of the view to initialize
     * @param window The GLFW window to initialize the view with
     */
    void initializeView(const std::string& viewId, GLFWwindow* window) {
        auto view = getView(viewId);
        if (view) {
            view->initialize(window);
            getViewManagerLogger()->info("Initialized view with ID: {}", viewId);
        } else {
            getViewManagerLogger()->warn("Cannot initialize view with ID: {}, view not found", viewId);
        }
    }
    
    /**
     * @brief Renders all registered views
     */
    void renderAll() {
        for (auto& pair : myViews) {
            pair.second->newFrame();
            pair.second->render();
        }
    }
    
    /**
     * @brief Renders views in the specified order
     * @param viewIds Vector of view IDs in the order they should be rendered
     */
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
    
    /**
     * @brief Shuts down all views and clears the view registry
     */
    void shutdownAll() {
        LOG_FUNCTION_SCOPE(getViewManagerLogger(), "shutdownAll");
        for (auto& pair : myViews) {
            pair.second->shutdown();
        }
        myViews.clear();
    }
    
    /**
     * @brief Retrieves a view by its ID
     * @param viewId The ID of the view to retrieve
     * @return Shared pointer to the view, or nullptr if not found
     */
    std::shared_ptr<IView> getView(const std::string& viewId) {
        auto it = myViews.find(viewId);
        if (it != myViews.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    /**
     * @brief Retrieves a view by its ID with type casting
     * @tparam T The expected type of the view
     * @param viewId The ID of the view to retrieve
     * @return Shared pointer to the view of type T, or nullptr if not found or wrong type
     */
    template<typename T>
    std::shared_ptr<T> getView(const std::string& viewId) {
        auto view = getView(viewId);
        return std::dynamic_pointer_cast<T>(view);
    }
    
    /**
     * @brief Removes a view from the manager and shuts it down
     * @param viewId The ID of the view to remove
     */
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
    
    /**
     * @brief Gets the IDs of all registered views
     * @return Vector of view IDs
     */
    std::vector<std::string> getAllViewIds() const {
        std::vector<std::string> ids;
        ids.reserve(myViews.size());
        
        for (const auto& pair : myViews) {
            ids.push_back(pair.first);
        }
        
        return ids;
    }
    
    /**
     * @brief Checks if any view wants to capture mouse input
     * @return True if any view wants to capture mouse input, false otherwise
     */
    bool anyViewWantCaptureMouse() const {
        for (const auto& pair : myViews) {
            if (pair.second->wantCaptureMouse()) {
                return true;
            }
        }
        return false;
    }
    
    /**
     * @brief Handles window resize events for a specific view
     * @param occtViewId The ID of the OcctView to handle the event
     * @param width The new width of the window
     * @param height The new height of the window
     */
    void handleResize(const std::string& occtViewId, int width, int height) {
        auto occtView = getView<OcctView>(occtViewId);
        if (occtView) {
            occtView->onResize(width, height);
            getViewManagerLogger()->debug("Handled resize event for view with ID: {}", occtViewId);
        }
    }
    
    /**
     * @brief Handles mouse scroll events for a specific view
     * @param occtViewId The ID of the OcctView to handle the event
     * @param offsetX The horizontal scroll offset
     * @param offsetY The vertical scroll offset
     */
    void handleMouseScroll(const std::string& occtViewId, double offsetX, double offsetY) {
        if (!anyViewWantCaptureMouse()) {
            auto occtView = getView<OcctView>(occtViewId);
            if (occtView) {
                occtView->onMouseScroll(offsetX, offsetY);
                getViewManagerLogger()->debug("Handled mouse scroll event for view with ID: {}", occtViewId);
            }
        }
    }
    
    /**
     * @brief Handles mouse button events for a specific view
     * @param occtViewId The ID of the OcctView to handle the event
     * @param button The mouse button that was pressed or released
     * @param action The action (press, release) that occurred
     * @param mods Modifier keys that were held down
     */
    void handleMouseButton(const std::string& occtViewId, int button, int action, int mods) {
        if (!anyViewWantCaptureMouse()) {
            auto occtView = getView<OcctView>(occtViewId);
            if (occtView) {
                occtView->onMouseButton(button, action, mods);
                getViewManagerLogger()->debug("Handled mouse button event for view with ID: {}", occtViewId);
            }
        }
    }
    
    /**
     * @brief Handles mouse movement events for a specific view
     * @param occtViewId The ID of the OcctView to handle the event
     * @param posX The x-coordinate of the mouse position
     * @param posY The y-coordinate of the mouse position
     */
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
    /** Reference to the viewmodel manager */
    ViewModelManager& myViewModelManager;
    
    /** Reference to the message bus for event communication */
    MVVM::MessageBus& myMessageBus;
    
    /** Map of view IDs to view instances */
    std::map<std::string, std::shared_ptr<IView>> myViews;
}; 