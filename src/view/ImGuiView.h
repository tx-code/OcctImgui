#pragma once

#include "IView.h"
#include "mvvm/MessageBus.h"
#include "viewmodel/IViewModel.h"
#include "viewmodel/UnifiedViewModel.h"
#include <functional>
#include <imgui.h>
#include <map>
#include <memory>
#include <string>


struct GLFWwindow;

class ImGuiView: public IView
{
public:
    ImGuiView(std::shared_ptr<IViewModel> viewModel, MVVM::MessageBus& messageBus);
    ~ImGuiView() override;

    // IView接口实现
    void initialize(GLFWwindow* window) override;
    void newFrame() override;
    void render() override;
    void shutdown() override;
    bool wantCaptureMouse() const override;
    std::shared_ptr<IViewModel> getViewModel() const override
    {
        return myViewModel;
    }

private:
    std::shared_ptr<IViewModel> myViewModel;
    GLFWwindow* myWindow;

    /** Reference to the message bus */
    MVVM::MessageBus& myMessageBus;

    /** Connection tracker for signal connections */
    MVVM::ConnectionTracker myConnections;

    // UI状态
    bool showObjectProperties = true;
    bool showObjectTree = true;
    bool showDemoWindow = false;
    std::string mySelectionMessage;

    // 获取UnifiedViewModel的辅助方法
    std::shared_ptr<UnifiedViewModel> getUnifiedViewModel() const;

    // 各UI组件的渲染方法
    void renderMainMenu();
    void renderToolbar();
    void renderObjectProperties();
    void renderObjectTree();
    void renderStatusBar();

    // 特定类型视图模型的UI渲染
    void renderGeometryProperties();
    void renderGeometryTree();

    // 命令执行方法
    void executeCreateBox();
    void executeCreateCone();
    void executeCreateMesh();
    void executeDeleteSelected();
    void executeImportModel();

    // 订阅事件
    void subscribeToEvents();
};