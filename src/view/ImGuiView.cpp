#include "ImGuiView.h"
#include "../viewmodel/Commands.h"
#include "../mvvm/MessageBus.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// 添加spdlog头文件
#include <spdlog/spdlog.h>

ImGuiView::ImGuiView(std::shared_ptr<CadViewModel> viewModel)
    : myViewModel(viewModel), myWindow(nullptr) {
    subscribeToEvents();
}

ImGuiView::~ImGuiView() {
    // shutdown();
}

void ImGuiView::initialize(GLFWwindow* window) {
    spdlog::info("ImGui: Starting initialization");
    
    if (window == nullptr) {
        spdlog::error("ImGui: Initialization failed - window pointer is null");
        return;
    }
    myWindow = window;
    
    // 检查OpenGL上下文是否有效
    if (glfwGetCurrentContext() == nullptr) {
        spdlog::error("ImGui: Initialization failed - no valid OpenGL context");
        return;
    }

    try {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        
        // 设置ImGui风格
        ImGui::StyleColorsDark();
        
        // 初始化ImGui平台后端
        bool glfwInitSuccess = ImGui_ImplGlfw_InitForOpenGL(window, true);
        if (!glfwInitSuccess) {
            spdlog::error("ImGui: GLFW backend initialization failed");
            return;
        }
        
        bool opengl3InitSuccess = ImGui_ImplOpenGL3_Init("#version 330");
        if (!opengl3InitSuccess) {
            spdlog::error("ImGui: OpenGL3 backend initialization failed");
            return;
        }
        
        spdlog::info("ImGui: Initialization successful");
    } catch (const std::exception& e) {
        spdlog::error("ImGui: Initialization exception: {}", e.what());
    } catch (...) {
        spdlog::error("ImGui: Unknown exception during initialization");
    }
}

void ImGuiView::newFrame() {
    try {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    } catch (const std::exception& e) {
        spdlog::error("ImGui: New frame exception: {}", e.what());
    } catch (...) {
        spdlog::error("ImGui: Unknown exception during new frame");
    }
}

void ImGuiView::render() {
    try {
        newFrame();
        
        // 渲染各界面元素
        renderMainMenu();
        renderToolbar();
        
        if (showObjectProperties) {
            renderObjectProperties();
        }
        
        if (showObjectTree) {
            renderObjectTree();
        }
        
        renderStatusBar();
        
        // Demo窗口（用于开发）
        if (showDemoWindow) {
            ImGui::ShowDemoWindow(&showDemoWindow);
        }
        
        // 渲染
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    } catch (const std::exception& e) {
        spdlog::error("ImGui: Render exception: {}", e.what());
    } catch (...) {
        spdlog::error("ImGui: Unknown exception during render");
    }
}

void ImGuiView::shutdown() {
    spdlog::info("ImGui: Starting shutdown");
    if (myWindow) {
        try {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            spdlog::info("ImGui: Shutdown successful");
        } catch (const std::exception& e) {
            spdlog::error("ImGui: Shutdown exception: {}", e.what());
        } catch (...) {
            spdlog::error("ImGui: Unknown exception during shutdown");
        }
    }
}

bool ImGuiView::wantCaptureMouse() const {
    return ImGui::GetIO().WantCaptureMouse;
}

void ImGuiView::renderMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
                // 创建新文件逻辑
            }
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                // 打开文件逻辑
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                // 保存文件逻辑
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                glfwSetWindowShouldClose(myWindow, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Edit")) {
            bool hasSelection = myViewModel->hasSelection();
            
            if (ImGui::MenuItem("Delete", "Del", false, hasSelection)) {
                executeDeleteSelected();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Create")) {
            if (ImGui::MenuItem("Box")) {
                executeCreateBox();
            }
            if (ImGui::MenuItem("Cone")) {
                executeCreateCone();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Properties", nullptr, &showObjectProperties);
            ImGui::MenuItem("Object Tree", nullptr, &showObjectTree);
            ImGui::MenuItem("ImGui Demo", nullptr, &showDemoWindow);
            
            ImGui::Separator();
            
            bool isGridVisible = myViewModel->isGridVisible.get();
            if (ImGui::MenuItem("Show Grid", nullptr, &isGridVisible)) {
                myViewModel->isGridVisible.set(isGridVisible);
            }
            
            bool isViewCubeVisible = myViewModel->isViewCubeVisible.get();
            if (ImGui::MenuItem("Show ViewCube", nullptr, &isViewCubeVisible)) {
                myViewModel->isViewCubeVisible.set(isViewCubeVisible);
            }
            
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}

void ImGuiView::renderToolbar() {
    ImGui::Begin("Tools", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize 
                | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar 
                | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);
    
    ImGui::SetWindowPos(ImVec2(0, 20));
    ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 40));
    
    if (ImGui::Button("Box")) {
        executeCreateBox();
    }
    ImGui::SameLine();
    
    if (ImGui::Button("Cone")) {
        executeCreateCone();
    }
    ImGui::SameLine();
    
    bool hasSelection = myViewModel->hasSelection();
    ImGui::BeginDisabled(!hasSelection);
    if (ImGui::Button("Delete")) {
        executeDeleteSelected();
    }
    ImGui::EndDisabled();
    
    ImGui::End();
}

void ImGuiView::renderObjectProperties() {
    ImGui::Begin("Properties", &showObjectProperties);
    
    if (myViewModel->hasSelection()) {
        auto selectedObjects = myViewModel->getSelectedObjects();
        ImGui::Text("Selected objects: %zu", selectedObjects.size());
        
        // 颜色编辑器
        Quantity_Color currentColor = myViewModel->getSelectedColor();
        float color[3] = {
            (float)currentColor.Red(), 
            (float)currentColor.Green(), 
            (float)currentColor.Blue()
        };
        
        if (ImGui::ColorEdit3("Color", color)) {
            Quantity_Color newColor(color[0], color[1], color[2], Quantity_TOC_RGB);
            
            Commands::SetColorCommand cmd(myViewModel, newColor);
            cmd.execute();
        }
        
        // 显示模式
        int displayMode = myViewModel->displayMode.get();
        const char* displayModes[] = { "Shaded", "Wireframe", "Points" };
        if (ImGui::Combo("Display Mode", &displayMode, displayModes, IM_ARRAYSIZE(displayModes))) {
            myViewModel->displayMode.set(displayMode);
        }
    } else {
        ImGui::Text("No object selected");
    }
    
    ImGui::End();
}

void ImGuiView::renderObjectTree() {
    ImGui::Begin("Objects", &showObjectTree);
    
    auto allShapeIds = myViewModel->getModel()->getAllShapeIds();
    for (const auto& id : allShapeIds) {
        // TODO
        // bool isSelected = myViewModel->isObjectSelected(id);
        // if (ImGui::Selectable(id.c_str(), isSelected)) {
        //     myViewModel->selectObject(id, !isSelected);
        // }
    }
    
    ImGui::End();
}

void ImGuiView::renderStatusBar() {
    ImGui::Begin("StatusBar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize 
                | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar 
                | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse);
    
    float statusBarHeight = 20.0f;
    ImGui::SetWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - statusBarHeight));
    ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, statusBarHeight));
    
    ImGui::Text("Ready");
    ImGui::SameLine(ImGui::GetWindowWidth() - 120);
    ImGui::Text("Objects: %zu", myViewModel->getModel()->getAllShapeIds().size());
    
    ImGui::End();
}

void ImGuiView::executeCreateBox() {
    // 创建默认大小的立方体
    gp_Pnt location(0.0, 0.0, 0.0);
    Commands::CreateBoxCommand cmd(myViewModel, location, 50.0, 50.0, 50.0);
    cmd.execute();
}

void ImGuiView::executeCreateCone() {
    // 创建默认大小的圆锥
    gp_Pnt location(0.0, 0.0, 0.0);
    Commands::CreateConeCommand cmd(myViewModel, location, 25.0, 50.0);
    cmd.execute();
}

void ImGuiView::executeDeleteSelected() {
    Commands::DeleteSelectedCommand cmd(myViewModel);
    if (cmd.canExecute()) {
        cmd.execute();
    }
}

void ImGuiView::subscribeToEvents() {
    // 订阅选择变更事件
    MVVM::MessageBus::getInstance().subscribe(
        MVVM::MessageBus::MessageType::SelectionChanged,
        [this](const MVVM::MessageBus::Message& msg) {
            // 刷新属性面板
            // 这里可能需要某种形式的更新UI状态的机制
        }
    );
    
    // 订阅模型变更事件
    MVVM::MessageBus::getInstance().subscribe(
        MVVM::MessageBus::MessageType::ModelChanged,
        [this](const MVVM::MessageBus::Message& msg) {
            // 刷新对象树
            // 这里可能需要某种形式的更新UI状态的机制
        }
    );
} 