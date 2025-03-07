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
#include "view/ViewManager.h"
#include "viewmodel/UnifiedViewModel.h"
#include "viewmodel/ViewModelManager.h"
#include <stdexcept>


// 使用宏声明 Application 类的 logger
DECLARE_LOGGER(Application)

// 声明ModelFactory初始化函数
void InitializeModelFactory(ModelFactory& factory);

Application::Application()
    : myIsRunning(false)
    , myLogger(getApplicationLogger())
{
    myLogger->info("Application instance created");
    myBootstrapper = std::make_unique<ApplicationBootstrapper>();
}

Application::~Application()
{
    cleanup();
}

void Application::run()
{
    LOG_FUNCTION_SCOPE(myLogger, "run");

    myLogger->info("Starting application");

    // Initialize application components
    if (!myBootstrapper->initialize()) {
        myLogger->error("Failed to initialize application components");
        throw std::runtime_error("Failed to initialize application components");
    }

    // Enter main loop
    myIsRunning = true;
    mainloop();
}

void Application::mainloop()
{
    myLogger->info("Starting main loop");

    auto& windowManager = myBootstrapper->getWindowManager();
    auto& viewManager = myBootstrapper->getViewManager();

    // Define view render order
    std::vector<std::string> renderOrder = {"OcctView", "ImGuiView"};

    // Main loop
    while (myIsRunning && !windowManager.shouldClose()) {
        // Poll window events
        windowManager.pollEvents();

        try {
            // Render views in order
            viewManager.renderInOrder(renderOrder);

            // Swap buffers
            windowManager.swapBuffers();
        }
        catch (const std::exception& e) {
            myLogger->error("Main loop exception: {}", e.what());
        }
        catch (...) {
            myLogger->error("Unknown exception in main loop");
        }
    }

    myLogger->info("Main loop ended");
}

void Application::cleanup()
{
    myLogger->info("Starting cleanup");

    // Stop main loop
    myIsRunning = false;

    // Clean up bootstrapper (will clean up all components)
    myBootstrapper.reset();

    myLogger->info("Application resources cleaned up");
}