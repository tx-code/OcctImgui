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

// 声明ModelFactory初始化函数
void InitializeModelFactory(ModelFactory& factory);

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
    
    // Initialize manager instances
    myMessageBus = std::make_unique<MVVM::MessageBus>();
    myGlobalSettings = std::make_unique<MVVM::GlobalSettings>();
    myModelFactory = std::make_unique<ModelFactory>();
    myModelManager = std::make_unique<ModelManager>();
    myModelImporter = std::make_unique<ModelImporter>();
    myViewModelManager = std::make_unique<ViewModelManager>(*myModelManager, *myMessageBus, *myGlobalSettings, *myModelImporter);
    myViewManager = std::make_unique<ViewManager>(*myViewModelManager, *myMessageBus);
    
    // Initialize model factory
    InitializeModelFactory(*myModelFactory);
    
    getAppLogger()->info("Manager instances initialized");
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
    myModelId = "MainModel";
    myModel = myModelManager->createModel<UnifiedModel>(myModelId);
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
        myViewModelId = "MainViewModel";
        myViewModel = myViewModelManager->createViewModel<UnifiedViewModel, UnifiedModel>(
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
        
        // 创建ImGui视图
        getAppLogger()->info("App: Creating ImGuiView");
        myImGuiViewId = "ImGuiView";
        myImGuiView = myViewManager->createView<ImGuiView>(myImGuiViewId, myViewModelId);
        // 使用ViewManager初始化视图
        myViewManager->initializeView(myImGuiViewId, myGlfwWindow);

        // 创建OCCT视图 - 使用专门的工厂方法
        getAppLogger()->info("App: Creating OcctView");
        myOcctViewId = "OcctView";
        myOcctView = myViewManager->createOcctView(myOcctViewId, myViewModelId, myWindow);
        if (!myOcctView) {
            getAppLogger()->error("App: Failed to create OcctView");
            throw std::runtime_error("Failed to create OcctView");
        }
        // 使用ViewManager初始化视图
        myViewManager->initializeView(myOcctViewId, myGlfwWindow);

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
    auto occtView = myViewManager->getView<OcctView>(myOcctViewId);
    
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
            myViewManager->renderInOrder(renderOrder);
            
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
    getAppLogger()->info("App: Shutting down all views");
    myViewManager->shutdownAll();
    
    // 清理ViewModelManager
    getAppLogger()->info("App: Removing view models");
    myViewModelManager->removeViewModel(myViewModelId);
    
    // 清理ModelManager
    getAppLogger()->info("App: Removing models");
    myModelManager->removeModel(myModelId);

    if (!myWindow.IsNull()) {
        getAppLogger()->info("App: Closing window");
        myWindow->Close();
    }
    
    // 终止GLFW
    glfwTerminate();
    getAppLogger()->info("App: GLFW terminated");
}

Application* Application::toApplication(GLFWwindow* theWin)
{
    return static_cast<Application*>(glfwGetWindowUserPointer(theWin));
}

void Application::onResizeCallback(GLFWwindow* theWin, int theWidth, int theHeight)
{
    Application* app = toApplication(theWin);
    if (app) {
        app->myViewManager->handleResize(app->myOcctViewId, theWidth, theHeight);
    }
}

void Application::onFBResizeCallback(GLFWwindow* theWin, int theWidth, int theHeight)
{
    Application* app = toApplication(theWin);
    if (app) {
        app->myViewManager->handleResize(app->myOcctViewId, theWidth, theHeight);
    }
}

void Application::onMouseScrollCallback(GLFWwindow* theWin, double theOffsetX, double theOffsetY)
{
    Application* app = toApplication(theWin);
    if (app) {
        app->myViewManager->handleMouseScroll(app->myOcctViewId, theOffsetX, theOffsetY);
    }
}

void Application::onMouseButtonCallback(GLFWwindow* theWin, 
                                       int theButton,
                                       int theAction,
                                       int theMods)
{
    Application* app = toApplication(theWin);
    if (app) {
        app->myViewManager->handleMouseButton(app->myOcctViewId, theButton, theAction, theMods);
    }
}

void Application::onMouseMoveCallback(GLFWwindow* theWin, double thePosX, double thePosY)
{
    Application* app = toApplication(theWin);
    if (app) {
        app->myViewManager->handleMouseMove(app->myOcctViewId, thePosX, thePosY);
    }
}

void Application::errorCallback(int theError, const char* theDescription)
{
    getAppLogger()->error("App: GLFW error {}: {}", theError, theDescription);
}

bool Application::importModel(const std::string& filePath, const std::string& modelId)
{
    LOG_FUNCTION_SCOPE(getAppLogger(), "importModel");
    getAppLogger()->info("Importing model from '{}'", filePath);
    
    // 使用 UnifiedViewModel 的 importModel 方法
    auto viewModel = myViewModelManager->getViewModel<UnifiedViewModel>(myViewModelId);
    if (!viewModel) {
        getAppLogger()->error("Failed to get UnifiedViewModel");
        return false;
    }
    
    bool result = viewModel->importModel(filePath, modelId);
    
    if (result) {
        getAppLogger()->info("Model imported successfully");
    } else {
        getAppLogger()->error("Failed to import model");
    }
    
    return result;
}