#pragma once

#include "../viewmodel/CadViewModel.h"
#include <imgui.h>
#include <memory>

struct GLFWwindow;

class ImGuiView
{
public:
    ImGuiView(std::shared_ptr<CadViewModel> viewModel);
    ~ImGuiView();

    void initialize(GLFWwindow* window);
    void newFrame();
    void render();
    void shutdown();

    bool wantCaptureMouse() const;

private:
    std::shared_ptr<CadViewModel> myViewModel;
    GLFWwindow* myWindow;

    // UI状态
    bool showObjectProperties = true;
    bool showObjectTree = true;
    bool showDemoWindow = false;

    // 各UI组件的渲染方法
    void renderMainMenu();
    void renderToolbar();
    void renderObjectProperties();
    void renderObjectTree();
    void renderStatusBar();

    // 命令执行方法
    void executeCreateBox();
    void executeCreateCone();
    void executeDeleteSelected();

    // 订阅事件
    void subscribeToEvents();
};