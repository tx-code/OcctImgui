#include "Application.h"
#include <AIS_InteractiveContext.hxx>
#include <GLFW/glfw3.h>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

// 添加日志头文件
#include "utils/Logger.h"
#include <spdlog/spdlog.h>

// Include manager headers
#include "model/ModelManager.h"
#include "viewmodel/ViewModelManager.h"
#include "view/ViewManager.h"

// 创建应用程序日志记录器 - 使用函数确保安全初始化
std::shared_ptr<Utils::Logger>& getAppLogger() {
    static std::shared_ptr<Utils::Logger> logger = Utils::Logger::getLogger("app");
    return logger;
}

Application::Application()
    : myWidth(800)
    , myHeight(600)
    , myTitle("OCCT MVVM")
{
    getAppLogger()->info("Application instance created");
}

Application::~Application()
{
    cleanup();
}

void Application::run()
{
    LOG_FUNCTION_SCOPE(getAppLogger(), "run");
    
    getAppLogger()->info("Starting application");
    glfwSetErrorCallback(Application::errorCallback);
    if (!glfwInit()) {
        getAppLogger()->error("Failed to initialize GLFW");
        throw std::runtime_error("Failed to initialize GLFW");
    }
    getAppLogger()->info("GLFW initialized");

    initWindow();
    initModel();
    initViewModel();
    initViews();
    mainloop();
}

void Application::initWindow()
{
    LOG_FUNCTION_SCOPE(getAppLogger(), "initWindow");
    getAppLogger()->info("Initializing window");
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
    getAppLogger()->info("App: Window hints set - OpenGL 3.3 Core Profile");

    // 创建窗口
    try {
        myWindow = new GlfwOcctWindow(myWidth, myHeight, myTitle);
        myGlfwWindow = myWindow->getGlfwWindow();
        if (myGlfwWindow == nullptr) {
            getAppLogger()->error("App: Failed to create GLFW window");
            throw std::runtime_error("Failed to create GLFW window");
        }
        
        // 确保窗口的OpenGL上下文是当前上下文
        glfwMakeContextCurrent(myGlfwWindow);
        getAppLogger()->info("App: GLFW window created and set as current context");
        
        glfwSetWindowUserPointer(myGlfwWindow, this);
        
        // 输出GLFW版本信息
        int major, minor, revision;
        glfwGetVersion(&major, &minor, &revision);
        getAppLogger()->info("App: GLFW version: {}.{}.{}", major, minor, revision);
        
        // 设置回调
        glfwSetWindowSizeCallback(myGlfwWindow, Application::onResizeCallback);
        glfwSetFramebufferSizeCallback(myGlfwWindow, Application::onFBResizeCallback);
        glfwSetScrollCallback(myGlfwWindow, Application::onMouseScrollCallback);
        glfwSetMouseButtonCallback(myGlfwWindow, Application::onMouseButtonCallback);
        glfwSetCursorPosCallback(myGlfwWindow, Application::onMouseMoveCallback);
        getAppLogger()->info("App: GLFW callbacks set");
    } catch (const std::exception& e) {
        getAppLogger()->error("App: Window initialization exception: {}", e.what());
        throw;
    } catch (...) {
        getAppLogger()->error("App: Unknown exception during window initialization");
        throw std::runtime_error("Unknown error during window initialization");
    }
}

void Application::initModel()
{
    getAppLogger()->info("App: Initializing model");
    // 使用ModelManager创建统一模型
    auto& modelManager = ModelManager::instance();
    myModelId = "MainModel";
    myModel = modelManager.createModel<UnifiedModel>(myModelId);
    getAppLogger()->info("App: Model initialization complete with ID: {}", myModelId);
}

void Application::initViewModel()
{
    getAppLogger()->info("App: Initializing view model");
    try {
        // 创建交互上下文
        Handle(OpenGl_GraphicDriver) aGraphicDriver =
            new OpenGl_GraphicDriver(myWindow->GetDisplay(), Standard_False);
        aGraphicDriver->SetBuffersNoSwap(Standard_True);
        getAppLogger()->info("App: OpenGL graphic driver created, BuffersNoSwap=True");

        Handle(V3d_Viewer) aViewer = new V3d_Viewer(aGraphicDriver);
        aViewer->SetDefaultLights();
        aViewer->SetLightOn();
        getAppLogger()->info("App: V3d_Viewer created");

        Handle(AIS_InteractiveContext) aContext = new AIS_InteractiveContext(aViewer);
        getAppLogger()->info("App: AIS_InteractiveContext created");

        // 使用ViewModelManager创建统一视图模型
        auto& viewModelManager = ViewModelManager::instance();
        myViewModelId = "MainViewModel";
        myViewModel = viewModelManager.createViewModel<UnifiedViewModel, UnifiedModel>(
            myViewModelId, myModelId, aContext);
        getAppLogger()->info("App: View model initialization complete with ID: {}", myViewModelId);
    } catch (const std::exception& e) {
        getAppLogger()->error("App: View model initialization exception: {}", e.what());
        throw;
    } catch (...) {
        getAppLogger()->error("App: Unknown exception during view model initialization");
        throw std::runtime_error("Unknown error during view model initialization");
    }
}

void Application::initViews()
{
    getAppLogger()->info("App: Initializing views");
    try {
        // 确保当前上下文是GLFW窗口
        GLFWwindow* currentContext = glfwGetCurrentContext();
        if (currentContext != myGlfwWindow) {
            getAppLogger()->warn("App: Current context is not application window, resetting context");
            glfwMakeContextCurrent(myGlfwWindow);
        }
        
        // 使用ViewManager创建视图
        auto& viewManager = ViewManager::instance();
        
        // 创建ImGui视图
        getAppLogger()->info("App: Creating ImGuiView");
        myImGuiViewId = "ImGuiView";
        myImGuiView = viewManager.createView<ImGuiView>(myImGuiViewId, myViewModelId);
        // 使用ViewManager初始化视图
        viewManager.initializeView(myImGuiViewId, myGlfwWindow);

        // 创建OCCT视图 - 使用专门的工厂方法
        getAppLogger()->info("App: Creating OcctView");
        myOcctViewId = "OcctView";
        myOcctView = viewManager.createOcctView(myOcctViewId, myViewModelId, myWindow);
        if (!myOcctView) {
            getAppLogger()->error("App: Failed to create OcctView");
            throw std::runtime_error("Failed to create OcctView");
        }
        // 使用ViewManager初始化视图
        viewManager.initializeView(myOcctViewId, myGlfwWindow);

        myOcctView->getView()->MustBeResized();
        myWindow->Map();
        getAppLogger()->info("App: Views initialization complete");
    } catch (const std::exception& e) {
        getAppLogger()->error("App: Views initialization exception: {}", e.what());
        throw;
    } catch (...) {
        getAppLogger()->error("App: Unknown exception during views initialization");
        throw std::runtime_error("Unknown error during view initialization");
    }
}

void Application::mainloop()
{
    getAppLogger()->info("App: Starting main loop");
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
            getAppLogger()->error("App: Main loop exception: {}", e.what());
        } catch (...) {
            getAppLogger()->error("App: Unknown exception in main loop");
        }
    }
    getAppLogger()->info("App: Main loop ended");
}

void Application::cleanup()
{
    getAppLogger()->info("App: Starting cleanup");
    
    // 使用ViewManager清理视图
    auto& viewManager = ViewManager::instance();
    getAppLogger()->info("App: Shutting down all views");
    viewManager.shutdownAll();
    
    // 清理ViewModelManager
    auto& viewModelManager = ViewModelManager::instance();
    getAppLogger()->info("App: Removing view models");
    viewModelManager.removeViewModel(myViewModelId);
    
    // 清理ModelManager
    auto& modelManager = ModelManager::instance();
    getAppLogger()->info("App: Removing models");
    modelManager.removeModel(myModelId);

    if (!myWindow.IsNull()) {
        getAppLogger()->info("App: Closing window");
        myWindow->Close();
    }

    getAppLogger()->info("App: Terminating GLFW");
    glfwTerminate();
    getAppLogger()->info("App: Cleanup complete");
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
    getAppLogger()->error("App: GLFW error {}: {}", theError, theDescription);
}