/**
 * @file InputManager.h
 * @brief Defines the InputManager class that handles input events.
 */
#pragma once

#include <functional>
#include <memory>

// Forward declarations
class GLFWwindow;
namespace Utils
{
class Logger;
}

/**
 * @class InputManager
 * @brief Manages input events from the window.
 *
 * This class is responsible for handling input events from the window,
 * such as mouse and keyboard events, and dispatching them to the appropriate
 * handlers.
 */
class InputManager
{
public:
    /**
     * @brief Constructor
     */
    InputManager();

    /**
     * @brief Destructor
     */
    ~InputManager();

    /**
     * @brief Initialize the input manager
     *
     * @param window The GLFW window to handle input for
     * @return True if initialization was successful, false otherwise
     */
    bool initialize(GLFWwindow* window);

    /**
     * @brief Set the resize callback
     *
     * @param callback The callback function
     */
    void setResizeCallback(std::function<void(int, int)> callback);

    /**
     * @brief Set the framebuffer resize callback
     *
     * @param callback The callback function
     */
    void setFramebufferResizeCallback(std::function<void(int, int)> callback);

    /**
     * @brief Set the mouse scroll callback
     *
     * @param callback The callback function
     */
    void setMouseScrollCallback(std::function<void(double, double)> callback);

    /**
     * @brief Set the mouse button callback
     *
     * @param callback The callback function
     */
    void setMouseButtonCallback(std::function<void(int, int, int)> callback);

    /**
     * @brief Set the mouse move callback
     *
     * @param callback The callback function
     */
    void setMouseMoveCallback(std::function<void(double, double)> callback);

    /**
     * @brief Set the keyboard callback
     *
     * @param callback The callback function
     */
    void setKeyboardCallback(std::function<void(int, int, int, int)> callback);

private:
    /**
     * @brief Static callback for window resize events
     *
     * @param window The GLFW window
     * @param width The new width
     * @param height The new height
     */
    static void onResizeCallback(GLFWwindow* window, int width, int height);

    /**
     * @brief Static callback for framebuffer resize events
     *
     * @param window The GLFW window
     * @param width The new width
     * @param height The new height
     */
    static void onFBResizeCallback(GLFWwindow* window, int width, int height);

    /**
     * @brief Static callback for mouse scroll events
     *
     * @param window The GLFW window
     * @param offsetX The horizontal scroll offset
     * @param offsetY The vertical scroll offset
     */
    static void onMouseScrollCallback(GLFWwindow* window, double offsetX, double offsetY);

    /**
     * @brief Static callback for mouse button events
     *
     * @param window The GLFW window
     * @param button The mouse button
     * @param action The action (press, release)
     * @param mods Modifier keys
     */
    static void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    /**
     * @brief Static callback for mouse move events
     *
     * @param window The GLFW window
     * @param posX The x-coordinate of the mouse position
     * @param posY The y-coordinate of the mouse position
     */
    static void onMouseMoveCallback(GLFWwindow* window, double posX, double posY);

    /**
     * @brief Static callback for keyboard events
     *
     * @param window The GLFW window
     * @param key The key code
     * @param scancode The scancode
     * @param action The action (press, release, repeat)
     * @param mods Modifier keys
     */
    static void onKeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    /**
     * @brief Get the InputManager instance from a GLFW window
     *
     * @param window The GLFW window
     * @return Pointer to the InputManager instance
     */
    static InputManager* toInputManager(GLFWwindow* window);

    /** The GLFW window */
    GLFWwindow* myWindow;

    /** Resize callback */
    std::function<void(int, int)> myResizeCallback;

    /** Framebuffer resize callback */
    std::function<void(int, int)> myFramebufferResizeCallback;

    /** Mouse scroll callback */
    std::function<void(double, double)> myMouseScrollCallback;

    /** Mouse button callback */
    std::function<void(int, int, int)> myMouseButtonCallback;

    /** Mouse move callback */
    std::function<void(double, double)> myMouseMoveCallback;

    /** Keyboard callback */
    std::function<void(int, int, int, int)> myKeyboardCallback;

    /** Logger */
    std::shared_ptr<Utils::Logger> myLogger;
};