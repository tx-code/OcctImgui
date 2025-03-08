#include "WindowManager.h"
#include "../utils/Logger.h"
#include <GLFW/glfw3.h>
#include <stdexcept>

// 使用宏声明 WindowManager 类的 logger
DECLARE_LOGGER(WindowManager)

WindowManager::WindowManager(int width, int height, const std::string& title)
    : myWidth(width)
    , myHeight(height)
    , myTitle(title)
    , myGlfwWindow(nullptr)
    , myLogger(getWindowManagerLogger())
{
    myLogger->info("WindowManager created with dimensions {}x{} and title '{}'",
                   width,
                   height,
                   title);
}

WindowManager::~WindowManager()
{
    close();
}

bool WindowManager::initialize()
{
    myLogger->info("Initializing window");

    // Set GLFW error callback
    glfwSetErrorCallback(WindowManager::errorCallback);

    // Initialize GLFW if not already initialized
    if (!glfwInit()) {
        myLogger->error("Failed to initialize GLFW");
        return false;
    }
    myLogger->info("GLFW initialized");

    // Set OpenGL context hints
    setContextHints();

    try {
        // Create OCCT window
        myWindow = new GlfwOcctWindow(myWidth, myHeight, myTitle.c_str());
        myGlfwWindow = myWindow->getGlfwWindow();

        if (myGlfwWindow == nullptr) {
            myLogger->error("Failed to create GLFW window");
            return false;
        }

        // Make the window's context current
        glfwMakeContextCurrent(myGlfwWindow);
        myLogger->info("GLFW window created and set as current context");

        // Log GLFW version
        logGlfwVersion();

        return true;
    }
    catch (const std::exception& e) {
        myLogger->error("Window initialization exception: {}", e.what());
        return false;
    }
    catch (...) {
        myLogger->error("Unknown exception during window initialization");
        return false;
    }
}

bool WindowManager::shouldClose() const
{
    return myGlfwWindow == nullptr || glfwWindowShouldClose(myGlfwWindow);
}

void WindowManager::pollEvents() const
{
    glfwPollEvents();
}

void WindowManager::swapBuffers() const
{
    if (myGlfwWindow) {
        glfwSwapBuffers(myGlfwWindow);
    }
}

void WindowManager::close()
{
    if (!myWindow.IsNull()) {
        myLogger->info("Closing window");
        myWindow->Close();
        myWindow.Nullify();
        myGlfwWindow = nullptr;
    }

    // Terminate GLFW if it was initialized
    if (glfwGetCurrentContext() != nullptr) {
        glfwTerminate();
        myLogger->info("GLFW terminated");
    }
}

const Handle(GlfwOcctWindow) & WindowManager::getOcctWindow() const
{
    return myWindow;
}

void WindowManager::setContextHints()
{
    // Set OpenGL context hints
    constexpr bool toAskCoreProfile = true;
    if constexpr (toAskCoreProfile) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#if defined(__APPLE__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }
    myLogger->info("Window hints set - OpenGL 3.3 Core Profile");
}

void WindowManager::logGlfwVersion()
{
    int major, minor, revision;
    glfwGetVersion(&major, &minor, &revision);
    myLogger->info("GLFW version: {}.{}.{}", major, minor, revision);
}

void WindowManager::errorCallback(int error, const char* description)
{
    getWindowManagerLogger()->error("GLFW error {}: {}", error, description);
}
