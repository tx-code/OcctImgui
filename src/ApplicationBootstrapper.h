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
    WindowManager& getWindowManager() const
    {
        return *myWindowManager;
    }

    /**
     * @brief Get the input manager
     *
     * @return Reference to the input manager
     */
    InputManager& getInputManager() const
    {
        return *myInputManager;
    }

    /**
     * @brief Get the model manager
     *
     * @return Reference to the model manager
     */
    ModelManager& getModelManager() const
    {
        return *myModelManager;
    }

    /**
     * @brief Get the view manager
     *
     * @return Reference to the view manager
     */
    ViewManager& getViewManager() const
    {
        return *myViewManager;
    }

    /**
     * @brief Get the viewmodel manager
     *
     * @return Reference to the viewmodel manager
     */
    ViewModelManager& getViewModelManager() const
    {
        return *myViewModelManager;
    }

    /**
     * @brief Get the message bus
     *
     * @return Reference to the message bus
     */
    MVVM::MessageBus& getMessageBus() const
    {
        return MVVM::MessageBus::getInstance();
    }

    /**
     * @brief Get the selection manager
     *
     * @return Reference to the selection manager
     */
    MVVM::SelectionManager& getSelectionManager() const
    {
        return MVVM::SelectionManager::getInstance();
    }

    /**
     * @brief Get the global settings
     *
     * @return Reference to the global settings
     */
    MVVM::GlobalSettings& getGlobalSettings() const
    {
        return *myGlobalSettings;
    }

    /**
     * @brief Get the model factory
     *
     * @return Reference to the model factory
     */
    ModelFactory& getModelFactory() const
    {
        return *myModelFactory;
    }

    /**
     * @brief Get the model importer
     *
     * @return Reference to the model importer
     */
    ModelImporter& getModelImporter() const
    {
        return *myModelImporter;
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
