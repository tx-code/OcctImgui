/**
 * @file WindowManager.h
 * @brief Defines the WindowManager class that handles window creation and management.
 */
#pragma once

#include "GlfwOcctWindow.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <string>

// Forward declarations
class GLFWwindow;
namespace Utils
{
class Logger;
}

/**
 * @class WindowManager
 * @brief Manages window creation and lifecycle.
 *
 * This class is responsible for creating and managing the application window,
 * handling window events, and providing access to the window instance.
 */
class WindowManager
{
public:
    /**
     * @brief Constructor
     *
     * @param width Initial window width
     * @param height Initial window height
     * @param title Window title
     */
    WindowManager(int width = 800, int height = 600, const std::string& title = "OCCT MVVM");

    /**
     * @brief Destructor
     */
    ~WindowManager();

    /**
     * @brief Initialize the window
     *
     * Creates the GLFW window and sets up callbacks.
     *
     * @return True if initialization was successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Check if the window should close
     *
     * @return True if the window should close, false otherwise
     */
    bool shouldClose() const;

    /**
     * @brief Poll for window events
     */
    void pollEvents() const;

    /**
     * @brief Swap the window buffers
     */
    void swapBuffers() const;

    /**
     * @brief Close the window
     */
    void close();

    /**
     * @brief Get the OCCT window
     *
     * @return Reference to the OCCT window
     */
    const Handle(GlfwOcctWindow) & getOcctWindow() const;

private:
    /**
     * @brief Set OpenGL context hints
     */
    void setContextHints();

    /**
     * @brief Log GLFW version information
     */
    void logGlfwVersion();

    /**
     * @brief Static error callback for GLFW
     *
     * @param error The error code
     * @param description The error description
     */
    static void errorCallback(int error, const char* description);

    /** Window width */
    int myWidth;

    /** Window height */
    int myHeight;

    /** Window title */
    std::string myTitle;

    /** OCCT window */
    Handle(GlfwOcctWindow) myWindow;

    /** GLFW window */
    GLFWwindow* myGlfwWindow;

    /** Logger */
    std::shared_ptr<Utils::Logger> myLogger;
};