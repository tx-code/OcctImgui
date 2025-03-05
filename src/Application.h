/**
 * @file Application.h
 * @brief Defines the main Application class that coordinates the MVVM architecture.
 * 
 * The Application class is the central coordinator of the application, managing
 * the lifecycle of all components and their interactions. It initializes the window,
 * models, viewmodels, and views, and handles the main application loop.
 */
#pragma once

#include "GlfwOcctWindow.h"
#include "model/UnifiedModel.h"
#include "viewmodel/UnifiedViewModel.h"
#include "view/ImGuiView.h"
#include "view/OcctView.h"
#include "view/ViewManager.h"
#include "model/ModelManager.h"
#include "model/ModelFactory.h"
#include "model/ModelImporter.h"
#include "viewmodel/ViewModelManager.h"
#include "mvvm/MessageBus.h"
#include "mvvm/GlobalSettings.h"

#include <memory>
#include <string>

/**
 * @class Application
 * @brief Main application class that coordinates the MVVM architecture.
 * 
 * This class is responsible for initializing and managing all components of the application,
 * including the window, models, viewmodels, and views. It also handles the main application
 * loop and event processing.
 */
class Application {
public:
    /**
     * @brief Constructor
     * 
     * Initializes the application and its components.
     */
    Application();
    
    /**
     * @brief Destructor
     * 
     * Cleans up resources used by the application.
     */
    ~Application();
    
    /**
     * @brief Runs the application
     * 
     * Initializes components and enters the main application loop.
     */
    void run();
    
    // Get manager instances
    /**
     * @brief Gets the view manager
     * @return Reference to the view manager
     */
    ViewManager& getViewManager() { return *myViewManager; }
    
    /**
     * @brief Gets the model manager
     * @return Reference to the model manager
     */
    ModelManager& getModelManager() { return *myModelManager; }
    
    /**
     * @brief Gets the viewmodel manager
     * @return Reference to the viewmodel manager
     */
    ViewModelManager& getViewModelManager() { return *myViewModelManager; }
    
    /**
     * @brief Gets the message bus
     * @return Reference to the message bus
     */
    MVVM::MessageBus& getMessageBus() { return *myMessageBus; }
    
    /**
     * @brief Gets the global settings
     * @return Reference to the global settings
     */
    MVVM::GlobalSettings& getGlobalSettings() { return *myGlobalSettings; }
    
    /**
     * @brief Gets the model factory
     * @return Reference to the model factory
     */
    ModelFactory& getModelFactory() { return *myModelFactory; }
    
    /**
     * @brief Gets the model importer
     * @return Reference to the model importer
     */
    ModelImporter& getModelImporter() { return *myModelImporter; }
    
    /**
     * @brief Imports a model from a file
     * 
     * @param filePath The path to the model file
     * @param modelId The ID to assign to the imported model (if empty, the filename will be used)
     * @return bool True if import was successful, false otherwise
     */
    bool importModel(const std::string& filePath, const std::string& modelId = "");

private:
    // Initialization methods
    /**
     * @brief Initializes the window
     */
    void initWindow();
    
    /**
     * @brief Initializes the model
     */
    void initModel();
    
    /**
     * @brief Initializes the viewmodel
     */
    void initViewModel();
    
    /**
     * @brief Initializes the views
     */
    void initViews();
    
    // Event callbacks
    /**
     * @brief Callback for window resize events
     * @param theWin The GLFW window
     * @param theWidth The new width
     * @param theHeight The new height
     */
    static void onResizeCallback(GLFWwindow* theWin, int theWidth, int theHeight);
    
    /**
     * @brief Callback for framebuffer resize events
     * @param theWin The GLFW window
     * @param theWidth The new width
     * @param theHeight The new height
     */
    static void onFBResizeCallback(GLFWwindow* theWin, int theWidth, int theHeight);
    
    /**
     * @brief Callback for mouse scroll events
     * @param theWin The GLFW window
     * @param theOffsetX The horizontal scroll offset
     * @param theOffsetY The vertical scroll offset
     */
    static void onMouseScrollCallback(GLFWwindow* theWin, double theOffsetX, double theOffsetY);
    
    /**
     * @brief Callback for mouse button events
     * @param theWin The GLFW window
     * @param theButton The mouse button
     * @param theAction The action (press, release)
     * @param theMods Modifier keys
     */
    static void onMouseButtonCallback(GLFWwindow* theWin, int theButton, int theAction, int theMods);
    
    /**
     * @brief Callback for mouse movement events
     * @param theWin The GLFW window
     * @param thePosX The x-coordinate of the mouse position
     * @param thePosY The y-coordinate of the mouse position
     */
    static void onMouseMoveCallback(GLFWwindow* theWin, double thePosX, double thePosY);
    
    /**
     * @brief Callback for GLFW errors
     * @param theError The error code
     * @param theDescription The error description
     */
    static void errorCallback(int theError, const char* theDescription);
    
    /**
     * @brief Gets the Application instance from a GLFW window
     * @param theWin The GLFW window
     * @return Pointer to the Application instance
     */
    static Application* toApplication(GLFWwindow* theWin);
    
    // Main loop
    /**
     * @brief Runs the main application loop
     */
    void mainloop();
    
    /**
     * @brief Cleans up resources
     */
    void cleanup();
    
    // MVVM components
    /** The main model */
    std::shared_ptr<UnifiedModel> myModel;
    /** The main viewmodel */
    std::shared_ptr<UnifiedViewModel> myViewModel;
    /** The ImGui view */
    std::shared_ptr<ImGuiView> myImGuiView;
    /** The OCCT view */
    std::shared_ptr<OcctView> myOcctView;
    
    // MVVM component IDs
    /** ID of the model */
    std::string myModelId;
    /** ID of the viewmodel */
    std::string myViewModelId;
    /** ID of the ImGui view */
    std::string myImGuiViewId;
    /** ID of the OCCT view */
    std::string myOcctViewId;
    
    // Window management
    /** The OCCT window */
    Handle(GlfwOcctWindow) myWindow;
    /** The GLFW window */
    GLFWwindow* myGlfwWindow;
    
    // Configuration
    /** The window width */
    int myWidth;
    /** The window height */
    int myHeight;
    /** The window title */
    TCollection_AsciiString myTitle;

    // Manager instances owned by the application
    /** The view manager */
    std::unique_ptr<ViewManager> myViewManager;
    /** The model manager */
    std::unique_ptr<ModelManager> myModelManager;
    /** The viewmodel manager */
    std::unique_ptr<ViewModelManager> myViewModelManager;
    /** The message bus */
    std::unique_ptr<MVVM::MessageBus> myMessageBus;
    /** The global settings */
    std::unique_ptr<MVVM::GlobalSettings> myGlobalSettings;
    /** The model factory */
    std::unique_ptr<ModelFactory> myModelFactory;
    /** The model importer */
    std::unique_ptr<ModelImporter> myModelImporter;
}; 