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

// Include manager headers
#include "model/ModelManager.h"
#include "viewmodel/ViewModelManager.h"
#include "view/ViewManager.h"

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
    // 使用ModelManager创建统一模型
    auto& modelManager = ModelManager::instance();
    myModelId = "MainModel";
    myModel = modelManager.createModel<UnifiedModel>(myModelId);
    spdlog::info("App: Model initialization complete with ID: {}", myModelId);
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

        // 使用ViewModelManager创建统一视图模型
        auto& viewModelManager = ViewModelManager::instance();
        myViewModelId = "MainViewModel";
        myViewModel = viewModelManager.createViewModel<UnifiedViewModel, UnifiedModel>(
            myViewModelId, myModelId, aContext);
        spdlog::info("App: View model initialization complete with ID: {}", myViewModelId);
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
        
        // 使用ViewManager创建视图
        auto& viewManager = ViewManager::instance();
        
        // 创建ImGui视图
        spdlog::info("App: Creating ImGuiView");
        myImGuiViewId = "ImGuiView";
        myImGuiView = viewManager.createView<ImGuiView>(myImGuiViewId, myViewModelId);
        // 使用ViewManager初始化视图
        viewManager.initializeView(myImGuiViewId, myGlfwWindow);

        // 创建OCCT视图 - 使用专门的工厂方法
        spdlog::info("App: Creating OcctView");
        myOcctViewId = "OcctView";
        myOcctView = viewManager.createOcctView(myOcctViewId, myViewModelId, myWindow);
        if (!myOcctView) {
            spdlog::error("App: Failed to create OcctView");
            throw std::runtime_error("Failed to create OcctView");
        }
        // 使用ViewManager初始化视图
        viewManager.initializeView(myOcctViewId, myGlfwWindow);

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
    auto& viewManager = ViewManager::instance();
    auto occtView = viewManager.getView<OcctView>(myOcctViewId);
    
    // 定义视图渲染顺序
    std::vector<std::string> renderOrder = {myOcctViewId, myImGuiViewId};
    
    // 主循环
    while (!glfwWindowShouldClose(myGlfwWindow)) {
        if (occtView && occtView->toWaitEvents()) {
            glfwWaitEvents();
        }
        else {
            glfwPollEvents();
        }

        try {
            // 按照指定的顺序渲染视图
            viewManager.renderInOrder(renderOrder);
            
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
    
    // 使用ViewManager清理视图
    auto& viewManager = ViewManager::instance();
    spdlog::info("App: Shutting down all views");
    viewManager.shutdownAll();
    
    // 清理ViewModelManager
    auto& viewModelManager = ViewModelManager::instance();
    spdlog::info("App: Removing view models");
    viewModelManager.removeViewModel(myViewModelId);
    
    // 清理ModelManager
    auto& modelManager = ModelManager::instance();
    spdlog::info("App: Removing models");
    modelManager.removeModel(myModelId);

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
    if (app) {
        auto& viewManager = ViewManager::instance();
        viewManager.handleResize(app->myOcctViewId, theWidth, theHeight);
    }
}

void Application::onFBResizeCallback(GLFWwindow* theWin, int theWidth, int theHeight)
{
    Application* app = toApplication(theWin);
    if (app) {
        auto& viewManager = ViewManager::instance();
        viewManager.handleResize(app->myOcctViewId, theWidth, theHeight);
    }
}

void Application::onMouseScrollCallback(GLFWwindow* theWin, double theOffsetX, double theOffsetY)
{
    Application* app = toApplication(theWin);
    if (app) {
        auto& viewManager = ViewManager::instance();
        viewManager.handleMouseScroll(app->myOcctViewId, theOffsetX, theOffsetY);
    }
}

void Application::onMouseButtonCallback(GLFWwindow* theWin, 
                                       int theButton,
                                       int theAction,
                                       int theMods)
{
    Application* app = toApplication(theWin);
    if (app) {
        auto& viewManager = ViewManager::instance();
        viewManager.handleMouseButton(app->myOcctViewId, theButton, theAction, theMods);
    }
}

void Application::onMouseMoveCallback(GLFWwindow* theWin, double thePosX, double thePosY)
{
    Application* app = toApplication(theWin);
    if (app) {
        auto& viewManager = ViewManager::instance();
        viewManager.handleMouseMove(app->myOcctViewId, thePosX, thePosY);
    }
}

void Application::errorCallback(int theError, const char* theDescription)
{
    spdlog::error("App: GLFW error {}: {}", theError, theDescription);
}