#include "OcctView.h"
#include "mvvm/GlobalSettings.h"
#include "mvvm/MessageBus.h"
#include "utils/Logger.h"

#include <AIS_Shape.hxx>
#include <AIS_ViewCube.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_Handle.hxx>
#include <GLFW/glfw3.h>
#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

// 使用宏声明 OcctView 类的 logger
DECLARE_LOGGER(OcctView)

// 辅助函数，转换GLFW鼠标按键为OCCT按键
namespace
{
//! Convert GLFW mouse button into Aspect_VKeyMouse.
static Aspect_VKeyMouse mouseButtonFromGlfw(int theButton)
{
    switch (theButton) {
        case GLFW_MOUSE_BUTTON_LEFT:
            return Aspect_VKeyMouse_LeftButton;
        case GLFW_MOUSE_BUTTON_RIGHT:
            return Aspect_VKeyMouse_RightButton;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            return Aspect_VKeyMouse_MiddleButton;
    }
    return Aspect_VKeyMouse_NONE;
}

//! Convert GLFW key modifiers into Aspect_VKeyFlags.
static Aspect_VKeyFlags keyFlagsFromGlfw(int theFlags)
{
    Aspect_VKeyFlags aFlags = Aspect_VKeyFlags_NONE;
    if ((theFlags & GLFW_MOD_SHIFT) != 0) {
        aFlags |= Aspect_VKeyFlags_SHIFT;
    }
    if ((theFlags & GLFW_MOD_CONTROL) != 0) {
        aFlags |= Aspect_VKeyFlags_CTRL;
    }
    if ((theFlags & GLFW_MOD_ALT) != 0) {
        aFlags |= Aspect_VKeyFlags_ALT;
    }
    if ((theFlags & GLFW_MOD_SUPER) != 0) {
        aFlags |= Aspect_VKeyFlags_META;
    }
    return aFlags;
}
}  // namespace

OcctView::OcctView(std::shared_ptr<GeometryViewModel> viewModel, Handle(GlfwOcctWindow) window)
    : myViewModel(viewModel)
    , myWindow(window)
{
    getOcctViewLogger()->info("Creating view");
    subscribeToEvents();
}

OcctView::~OcctView()
{
    getOcctViewLogger()->info("Cleaning up view");

    // Disconnect all signal connections
    myConnections.disconnectAll();

    // Clean up resources
    cleanup();
}

void OcctView::cleanup()
{
    getOcctViewLogger()->info("Cleaning up view");
    if (!myView.IsNull()) {
        myView->Remove();
    }
}

void OcctView::initialize()
{
    LOG_FUNCTION_SCOPE(getOcctViewLogger(), "initialize");
    getOcctViewLogger()->info("Starting initialization");
    if (myWindow.IsNull() || myWindow->getGlfwWindow() == nullptr) {
        getOcctViewLogger()->error("Initialization failed - invalid window");
        return;
    }

    try {
        // 检查OpenGL上下文
        if (glfwGetCurrentContext() == nullptr) {
            getOcctViewLogger()->error("Initialization failed - no current OpenGL context");
            return;
        }

        // 创建图形驱动
        Handle(OpenGl_GraphicDriver) aGraphicDriver =
            new OpenGl_GraphicDriver(myWindow->GetDisplay(), Standard_False);
        aGraphicDriver->SetBuffersNoSwap(Standard_True);
        getOcctViewLogger()->info("OCCT: OpenGL graphic driver created, BuffersNoSwap=True");

        // 创建3D查看器
        Handle(V3d_Viewer) aViewer = myViewModel->getViewer();
        aViewer->SetDefaultLights();
        aViewer->SetLightOn();
        aViewer->SetDefaultTypeOfView(V3d_PERSPECTIVE);
        aViewer->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);
        getOcctViewLogger()->info("OCCT: V3d_Viewer configured");

        // 创建视图
        myView = aViewer->CreateView();
        if (myView.IsNull()) {
            getOcctViewLogger()->error("OCCT: Failed to create view");
            return;
        }

        myView->SetWindow(myWindow, myWindow->NativeGlContext());
        myView->Window()->DoResize();
        myView->ChangeRenderingParams().ToShowStats = Standard_True;
        getOcctViewLogger()->info("OCCT: V3d_View created and configured");

        // 显示视图
        myWindow->Map();
        getOcctViewLogger()->info("OCCT: Window mapped");

        // 设置视图组件
        setupViewCube();
        setupGrid();
        getOcctViewLogger()->info("OCCT: View components setup complete");

        // 应用初始设置
        updateVisibility();

        // 输出OpenGL信息
        TCollection_AsciiString aGlInfo;
        TColStd_IndexedDataMapOfStringString aRendInfo;
        myView->DiagnosticInformation(aRendInfo, Graphic3d_DiagnosticInfo_Basic);
        for (TColStd_IndexedDataMapOfStringString::Iterator aValueIter(aRendInfo);
             aValueIter.More();
             aValueIter.Next()) {
            getOcctViewLogger()->info("OCCT OpenGL: {} = {}",
                                      aValueIter.Key().ToCString(),
                                      aValueIter.Value().ToCString());
        }

        getOcctViewLogger()->info("OCCT: Initialization complete");
    }
    catch (const std::exception& e) {
        getOcctViewLogger()->error("OCCT: Initialization exception: {}", e.what());
    }
    catch (...) {
        getOcctViewLogger()->error("OCCT: Unknown exception during initialization");
    }
}

void OcctView::render()
{
    if (myView.IsNull() || myViewModel->getContext().IsNull()) {
        getOcctViewLogger()->warn("OCCT: Render skipped - view or context is null");
        return;
    }

    try {
        // 立即更新视图
        myView->InvalidateImmediate();

        // 刷新视图事件
        FlushViewEvents(myViewModel->getContext(), myView, Standard_True);
    }
    catch (const std::exception& e) {
        getOcctViewLogger()->error("OCCT: Render exception: {}", e.what());
    }
    catch (...) {
        getOcctViewLogger()->error("OCCT: Unknown exception during render");
    }
}

void OcctView::onMouseMove(int posX, int posY)
{
    if (myView.IsNull()) {
        return;
    }

    const Graphic3d_Vec2i aNewPos(posX, posY);
    UpdateMousePosition(aNewPos, PressedMouseButtons(), LastMouseFlags(), Standard_False);
}

void OcctView::onMouseButton(int button, int action, int mods)
{
    auto logger = getOcctViewLogger();
    logger->debug("Mouse button: {}, action: {}, mods: {}", button, action, mods);

    if (myView.IsNull()) {
        return;
    }

    const Graphic3d_Vec2i aPos = myWindow->CursorPosition();

    // Handle OCCT view control
    if (action == GLFW_PRESS) {
        PressMouseButton(aPos, mouseButtonFromGlfw(button), keyFlagsFromGlfw(mods), false);

        // Handle selection on left click without modifiers
        if (button == GLFW_MOUSE_BUTTON_LEFT && (mods & GLFW_MOD_CONTROL) == 0) {
            handleSelection(aPos.x(), aPos.y());
        }
        // Right click to clear selection
        else if (button == GLFW_MOUSE_BUTTON_RIGHT && (mods & GLFW_MOD_CONTROL) == 0) {
            getOcctViewLogger()->info("Clearing selection");
            MVVM::SelectionManager::getInstance().clearSelection();
            myViewModel->getContext()->ClearSelected(Standard_True);
        }
    }
    else {
        ReleaseMouseButton(aPos, mouseButtonFromGlfw(button), keyFlagsFromGlfw(mods), false);
    }
}

void OcctView::onMouseScroll(double offsetX, double offsetY)
{
    if (myView.IsNull()) {
        return;
    }

    UpdateZoom(Aspect_ScrollDelta(myWindow->CursorPosition(), int(offsetY * 8.0)));
}

void OcctView::onResize(int width, int height)
{
    if (width != 0 && height != 0 && !myView.IsNull()) {
        myView->Window()->DoResize();
        myView->MustBeResized();
        myView->Invalidate();
        FlushViewEvents(myViewModel->getContext(), myView, true);
    }
}

void OcctView::handleViewRedraw(const Handle(AIS_InteractiveContext) & theCtx,
                                const Handle(V3d_View) & theView)
{
    AIS_ViewController::handleViewRedraw(theCtx, theView);
    myToWaitEvents = !myToAskNextFrame;
}

void OcctView::setupViewCube()
{
    myViewCube = new AIS_ViewCube();
    myViewCube->SetSize(55);
    myViewCube->SetFontHeight(12);
    myViewCube->SetAxesLabels("", "", "");
    myViewCube->SetTransformPersistence(new Graphic3d_TransformPers(Graphic3d_TMF_TriedronPers,
                                                                    Aspect_TOTP_RIGHT_UPPER,
                                                                    Graphic3d_Vec2i(85, 85)));
    myViewCube->SetViewAnimation(ViewAnimation());
    myViewCube->SetFixedAnimationLoop(false);
    myViewModel->getContext()->Display(myViewCube, false);
}

void OcctView::setupGrid()
{
    // 配置网格
    myViewModel->getContext()->CurrentViewer()->ActivateGrid(Aspect_GT_Rectangular,
                                                             Aspect_GDM_Lines);
}

void OcctView::updateVisibility()
{
    // 使用ViewModel获取全局设置
    auto& globalSettings = myViewModel->getGlobalSettings();

    // 更新网格可见性
    bool isGridVisible = globalSettings.isGridVisible.get();
    if (isGridVisible) {
        myViewModel->getContext()->CurrentViewer()->ActivateGrid(Aspect_GT_Rectangular,
                                                                 Aspect_GDM_Lines);
    }
    else {
        myViewModel->getContext()->CurrentViewer()->DeactivateGrid();
    }

    // 更新视图立方体可见性
    bool isViewCubeVisible = globalSettings.isViewCubeVisible.get();
    if (!myViewCube.IsNull()) {
        if (isViewCubeVisible) {
            myViewModel->getContext()->Display(myViewCube, false);
        }
        else {
            myViewModel->getContext()->Erase(myViewCube, false);
        }
    }

    // 更新显示模式
    int displayMode = myViewModel->displayMode.get();
    switch (displayMode) {
        case 0:  // Shaded
            myViewModel->getContext()->SetDisplayMode(AIS_Shaded, Standard_True);
            break;
        case 1:  // Wireframe
            myViewModel->getContext()->SetDisplayMode(AIS_WireFrame, Standard_True);
            break;
        case 2:  // Points
            // 需要其他处理...
            break;
    }

    myViewModel->getContext()->UpdateCurrentViewer();

    // 强制重绘视图
    if (!myView.IsNull()) {
        myView->Invalidate();
    }
}

void OcctView::handleSelection(int x, int y)
{
    auto logger = getOcctViewLogger();
    logger->info("Handling selection at position ({}, {})", x, y);

    // Move to the position and perform selection
    myViewModel->getContext()->MoveTo(x, y, myView, Standard_True);
    myViewModel->getContext()->Select(Standard_True);

    // Get selected objects
    AIS_ListOfInteractive selected;
    myViewModel->getContext()->DisplayedObjects(selected);

    // Filter to only get selected objects
    AIS_ListOfInteractive selectedObjects;
    for (AIS_ListOfInteractive::Iterator it(selected); it.More(); it.Next()) {
        Handle(AIS_InteractiveObject) obj = it.Value();
        if (myViewModel->getContext()->IsSelected(obj)) {
            selectedObjects.Append(obj);
        }
    }

    logger->info("Selected {} objects", selectedObjects.Extent());

    // Process each selected object
    for (AIS_ListOfInteractive::Iterator it(selectedObjects); it.More(); it.Next()) {
        Handle(AIS_InteractiveObject) obj = it.Value();

        // Generate a unique ID for the object
        std::string objectId =
            "object_" + std::to_string(reinterpret_cast<std::uintptr_t>(obj.get()));

        logger->info("Selected object: {}", objectId);

        // Add to selection manager
        MVVM::SelectionManager::getInstance().addToSelection(obj, objectId);

        // Also process in view model if needed
        myViewModel->processSelection(obj, true);
    }
}

void OcctView::subscribeToEvents()
{
    getOcctViewLogger()->info("Subscribing to events");

    // 使用单个订阅对象订阅多个消息类型
    mySubscriptions = MVVM::MessageBus::getInstance().subscribeMultiple(
        {MVVM::MessageBus::MessageType::ModelChanged,
         MVVM::MessageBus::MessageType::SelectionChanged},
        [this](const MVVM::MessageBus::Message& message) {
            switch (message.type) {
                case MVVM::MessageBus::MessageType::ModelChanged:
                    // Force view redraw on model change
                    if (!myView.IsNull()) {
                        myView->Invalidate();
                    }
                    break;

                case MVVM::MessageBus::MessageType::SelectionChanged:
                    // Handle selection change
                    try {
                        const auto& selectionInfo =
                            std::any_cast<MVVM::SelectionInfo>(message.data);
                        getOcctViewLogger()->info("Selection changed: {} objects selected",
                                                  selectionInfo.selectedObjects.size());

                        // Highlight selected objects in the view
                        if (!myView.IsNull()) {
                            myView->Invalidate();
                        }
                    }
                    catch (const std::bad_any_cast& e) {
                        getOcctViewLogger()->error("Failed to cast selection info: {}", e.what());
                    }
                    break;

                default:
                    break;
            }
        });

    // Get global settings
    auto& globalSettings = myViewModel->getGlobalSettings();

    // Connect to grid visibility property
    auto gridConn = globalSettings.isGridVisible.valueChanged.connect(
        [this](const bool&, const bool& isVisible) {
            updateVisibility();
            if (!myView.IsNull()) {
                myView->Invalidate();
            }
        });
    myConnections.track(gridConn);

    // Connect to view cube visibility property
    auto cubeConn = globalSettings.isViewCubeVisible.valueChanged.connect(
        [this](const bool&, const bool& isVisible) {
            updateVisibility();
            if (!myView.IsNull()) {
                myView->Invalidate();
            }
        });
    myConnections.track(cubeConn);

    // Connect to display mode property
    auto displayConn =
        myViewModel->displayMode.valueChanged.connect([this](const int&, const int& mode) {
            // Update display mode
            if (!myView.IsNull()) {
                myView->Invalidate();
            }
        });
    myConnections.track(displayConn);

    // 不再需要连接到选择属性，因为现在使用 SelectionManager
    // 订阅 SelectionChanged 消息已经足够
}

// IView 接口实现
void OcctView::initialize(GLFWwindow* window)
{
    getOcctViewLogger()->info("OcctView: Initializing with GLFW window");
    // 调用原始的初始化方法
    initialize();
}

void OcctView::newFrame()
{
    // OcctView 不需要为每一帧做特殊准备
    // 这个方法是为了满足 IView 接口
}

void OcctView::shutdown()
{
    getOcctViewLogger()->info("OcctView: Shutting down");
    cleanup();
}

bool OcctView::wantCaptureMouse() const
{
    // OcctView 通常需要捕获鼠标事件，但不应该阻止其他视图
    // 返回 false 允许事件继续传播
    return false;
}

std::shared_ptr<IViewModel> OcctView::getViewModel() const
{
    return std::static_pointer_cast<IViewModel>(myViewModel);
}