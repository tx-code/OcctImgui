#pragma once

#include "../GlfwOcctWindow.h"
#include "../viewmodel/UnifiedViewModel.h"
#include "../mvvm/MessageBus.h"
#include "IView.h"
#include <AIS_ViewController.hxx>
#include <memory>

class AIS_ViewCube;

class OcctView: public IView, protected AIS_ViewController
{
public:
    OcctView(std::shared_ptr<UnifiedViewModel> viewModel, 
             Handle(GlfwOcctWindow) window,
             MVVM::MessageBus& messageBus);
    ~OcctView() override;

    // IView 接口实现
    void initialize(GLFWwindow* window) override;
    void newFrame() override;
    void render() override;
    void shutdown() override;
    bool wantCaptureMouse() const override;
    std::shared_ptr<IViewModel> getViewModel() const override;

    // OcctView 特有方法
    void initialize();
    void cleanup();

    // 事件处理
    void onMouseMove(int posX, int posY);
    void onMouseButton(int button, int action, int mods);
    void onMouseScroll(double offsetX, double offsetY);
    void onResize(int width, int height);

    // 获取视图
    Handle(V3d_View) getView() const
    {
        return myView;
    }

    bool toWaitEvents() const
    {
        return myToWaitEvents;
    }

protected:
    // 重写基类方法
    void handleViewRedraw(const Handle(AIS_InteractiveContext) & theCtx,
                          const Handle(V3d_View) & theView) override;

private:
    std::shared_ptr<UnifiedViewModel> myViewModel;
    Handle(GlfwOcctWindow) myWindow;
    Handle(V3d_View) myView;
    Handle(AIS_ViewCube) myViewCube;
    MVVM::MessageBus& myMessageBus;
    bool myToWaitEvents = true;

    // 创建视图配置
    void setupViewCube();
    void setupGrid();
    void updateVisibility();

    // 处理选择
    void handleSelection(int x, int y);

    // 订阅事件
    void subscribeToEvents();
};