#include "ImGuiView.h"
#include "../viewmodel/Commands.h"
#include "../mvvm/MessageBus.h"
#include "../mvvm/GlobalSettings.h"

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
    if (std::dynamic_pointer_cast<UnifiedViewModel>(myViewModel)) {
        return ViewModelType::UNIFIED;
    }
    return ViewModelType::UNKNOWN;
}

std::shared_ptr<UnifiedViewModel> ImGuiView::getUnifiedViewModel() const {
    return std::dynamic_pointer_cast<UnifiedViewModel>(myViewModel);
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
            
            if (viewModelType == ViewModelType::UNIFIED) {
                if (ImGui::MenuItem("Box")) {
                    executeCreateBox();
                }
                if (ImGui::MenuItem("Cone")) {
                    executeCreateCone();
                }
                if (ImGui::MenuItem("Mesh")) {
                    executeCreateMesh();
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
    
    if (viewModelType == ViewModelType::UNIFIED) {
        if (ImGui::Button("Box")) {
            executeCreateBox();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cone")) {
            executeCreateCone();
        }
        ImGui::SameLine();
        if (ImGui::Button("Mesh")) {
            executeCreateMesh();
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
    
    if (viewModelType == ViewModelType::UNIFIED) {
        renderGeometryProperties();
    } else {
        ImGui::Text("Unknown view model type");
    }
    
    ImGui::End();
}

void ImGuiView::renderGeometryProperties() {
    auto unifiedViewModel = getUnifiedViewModel();
    if (!unifiedViewModel) return;
    
    if (unifiedViewModel->hasSelection()) {
        ImGui::Text("Selected objects: %zu", unifiedViewModel->getSelectedObjects().size());
        
        // 显示颜色选择器
        Quantity_Color currentColor = unifiedViewModel->getSelectedColor();
        float color[3] = {
            static_cast<float>(currentColor.Red()),
            static_cast<float>(currentColor.Green()),
            static_cast<float>(currentColor.Blue())
        };
        
        if (ImGui::ColorEdit3("Color", color)) {
            // 更新颜色
            Quantity_Color newColor(color[0], color[1], color[2], Quantity_TOC_RGB);
            unifiedViewModel->setSelectedColor(newColor);
        }
        
        // 显示显示模式选择
        int displayMode = unifiedViewModel->displayMode.get();
        const char* displayModes[] = { "Shaded", "Wireframe", "Vertices" };
        
        if (ImGui::Combo("Display Mode", &displayMode, displayModes, IM_ARRAYSIZE(displayModes))) {
            unifiedViewModel->displayMode = displayMode;
        }
    } else {
        ImGui::Text("No objects selected");
    }
    
    // 显示全局设置
    auto& globalSettings = unifiedViewModel->getGlobalSettings();
    
    bool isGridVisible = globalSettings.isGridVisible.get();
    if (ImGui::Checkbox("Show Grid", &isGridVisible)) {
        globalSettings.isGridVisible = isGridVisible;
    }
    
    bool isViewCubeVisible = globalSettings.isViewCubeVisible.get();
    if (ImGui::Checkbox("Show View Cube", &isViewCubeVisible)) {
        globalSettings.isViewCubeVisible = isViewCubeVisible;
    }
}

void ImGuiView::renderObjectTree() {
    ImGui::Begin("Objects", &showObjectTree);
    
    auto viewModelType = getViewModelType();
    
    if (viewModelType == ViewModelType::UNIFIED) {
        renderGeometryTree();
    } else {
        ImGui::Text("Unknown view model type");
    }
    
    ImGui::End();
}

void ImGuiView::renderGeometryTree() {
    auto unifiedViewModel = getUnifiedViewModel();
    if (!unifiedViewModel) return;
    
    auto model = unifiedViewModel->getUnifiedModel();
    if (!model) {
        ImGui::Text("No model available");
        return;
    }
    
    const auto& entityIds = model->getAllEntityIds();
    
    ImGui::Text("Objects: %zu", entityIds.size());
    ImGui::Separator();
    
    for (const auto& id : entityIds) {
        try {
            UnifiedModel::GeometryType type = model->getGeometryType(id);
            std::string typeStr;
            
            switch (type) {
                case UnifiedModel::GeometryType::SHAPE:
                    typeStr = "CAD";
                    break;
                case UnifiedModel::GeometryType::MESH:
                    typeStr = "Mesh";
                    break;
                default:
                    typeStr = "Unknown";
            }
            
            std::string label = id + " [" + typeStr + "]";
            bool isSelected = std::find(unifiedViewModel->getSelectedObjects().begin(),
                                        unifiedViewModel->getSelectedObjects().end(),
                                        id) != unifiedViewModel->getSelectedObjects().end();
            
            if (ImGui::Selectable(label.c_str(), isSelected)) {
                // TODO: 处理选择
            }
        } catch (const std::exception& e) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", e.what());
        }
    }
}

void ImGuiView::renderStatusBar() {
    const float height = ImGui::GetFrameHeight();
    const ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
    
    ImGui::SetNextWindowPos(ImVec2(0, viewportSize.y - height));
    ImGui::SetNextWindowSize(ImVec2(viewportSize.x, height));
    
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | 
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;
    
    if (ImGui::Begin("StatusBar", nullptr, windowFlags)) {
        ImGui::Text("OpenCascade ImGui Demo");
        ImGui::SameLine(ImGui::GetWindowWidth() - 120);
        
        if (myViewModel->hasSelection()) {
            ImGui::Text("Selected: %zu", myViewModel->getSelectedObjects().size());
        } else {
            ImGui::Text("No selection");
        }
    }
    ImGui::End();
}

void ImGuiView::executeCreateBox() {
    auto unifiedViewModel = getUnifiedViewModel();
    if (!unifiedViewModel) return;
    
    // 创建一个10x10x10的盒子，位于原点
    unifiedViewModel->createBox(gp_Pnt(0, 0, 0), 10, 10, 10);
}

void ImGuiView::executeCreateCone() {
    auto unifiedViewModel = getUnifiedViewModel();
    if (!unifiedViewModel) return;
    
    // 创建一个底半径5，高为10的圆锥，位于原点
    unifiedViewModel->createCone(gp_Pnt(0, 0, 0), 5, 10);
}

void ImGuiView::executeCreateMesh() {
    auto unifiedViewModel = getUnifiedViewModel();
    if (!unifiedViewModel) return;
    
    // 创建一个示例网格
    // 注意：这个方法需要在UnifiedViewModel中实现
    unifiedViewModel->createMesh();
}

void ImGuiView::executeDeleteSelected() {
    if (myViewModel->hasSelection()) {
        myViewModel->deleteSelectedObjects();
    }
}

void ImGuiView::subscribeToEvents() {
    // 订阅相关事件
    // ...
} 