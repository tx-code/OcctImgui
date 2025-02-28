#include "OcctView.h"
#include "../mvvm/MessageBus.h"

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


// 添加spdlog头文件
#include <spdlog/spdlog.h>

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

OcctView::OcctView(std::shared_ptr<CadViewModel> viewModel, Handle(GlfwOcctWindow) window)
    : myViewModel(viewModel)
    , myWindow(window)
{
    spdlog::info("OCCT: Creating view");
    subscribeToEvents();
}

OcctView::~OcctView()
{
}

void OcctView::cleanup()
{
    spdlog::info("OCCT: Cleaning up view");
    if (!myView.IsNull()) {
        myView->Remove();
    }
}

void OcctView::initialize()
{
    spdlog::info("OCCT: Starting initialization");
    if (myWindow.IsNull() || myWindow->getGlfwWindow() == nullptr) {
        spdlog::error("OCCT: Initialization failed - invalid window");
        return;
    }
    
    try {
        // 检查OpenGL上下文
        if (glfwGetCurrentContext() == nullptr) {
            spdlog::error("OCCT: Initialization failed - no current OpenGL context");
            return;
        }
    
        // 创建图形驱动
        Handle(OpenGl_GraphicDriver) aGraphicDriver =
            new OpenGl_GraphicDriver(myWindow->GetDisplay(), Standard_False);
        aGraphicDriver->SetBuffersNoSwap(Standard_True);
        spdlog::info("OCCT: OpenGL graphic driver created, BuffersNoSwap=True");

        // 创建3D查看器
        Handle(V3d_Viewer) aViewer = myViewModel->getViewer();
        aViewer->SetDefaultLights();
        aViewer->SetLightOn();
        aViewer->SetDefaultTypeOfView(V3d_PERSPECTIVE);
        aViewer->ActivateGrid(Aspect_GT_Rectangular, Aspect_GDM_Lines);
        spdlog::info("OCCT: V3d_Viewer configured");

        // 创建视图
        myView = aViewer->CreateView();
        if (myView.IsNull()) {
            spdlog::error("OCCT: Failed to create view");
            return;
        }
        
        myView->SetWindow(myWindow, myWindow->NativeGlContext());
        myView->Window()->DoResize();
        myView->ChangeRenderingParams().ToShowStats = Standard_True;
        spdlog::info("OCCT: V3d_View created and configured");

        // 显示视图
        myWindow->Map();
        spdlog::info("OCCT: Window mapped");

        // 设置视图组件
        setupViewCube();
        setupGrid();
        spdlog::info("OCCT: View components setup complete");

        // 应用初始设置
        updateVisibility();
        
        // 输出OpenGL信息
        TCollection_AsciiString aGlInfo;
        TColStd_IndexedDataMapOfStringString aRendInfo;
        myView->DiagnosticInformation(aRendInfo, Graphic3d_DiagnosticInfo_Basic);
        for (TColStd_IndexedDataMapOfStringString::Iterator aValueIter(aRendInfo);
             aValueIter.More();
             aValueIter.Next()) {
            spdlog::info("OCCT OpenGL: {} = {}", 
                          aValueIter.Key().ToCString(), 
                          aValueIter.Value().ToCString());
        }
        
        spdlog::info("OCCT: Initialization complete");
    } catch (const std::exception& e) {
        spdlog::error("OCCT: Initialization exception: {}", e.what());
    } catch (...) {
        spdlog::error("OCCT: Unknown exception during initialization");
    }
}

void OcctView::render()
{
    if (myView.IsNull() || myViewModel->getContext().IsNull()) {
        spdlog::warn("OCCT: Render skipped - view or context is null");
        return;
    }
    
    try {
        // 立即更新视图
        myView->InvalidateImmediate();
        
        // 刷新视图事件
        FlushViewEvents(myViewModel->getContext(), myView, Standard_True);
    } catch (const std::exception& e) {
        spdlog::error("OCCT: Render exception: {}", e.what());
    } catch (...) {
        spdlog::error("OCCT: Unknown exception during render");
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
    if (myView.IsNull()) {
        return;
    }

    const Graphic3d_Vec2i aPos = myWindow->CursorPosition();
    if (action == GLFW_PRESS) {
        PressMouseButton(aPos, mouseButtonFromGlfw(button), keyFlagsFromGlfw(mods), false);

        // 处理选择
        if (button == GLFW_MOUSE_BUTTON_LEFT && (mods & GLFW_MOD_CONTROL) == 0) {
            handleSelection(aPos.x(), aPos.y());
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
    // 更新网格可见性
    bool isGridVisible = myViewModel->isGridVisible.get();
    if (isGridVisible) {
        myViewModel->getContext()->CurrentViewer()->ActivateGrid(Aspect_GT_Rectangular,
                                                                 Aspect_GDM_Lines);
    }
    else {
        myViewModel->getContext()->CurrentViewer()->DeactivateGrid();
    }

    // 更新ViewCube可见性
    bool isViewCubeVisible = myViewModel->isViewCubeVisible.get();
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
}

void OcctView::handleSelection(int x, int y)
{
    myViewModel->getContext()->MoveTo(x, y, myView, Standard_True);
    myViewModel->getContext()->Select(Standard_True);

    // 更新选中状态
    AIS_ListOfInteractive selected;
    // myViewModel->getContext()->Selection(selected);

    for (const Handle(AIS_InteractiveObject) & obj : selected) {
        myViewModel->processSelection(obj, true);
    }
}

void OcctView::subscribeToEvents()
{
    // 订阅模型变更事件
    MVVM::MessageBus::getInstance().subscribe(MVVM::MessageBus::MessageType::ModelChanged,
                                              [this](const MVVM::MessageBus::Message& msg) {
                                                  // 强制重绘
                                                  if (!myView.IsNull()) {
                                                      myView->Invalidate();
                                                  }
                                              });

    // 订阅视图属性变更
    myViewModel->isGridVisible.addObserver([this](const bool& value) {
        updateVisibility();
    });

    myViewModel->isViewCubeVisible.addObserver([this](const bool& value) {
        updateVisibility();
    });

    myViewModel->displayMode.addObserver([this](const int& value) {
        updateVisibility();
    });
}