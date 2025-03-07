/**
 * @file Application.h
 * @brief Defines the main Application class that coordinates the MVVM architecture.
 *
 * The Application class is the central coordinator of the application, managing
 * the lifecycle of all components and their interactions.
 */
#pragma once

#include "ApplicationBootstrapper.h"
#include <memory>
#include <string>

// Forward declarations
namespace Utils
{
class Logger;
}

/**
 * @class Application
 * @brief Main application class that coordinates the MVVM architecture.
 *
 * This class is responsible for the application lifecycle, including initialization,
 * running the main loop, and cleanup. It uses ApplicationBootstrapper to initialize
 * and manage the components.
 */
class Application
{
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

private:
    /**
     * @brief Runs the main application loop
     */
    void mainloop();

    /**
     * @brief Cleans up resources
     */
    void cleanup();

    /** Application bootstrapper */
    std::unique_ptr<ApplicationBootstrapper> myBootstrapper;

    /** Flag indicating if the application is running */
    bool myIsRunning;

    /** Logger */
    std::shared_ptr<Utils::Logger> myLogger;
};