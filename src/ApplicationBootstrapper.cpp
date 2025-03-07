#include "ApplicationBootstrapper.h"
#include "model/UnifiedModel.h"
#include "utils/Logger.h"
#include "view/ImGuiView.h"
#include "view/OcctView.h"
#include "viewmodel/UnifiedViewModel.h"


#include <AIS_InteractiveContext.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_Viewer.hxx>

// 使用宏声明 ApplicationBootstrapper 类的 logger
DECLARE_LOGGER(ApplicationBootstrapper)

// Declare ModelFactory initialization function
void InitializeModelFactory(ModelFactory& factory);

ApplicationBootstrapper::ApplicationBootstrapper()
    : myModelId("MainModel")
    , myViewModelId("MainViewModel")
    , myImGuiViewId("ImGuiView")
    , myOcctViewId("OcctView")
    , myLogger(getApplicationBootstrapperLogger())
{
    myLogger->info("ApplicationBootstrapper created");

    // Initialize manager instances
    myMessageBus = std::make_unique<MVVM::MessageBus>();
    myGlobalSettings = std::make_unique<MVVM::GlobalSettings>();
    mySelectionManager = std::make_unique<MVVM::SelectionManager>(*myMessageBus);
    myModelFactory = std::make_unique<ModelFactory>();
    myModelManager = std::make_unique<ModelManager>();
    myModelImporter = std::make_unique<ModelImporter>();
    myViewModelManager = std::make_unique<ViewModelManager>(*myModelManager,
                                                            *myMessageBus,
                                                            *myGlobalSettings,
                                                            *myModelImporter);
    myViewManager = std::make_unique<ViewManager>(*myViewModelManager, *myMessageBus);

    // Initialize model factory
    InitializeModelFactory(*myModelFactory);

    // Create window and input managers
    myWindowManager = std::make_unique<WindowManager>();
    myInputManager = std::make_unique<InputManager>();

    myLogger->info("Manager instances initialized");
}

ApplicationBootstrapper::~ApplicationBootstrapper()
{
    myLogger->info("ApplicationBootstrapper destroyed");
}

bool ApplicationBootstrapper::initialize()
{
    myLogger->info("Initializing application components");

    // Initialize components in the correct order
    if (!initializeWindow()) {
        myLogger->error("Failed to initialize window");
        return false;
    }

    if (!initializeInput()) {
        myLogger->error("Failed to initialize input manager");
        return false;
    }

    if (!initializeModel()) {
        myLogger->error("Failed to initialize model");
        return false;
    }

    if (!initializeViewModel()) {
        myLogger->error("Failed to initialize viewmodel");
        return false;
    }

    if (!initializeViews()) {
        myLogger->error("Failed to initialize views");
        return false;
    }

    myLogger->info("Application components initialized successfully");
    return true;
}

bool ApplicationBootstrapper::initializeWindow()
{
    myLogger->info("Initializing window");

    if (!myWindowManager->initialize()) {
        myLogger->error("Failed to initialize window manager");
        return false;
    }

    return true;
}

bool ApplicationBootstrapper::initializeInput()
{
    myLogger->info("Initializing input manager");

    if (!myInputManager->initialize(myWindowManager->getOcctWindow()->getGlfwWindow())) {
        myLogger->error("Failed to initialize input manager");
        return false;
    }

    // Set up input callbacks to forward to the view manager
    myInputManager->setResizeCallback([this](int width, int height) {
        myViewManager->handleResize(myOcctViewId, width, height);
    });

    myInputManager->setFramebufferResizeCallback([this](int width, int height) {
        myViewManager->handleResize(myOcctViewId, width, height);
    });

    myInputManager->setMouseScrollCallback([this](double offsetX, double offsetY) {
        myViewManager->handleMouseScroll(myOcctViewId, offsetX, offsetY);
    });

    myInputManager->setMouseButtonCallback([this](int button, int action, int mods) {
        myViewManager->handleMouseButton(myOcctViewId, button, action, mods);
    });

    myInputManager->setMouseMoveCallback([this](double posX, double posY) {
        myViewManager->handleMouseMove(myOcctViewId, posX, posY);
    });

    return true;
}

bool ApplicationBootstrapper::initializeModel()
{
    myLogger->info("Initializing model");

    try {
        // Create the main model
        auto model = myModelManager->createModel<UnifiedModel>(myModelId);
        if (!model) {
            myLogger->error("Failed to create model");
            return false;
        }

        myLogger->info("Model initialized with ID: {}", myModelId);
        return true;
    }
    catch (const std::exception& e) {
        myLogger->error("Exception during model initialization: {}", e.what());
        return false;
    }
    catch (...) {
        myLogger->error("Unknown exception during model initialization");
        return false;
    }
}

bool ApplicationBootstrapper::initializeViewModel()
{
    myLogger->info("Initializing viewmodel");

    try {
        // Create OpenGL graphic driver
        Handle(OpenGl_GraphicDriver) aGraphicDriver =
            new OpenGl_GraphicDriver(myWindowManager->getOcctWindow()->GetDisplay(),
                                     Standard_False);
        aGraphicDriver->SetBuffersNoSwap(Standard_True);
        myLogger->info("OpenGL graphic driver created, BuffersNoSwap=True");

        // Create V3d_Viewer
        Handle(V3d_Viewer) aViewer = new V3d_Viewer(aGraphicDriver);
        aViewer->SetDefaultLights();
        aViewer->SetLightOn();
        myLogger->info("V3d_Viewer created");

        // Create AIS_InteractiveContext
        Handle(AIS_InteractiveContext) aContext = new AIS_InteractiveContext(aViewer);
        myLogger->info("AIS_InteractiveContext created");

        // Create the main viewmodel
        auto viewModel =
            myViewModelManager->createViewModel<UnifiedViewModel, UnifiedModel>(myViewModelId,
                                                                                myModelId,
                                                                                aContext);

        if (!viewModel) {
            myLogger->error("Failed to create viewmodel");
            return false;
        }

        myLogger->info("Viewmodel initialized with ID: {}", myViewModelId);
        return true;
    }
    catch (const std::exception& e) {
        myLogger->error("Exception during viewmodel initialization: {}", e.what());
        return false;
    }
    catch (...) {
        myLogger->error("Unknown exception during viewmodel initialization");
        return false;
    }
}

bool ApplicationBootstrapper::initializeViews()
{
    myLogger->info("Initializing views");

    try {
        // Create ImGui view
        myLogger->info("Creating ImGuiView");
        auto imguiView = myViewManager->createView<ImGuiView>(myImGuiViewId, myViewModelId);
        if (!imguiView) {
            myLogger->error("Failed to create ImGuiView");
            return false;
        }

        // Initialize ImGui view
        myViewManager->initializeView(myImGuiViewId, myWindowManager->getOcctWindow()->getGlfwWindow());

        // Create OCCT view
        myLogger->info("Creating OcctView");
        auto occtView = myViewManager->createOcctView(myOcctViewId,
                                                      myViewModelId,
                                                      myWindowManager->getOcctWindow(),
                                                      *mySelectionManager);

        if (!occtView) {
            myLogger->error("Failed to create OcctView");
            return false;
        }

        // Set selection mode to face selection
        mySelectionManager->setSelectionMode(4);  // 4 is for face selection in OCCT

        // Initialize OCCT view
        occtView->initialize();

        myLogger->info("Views initialized successfully");
        return true;
    }
    catch (const std::exception& e) {
        myLogger->error("Exception during view initialization: {}", e.what());
        return false;
    }
    catch (...) {
        myLogger->error("Unknown exception during view initialization");
        return false;
    }
}