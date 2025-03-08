/**
 * @file ApplicationBootstrapper.h
 * @brief Defines the ApplicationBootstrapper class that initializes application components.
 */
#pragma once

#include "input/InputManager.h"
#include "model/ModelFactory.h"
#include "model/ModelImporter.h"
#include "model/ModelManager.h"
#include "mvvm/GlobalSettings.h"
#include "mvvm/MessageBus.h"
#include "mvvm/SelectionManager.h"
#include "view/ViewManager.h"
#include "viewmodel/ViewModelManager.h"
#include "window/WindowManager.h"


#include <memory>
#include <string>

// Forward declarations
namespace Utils
{
class Logger;
}

/**
 * @class ApplicationBootstrapper
 * @brief Initializes and manages application components.
 *
 * This class is responsible for initializing and managing the lifecycle of all
 * application components, including the window, input, models, viewmodels, and views.
 */
class ApplicationBootstrapper
{
public:
    /**
     * @brief Constructor
     */
    ApplicationBootstrapper();

    /**
     * @brief Destructor
     */
    ~ApplicationBootstrapper();

    /**
     * @brief Initialize all application components
     *
     * @return True if initialization was successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Get the window manager
     *
     * @return Reference to the window manager
     */
    [[nodiscard]] WindowManager& getWindowManager() const
    {
        return *myWindowManager;
    }

    /**
     * @brief Get the view manager
     *
     * @return Reference to the view manager
     */
    [[nodiscard]] ViewManager& getViewManager() const
    {
        return *myViewManager;
    }

private:
    /**
     * @brief Initialize the window
     *
     * @return True if initialization was successful, false otherwise
     */
    bool initializeWindow();

    /**
     * @brief Initialize the input manager
     *
     * @return True if initialization was successful, false otherwise
     */
    bool initializeInput();

    /**
     * @brief Initialize the model
     *
     * @return True if initialization was successful, false otherwise
     */
    bool initializeModel();

    /**
     * @brief Initialize the viewmodel
     *
     * @return True if initialization was successful, false otherwise
     */
    bool initializeViewModel();

    /**
     * @brief Initialize the views
     *
     * @return True if initialization was successful, false otherwise
     */
    bool initializeViews();

    /** Window manager */
    std::unique_ptr<WindowManager> myWindowManager;

    /** Input manager */
    std::unique_ptr<InputManager> myInputManager;

    /** Global settings */
    std::unique_ptr<MVVM::GlobalSettings> myGlobalSettings;

    /** Model factory */
    std::unique_ptr<ModelFactory> myModelFactory;

    /** Model manager */
    std::unique_ptr<ModelManager> myModelManager;

    /** Model importer */
    std::unique_ptr<ModelImporter> myModelImporter;

    /** Viewmodel manager */
    std::unique_ptr<ViewModelManager> myViewModelManager;

    /** View manager */
    std::unique_ptr<ViewManager> myViewManager;

    /** Logger */
    std::shared_ptr<Utils::Logger> myLogger;

    /** Model ID */
    std::string myModelId;

    /** Viewmodel ID */
    std::string myViewModelId;

    /** ImGui view ID */
    std::string myImGuiViewId;

    /** OCCT view ID */
    std::string myOcctViewId;
};
