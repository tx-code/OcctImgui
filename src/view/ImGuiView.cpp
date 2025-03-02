#include "ImGuiView.h"
#include "../viewmodel/Commands.h"
#include "../mvvm/MessageBus.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

// 添加spdlog头文件
#include <spdlog/spdlog.h>

ImGuiView::ImGuiView(std::shared_ptr<IViewModel> viewModel)
    : myViewModel(viewModel), myWindow(nullptr) {
    subscribeToEvents();
}

ImGuiView::~ImGuiView() {
    // shutdown();
}

ImGuiView::ViewModelType ImGuiView::getViewModelType() const {
    if (std::dynamic_pointer_cast<CadViewModel>(myViewModel)) {
        return ViewModelType::CAD;
    } else if (std::dynamic_pointer_cast<PolyViewModel>(myViewModel)) {
        return ViewModelType::POLY;
    }
    return ViewModelType::UNKNOWN;
}

std::shared_ptr<CadViewModel> ImGuiView::getCadViewModel() const {
    return std::dynamic_pointer_cast<CadViewModel>(myViewModel);
}

std::shared_ptr<PolyViewModel> ImGuiView::getPolyViewModel() const {
    return std::dynamic_pointer_cast<PolyViewModel>(myViewModel);
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
        // 移除Docking特性
        // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        
        // 设置ImGui风格
        ImGui::StyleColorsDark();
        
        // 初始化ImGui平台后端
        bool glfwInitSuccess = ImGui_ImplGlfw_InitForOpenGL(window, true);
        if (!glfwInitSuccess) {
            spdlog::error("ImGui: GLFW backend initialization failed");
            return;
        }
        
        // 初始化ImGui渲染器后端
        bool gl3InitSuccess = ImGui_ImplOpenGL3_Init("#version 130");
        if (!gl3InitSuccess) {
            spdlog::error("ImGui: OpenGL3 backend initialization failed");
            return;
        }
        
        spdlog::info("ImGui: Initialization completed successfully");
    }
    catch (const std::exception& e) {
        spdlog::error("ImGui: Exception during initialization: {}", e.what());
    }
    catch (...) {
        spdlog::error("ImGui: Unknown exception during initialization");
    }
}

void ImGuiView::newFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiView::render() {
    // 渲染菜单栏
    renderMainMenu();
    
    // 渲染工具栏
    renderToolbar();
    
    // 渲染对象属性面板
    if (showObjectProperties) {
        renderObjectProperties();
    }
    
    // 渲染对象树面板
    if (showObjectTree) {
        renderObjectTree();
    }
    
    // 渲染状态栏
    renderStatusBar();
    
    // 渲染ImGui演示窗口（用于开发调试）
    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }
    
    // 渲染ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiView::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool ImGuiView::wantCaptureMouse() const {
    return ImGui::GetIO().WantCaptureMouse;
}

void ImGuiView::renderMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
                // 处理新建命令
            }
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                // 处理打开命令
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                // 处理保存命令
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                // 处理退出命令
                glfwSetWindowShouldClose(myWindow, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Delete Selected", "Delete", false, myViewModel->hasSelection())) {
                executeDeleteSelected();
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Object Properties", nullptr, &showObjectProperties);
            ImGui::MenuItem("Object Tree", nullptr, &showObjectTree);
            ImGui::Separator();
            ImGui::MenuItem("ImGui Demo Window", nullptr, &showDemoWindow);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Create")) {
            auto viewModelType = getViewModelType();
            
            if (viewModelType == ViewModelType::CAD) {
                if (ImGui::MenuItem("Box")) {
                    executeCreateBox();
                }
                if (ImGui::MenuItem("Cone")) {
                    executeCreateCone();
                }
            } else if (viewModelType == ViewModelType::POLY) {
                if (ImGui::MenuItem("Triangle")) {
                    executeCreateTriangle();
                }
                if (ImGui::MenuItem("Import Mesh...")) {
                    executeImportMesh();
                }
            }
            
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}

void ImGuiView::renderToolbar() {
    ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    
    auto viewModelType = getViewModelType();
    
    if (viewModelType == ViewModelType::CAD) {
        if (ImGui::Button("Box")) {
            executeCreateBox();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cone")) {
            executeCreateCone();
        }
    } else if (viewModelType == ViewModelType::POLY) {
        if (ImGui::Button("Triangle")) {
            executeCreateTriangle();
        }
        ImGui::SameLine();
        if (ImGui::Button("Import Mesh")) {
            executeImportMesh();
        }
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Delete", ImVec2(0, 0))) {
        executeDeleteSelected();
    }
    
    ImGui::End();
}

void ImGuiView::renderObjectProperties() {
    ImGui::Begin("Object Properties", &showObjectProperties);
    
    auto viewModelType = getViewModelType();
    
    if (viewModelType == ViewModelType::CAD) {
        renderCadProperties();
    } else if (viewModelType == ViewModelType::POLY) {
        renderPolyProperties();
    } else {
        ImGui::Text("Unknown view model type");
    }
    
    ImGui::End();
}

void ImGuiView::renderCadProperties() {
    auto cadViewModel = getCadViewModel();
    if (!cadViewModel) return;
    
    if (cadViewModel->hasSelection()) {
        ImGui::Text("Selected objects: %zu", cadViewModel->getSelectedObjects().size());
        
        // 显示颜色选择器
        Quantity_Color currentColor = cadViewModel->getSelectedColor();
        float color[3] = {
            static_cast<float>(currentColor.Red()),
            static_cast<float>(currentColor.Green()),
            static_cast<float>(currentColor.Blue())
        };
        
        if (ImGui::ColorEdit3("Color", color)) {
            Quantity_Color newColor(color[0], color[1], color[2], Quantity_TOC_RGB);
            Commands::SetColorCommand cmd(cadViewModel, newColor);
            cmd.execute();
        }
        
        // 显示显示模式选择
        int displayMode = cadViewModel->displayMode.get();
        const char* displayModes[] = { "Shaded", "Wireframe", "Points" };
        if (ImGui::Combo("Display Mode", &displayMode, displayModes, IM_ARRAYSIZE(displayModes))) {
            cadViewModel->displayMode = displayMode;
        }
    } else {
        ImGui::Text("No objects selected");
    }
    
    // 全局设置
    bool isGridVisible = cadViewModel->isGridVisible.get();
    if (ImGui::Checkbox("Show Grid", &isGridVisible)) {
        cadViewModel->isGridVisible = isGridVisible;
    }
    
    bool isViewCubeVisible = cadViewModel->isViewCubeVisible.get();
    if (ImGui::Checkbox("Show View Cube", &isViewCubeVisible)) {
        cadViewModel->isViewCubeVisible = isViewCubeVisible;
    }
}

void ImGuiView::renderPolyProperties() {
    auto polyViewModel = getPolyViewModel();
    if (!polyViewModel) return;
    
    if (polyViewModel->hasSelection()) {
        ImGui::Text("Selected objects: %zu", polyViewModel->getSelectedObjects().size());
        
        // 显示颜色选择器
        Quantity_Color currentColor = polyViewModel->getSelectedColor();
        float color[3] = {
            static_cast<float>(currentColor.Red()),
            static_cast<float>(currentColor.Green()),
            static_cast<float>(currentColor.Blue())
        };
        
        if (ImGui::ColorEdit3("Color", color)) {
            Quantity_Color newColor(color[0], color[1], color[2], Quantity_TOC_RGB);
            Commands::SetColorCommand cmd(polyViewModel, newColor);
            cmd.execute();
        }
        
        // 显示显示模式选择
        int displayMode = polyViewModel->displayMode.get();
        const char* displayModes[] = { "Shaded", "Wireframe", "Points" };
        if (ImGui::Combo("Display Mode", &displayMode, displayModes, IM_ARRAYSIZE(displayModes))) {
            polyViewModel->displayMode = displayMode;
        }
    } else {
        ImGui::Text("No objects selected");
    }
    
    // 特定于网格的设置
    bool isWireframeVisible = polyViewModel->isWireframeVisible.get();
    if (ImGui::Checkbox("Show Wireframe", &isWireframeVisible)) {
        polyViewModel->isWireframeVisible = isWireframeVisible;
    }
    
    bool isVerticesVisible = polyViewModel->isVerticesVisible.get();
    if (ImGui::Checkbox("Show Vertices", &isVerticesVisible)) {
        polyViewModel->isVerticesVisible = isVerticesVisible;
    }
}

void ImGuiView::renderObjectTree() {
    ImGui::Begin("Object Tree", &showObjectTree);
    
    auto viewModelType = getViewModelType();
    
    if (viewModelType == ViewModelType::CAD) {
        renderCadTree();
    } else if (viewModelType == ViewModelType::POLY) {
        renderPolyTree();
    } else {
        ImGui::Text("Unknown view model type");
    }
    
    ImGui::End();
}

void ImGuiView::renderCadTree() {
    auto cadViewModel = getCadViewModel();
    if (!cadViewModel) return;
    
    auto cadModel = cadViewModel->getCadModel();
    auto shapeIds = cadModel->getAllShapeIds();
    auto selectedIds = cadViewModel->getSelectedObjects();
    
    ImGui::Text("CAD Objects (%zu)", shapeIds.size());
    
    for (const auto& id : shapeIds) {
        bool isSelected = std::find(selectedIds.begin(), selectedIds.end(), id) != selectedIds.end();
        if (ImGui::Selectable(id.c_str(), isSelected)) {
            // 处理选择逻辑
        }
    }
}

void ImGuiView::renderPolyTree() {
    auto polyViewModel = getPolyViewModel();
    if (!polyViewModel) return;
    
    auto polyModel = polyViewModel->getPolyModel();
    auto meshIds = polyModel->getAllMeshIds();
    auto selectedIds = polyViewModel->getSelectedObjects();
    
    ImGui::Text("Mesh Objects (%zu)", meshIds.size());
    
    for (const auto& id : meshIds) {
        bool isSelected = std::find(selectedIds.begin(), selectedIds.end(), id) != selectedIds.end();
        if (ImGui::Selectable(id.c_str(), isSelected)) {
            // 处理选择逻辑
        }
    }
}

void ImGuiView::renderStatusBar() {
    ImGui::Begin("Status Bar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    
    auto viewModelType = getViewModelType();
    
    if (viewModelType == ViewModelType::CAD) {
        auto cadViewModel = getCadViewModel();
        auto cadModel = cadViewModel->getCadModel();
        ImGui::Text("CAD Model | Objects: %zu | Selected: %zu", 
                   cadModel->getAllShapeIds().size(), 
                   cadViewModel->getSelectedObjects().size());
    } else if (viewModelType == ViewModelType::POLY) {
        auto polyViewModel = getPolyViewModel();
        auto polyModel = polyViewModel->getPolyModel();
        ImGui::Text("Polygon Model | Meshes: %zu | Selected: %zu", 
                   polyModel->getAllMeshIds().size(), 
                   polyViewModel->getSelectedObjects().size());
    } else {
        ImGui::Text("Unknown model type");
    }
    
    ImGui::End();
}

void ImGuiView::executeCreateBox() {
    auto cadViewModel = getCadViewModel();
    if (!cadViewModel) return;
    
    // 创建一个位于原点的10x10x10的盒子
    Commands::CreateBoxCommand cmd(cadViewModel, gp_Pnt(0, 0, 0), 10, 10, 10);
    cmd.execute();
}

void ImGuiView::executeCreateCone() {
    auto cadViewModel = getCadViewModel();
    if (!cadViewModel) return;
    
    // 创建一个位于原点的底半径5，高度10的圆锥
    Commands::CreateConeCommand cmd(cadViewModel, gp_Pnt(0, 0, 0), 5, 10);
    cmd.execute();
}

void ImGuiView::executeCreateTriangle() {
    auto polyViewModel = getPolyViewModel();
    if (!polyViewModel) return;
    
    // 创建一个简单的三角形
    Commands::CreateTriangleCommand cmd(polyViewModel, 
                                       gp_Pnt(0, 0, 0), 
                                       gp_Pnt(10, 0, 0), 
                                       gp_Pnt(5, 10, 0));
    cmd.execute();
}

void ImGuiView::executeImportMesh() {
    auto polyViewModel = getPolyViewModel();
    if (!polyViewModel) return;
    
    // 这里应该打开文件对话框，但为了简单起见，我们直接使用一个假路径
    Commands::ImportMeshCommand cmd(polyViewModel, "example.stl");
    cmd.execute();
}

void ImGuiView::executeDeleteSelected() {
    Commands::DeleteSelectedCommand cmd(myViewModel);
    cmd.execute();
}

void ImGuiView::subscribeToEvents() {
    // 订阅模型变更事件
    // 在实际应用中，这里应该订阅来自MessageBus的事件
} 