#include "InputManager.h"
#include "../utils/Logger.h"
#include <GLFW/glfw3.h>

// 使用宏声明 InputManager 类的 logger
DECLARE_LOGGER(InputManager)

InputManager::InputManager()
    : myWindow(nullptr)
    , myLogger(getInputManagerLogger())
{
    myLogger->info("InputManager created");
}

InputManager::~InputManager()
{
    myLogger->info("InputManager destroyed");
}

bool InputManager::initialize(GLFWwindow* window)
{
    if (!window) {
        myLogger->error("Cannot initialize InputManager with null window");
        return false;
    }

    myWindow = window;

    // Store this instance in the window user pointer
    // Note: This might overwrite any existing user pointer, so we need to be careful
    // about how we manage this in a real application
    glfwSetWindowUserPointer(myWindow, this);

    // Set up callbacks
    glfwSetWindowSizeCallback(myWindow, InputManager::onResizeCallback);
    glfwSetFramebufferSizeCallback(myWindow, InputManager::onFBResizeCallback);
    glfwSetScrollCallback(myWindow, InputManager::onMouseScrollCallback);
    glfwSetMouseButtonCallback(myWindow, InputManager::onMouseButtonCallback);
    glfwSetCursorPosCallback(myWindow, InputManager::onMouseMoveCallback);
    glfwSetKeyCallback(myWindow, InputManager::onKeyboardCallback);

    myLogger->info("InputManager initialized with window");
    return true;
}

void InputManager::setResizeCallback(std::function<void(int, int)> callback)
{
    myResizeCallback = callback;
}

void InputManager::setFramebufferResizeCallback(std::function<void(int, int)> callback)
{
    myFramebufferResizeCallback = callback;
}

void InputManager::setMouseScrollCallback(std::function<void(double, double)> callback)
{
    myMouseScrollCallback = callback;
}

void InputManager::setMouseButtonCallback(std::function<void(int, int, int)> callback)
{
    myMouseButtonCallback = callback;
}

void InputManager::setMouseMoveCallback(std::function<void(double, double)> callback)
{
    myMouseMoveCallback = callback;
}

void InputManager::setKeyboardCallback(std::function<void(int, int, int, int)> callback)
{
    myKeyboardCallback = callback;
}

void InputManager::onResizeCallback(GLFWwindow* window, int width, int height)
{
    InputManager* inputManager = toInputManager(window);
    if (inputManager && inputManager->myResizeCallback) {
        inputManager->myLogger->debug("Window resized to {}x{}", width, height);
        inputManager->myResizeCallback(width, height);
    }
}

void InputManager::onFBResizeCallback(GLFWwindow* window, int width, int height)
{
    InputManager* inputManager = toInputManager(window);
    if (inputManager && inputManager->myFramebufferResizeCallback) {
        inputManager->myLogger->debug("Framebuffer resized to {}x{}", width, height);
        inputManager->myFramebufferResizeCallback(width, height);
    }
}

void InputManager::onMouseScrollCallback(GLFWwindow* window, double offsetX, double offsetY)
{
    InputManager* inputManager = toInputManager(window);
    if (inputManager && inputManager->myMouseScrollCallback) {
        inputManager->myLogger->debug("Mouse scroll: ({}, {})", offsetX, offsetY);
        inputManager->myMouseScrollCallback(offsetX, offsetY);
    }
}

void InputManager::onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    InputManager* inputManager = toInputManager(window);
    if (inputManager && inputManager->myMouseButtonCallback) {
        inputManager->myLogger->debug("Mouse button: button={}, action={}, mods={}",
                                      button,
                                      action,
                                      mods);
        inputManager->myMouseButtonCallback(button, action, mods);
    }
}

void InputManager::onMouseMoveCallback(GLFWwindow* window, double posX, double posY)
{
    InputManager* inputManager = toInputManager(window);
    if (inputManager && inputManager->myMouseMoveCallback) {
        // Logging mouse move events can be very verbose, so we'll skip it
        inputManager->myMouseMoveCallback(posX, posY);
    }
}

void InputManager::onKeyboardCallback(GLFWwindow* window,
                                      int key,
                                      int scancode,
                                      int action,
                                      int mods)
{
    InputManager* inputManager = toInputManager(window);
    if (inputManager && inputManager->myKeyboardCallback) {
        inputManager->myLogger->debug("Keyboard: key={}, scancode={}, action={}, mods={}",
                                      key,
                                      scancode,
                                      action,
                                      mods);
        inputManager->myKeyboardCallback(key, scancode, action, mods);
    }
}

InputManager* InputManager::toInputManager(GLFWwindow* window)
{
    return static_cast<InputManager*>(glfwGetWindowUserPointer(window));
}