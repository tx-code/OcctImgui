#include "Application.h"
#include <AIS_InteractiveContext.hxx>
#include <GLFW/glfw3.h>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

// 添加spdlog头文件
#include <spdlog/spdlog.h>

Application::Application()
    : myWidth(800)
    , myHeight(600)
    , myTitle("OCCT MVVM")
{
}

Application::~Application()
{
    cleanup();
}

void Application::run()
{
    spdlog::info("App: Starting application");
    glfwSetErrorCallback(Application::errorCallback);
    if (!glfwInit()) {
        spdlog::error("App: Failed to initialize GLFW");
        throw std::runtime_error("Failed to initialize GLFW");
    }
    spdlog::info("App: GLFW initialized");

    initWindow();
    initModel();
    initViewModel();
    initViews();
    mainloop();
}

void Application::initWindow()
{
    spdlog::info("App: Initializing window");
    // 设置OpenGL上下文
    const bool toAskCoreProfile = true;
    if (toAskCoreProfile) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#if defined(__APPLE__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }
    spdlog::info("App: Window hints set - OpenGL 3.3 Core Profile");

    // 创建窗口
    try {
        myWindow = new GlfwOcctWindow(myWidth, myHeight, myTitle);
        myGlfwWindow = myWindow->getGlfwWindow();
        if (myGlfwWindow == nullptr) {
            spdlog::error("App: Failed to create GLFW window");
            throw std::runtime_error("Failed to create GLFW window");
        }
        
        // 确保窗口的OpenGL上下文是当前上下文
        glfwMakeContextCurrent(myGlfwWindow);
        spdlog::info("App: GLFW window created and set as current context");
        
        glfwSetWindowUserPointer(myGlfwWindow, this);
        
        // 输出GLFW版本信息
        int major, minor, revision;
        glfwGetVersion(&major, &minor, &revision);
        spdlog::info("App: GLFW version: {}.{}.{}", major, minor, revision);
        
        // 设置回调
        glfwSetWindowSizeCallback(myGlfwWindow, Application::onResizeCallback);
        glfwSetFramebufferSizeCallback(myGlfwWindow, Application::onFBResizeCallback);
        glfwSetScrollCallback(myGlfwWindow, Application::onMouseScrollCallback);
        glfwSetMouseButtonCallback(myGlfwWindow, Application::onMouseButtonCallback);
        glfwSetCursorPosCallback(myGlfwWindow, Application::onMouseMoveCallback);
        spdlog::info("App: GLFW callbacks set");
    } catch (const std::exception& e) {
        spdlog::error("App: Window initialization exception: {}", e.what());
        throw;
    } catch (...) {
        spdlog::error("App: Unknown exception during window initialization");
        throw std::runtime_error("Unknown error during window initialization");
    }
}

void Application::initModel()
{
    spdlog::info("App: Initializing model");
    // 创建模型
    myModel = std::make_shared<CadModel>();
    spdlog::info("App: Model initialization complete");
}

void Application::initViewModel()
{
    spdlog::info("App: Initializing view model");
    try {
        // 创建交互上下文
        Handle(OpenGl_GraphicDriver) aGraphicDriver =
            new OpenGl_GraphicDriver(myWindow->GetDisplay(), Standard_False);
        aGraphicDriver->SetBuffersNoSwap(Standard_True);
        spdlog::info("App: OpenGL graphic driver created, BuffersNoSwap=True");

        Handle(V3d_Viewer) aViewer = new V3d_Viewer(aGraphicDriver);
        aViewer->SetDefaultLights();
        aViewer->SetLightOn();
        spdlog::info("App: V3d_Viewer created");

        Handle(AIS_InteractiveContext) aContext = new AIS_InteractiveContext(aViewer);
        spdlog::info("App: AIS_InteractiveContext created");

        // 创建视图模型
        myViewModel = std::make_shared<CadViewModel>(myModel, aContext);
        spdlog::info("App: View model initialization complete");
    } catch (const std::exception& e) {
        spdlog::error("App: View model initialization exception: {}", e.what());
        throw;
    } catch (...) {
        spdlog::error("App: Unknown exception during view model initialization");
        throw std::runtime_error("Unknown error during view model initialization");
    }
}

void Application::initViews()
{
    spdlog::info("App: Initializing views");
    try {
        // 确保当前上下文是GLFW窗口
        GLFWwindow* currentContext = glfwGetCurrentContext();
        if (currentContext != myGlfwWindow) {
            spdlog::warn("App: Current context is not application window, resetting context");
            glfwMakeContextCurrent(myGlfwWindow);
        }
        
        spdlog::info("App: Creating ImGuiView");
        // 创建ImGui视图
        myImGuiView = std::make_unique<ImGuiView>(myViewModel);
        spdlog::info("App: Initializing ImGuiView");
        myImGuiView->initialize(myGlfwWindow);

        spdlog::info("App: Creating OcctView");
        // 创建OCCT视图
        myOcctView = std::make_unique<OcctView>(myViewModel, myWindow);
        spdlog::info("App: Initializing OcctView");
        myOcctView->initialize();

        myOcctView->getView()->MustBeResized();
        myWindow->Map();
        spdlog::info("App: Views initialization complete");
    } catch (const std::exception& e) {
        spdlog::error("App: Views initialization exception: {}", e.what());
        throw;
    } catch (...) {
        spdlog::error("App: Unknown exception during views initialization");
        throw std::runtime_error("Unknown error during view initialization");
    }
}

void Application::mainloop()
{
    spdlog::info("App: Starting main loop");
    // 主循环
    while (!glfwWindowShouldClose(myGlfwWindow)) {
        if (myOcctView->toWaitEvents()) {
            glfwWaitEvents();
        }
        else {
            glfwPollEvents();
        }

        try {
            // 渲染3D视图
            myOcctView->render();
            
            // 渲染ImGui UI
            myImGuiView->render();
            
            glfwSwapBuffers(myGlfwWindow);
        } catch (const std::exception& e) {
            spdlog::error("App: Main loop exception: {}", e.what());
        } catch (...) {
            spdlog::error("App: Unknown exception in main loop");
        }
    }
    spdlog::info("App: Main loop ended");
}

void Application::cleanup()
{
    spdlog::info("App: Starting cleanup");
    // 清理资源
    if (myImGuiView) {
        spdlog::info("App: Shutting down ImGuiView");
        myImGuiView->shutdown();
    }

    // 先释放OCCT视图，确保在窗口关闭前释放OpenGL资源
    if (myOcctView) {
        myOcctView->cleanup();
    }

    if (!myWindow.IsNull()) {
        spdlog::info("App: Closing window");
        myWindow->Close();
    }

    spdlog::info("App: Terminating GLFW");
    glfwTerminate();
    spdlog::info("App: Cleanup complete");
}

Application* Application::toApplication(GLFWwindow* theWin)
{
    return static_cast<Application*>(glfwGetWindowUserPointer(theWin));
}

void Application::onResizeCallback(GLFWwindow* theWin, int theWidth, int theHeight)
{
    Application* app = toApplication(theWin);
    if (app && app->myOcctView) {
        app->myOcctView->onResize(theWidth, theHeight);
    }
}

void Application::onFBResizeCallback(GLFWwindow* theWin, int theWidth, int theHeight)
{
    Application* app = toApplication(theWin);
    if (app && app->myOcctView) {
        app->myOcctView->onResize(theWidth, theHeight);
    }
}

void Application::onMouseScrollCallback(GLFWwindow* theWin, double theOffsetX, double theOffsetY)
{
    Application* app = toApplication(theWin);
    if (app && app->myOcctView && !app->myImGuiView->wantCaptureMouse()) {
        app->myOcctView->onMouseScroll(theOffsetX, theOffsetY);
    }
}

void Application::onMouseButtonCallback(GLFWwindow* theWin,
                                        int theButton,
                                        int theAction,
                                        int theMods)
{
    Application* app = toApplication(theWin);
    if (app && app->myOcctView && !app->myImGuiView->wantCaptureMouse()) {
        app->myOcctView->onMouseButton(theButton, theAction, theMods);
    }
}

void Application::onMouseMoveCallback(GLFWwindow* theWin, double thePosX, double thePosY)
{
    Application* app = toApplication(theWin);
    if (app && app->myOcctView && !app->myImGuiView->wantCaptureMouse()) {
        app->myOcctView->onMouseMove((int)thePosX, (int)thePosY);
    }
}

void Application::errorCallback(int theError, const char* theDescription)
{
    spdlog::error("App: {}", theDescription);
}