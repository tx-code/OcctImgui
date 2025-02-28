#pragma once

#include "../GlfwOcctWindow.h"
#include "../viewmodel/CadViewModel.h"
#include <AIS_ViewController.hxx>
#include <memory>

class AIS_ViewCube;

class OcctView: protected AIS_ViewController
{
public:
    OcctView(std::shared_ptr<CadViewModel> viewModel, Handle(GlfwOcctWindow) window);
    ~OcctView();

    void initialize();
    void render();
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
    std::shared_ptr<CadViewModel> myViewModel;
    Handle(GlfwOcctWindow) myWindow;
    Handle(V3d_View) myView;
    Handle(AIS_ViewCube) myViewCube;
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