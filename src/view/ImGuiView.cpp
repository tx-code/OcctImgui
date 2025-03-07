#include "ImGuiView.h"
#include "mvvm/GlobalSettings.h"
#include "mvvm/MessageBus.h"
#include "utils/Logger.h"
#include "viewmodel/Commands.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <nfd.h>

// 使用宏声明 ImGuiView 类的 logger
DECLARE_LOGGER(ImGuiView)

ImGuiView::ImGuiView(std::shared_ptr<IViewModel> viewModel)
    : myViewModel(viewModel)
    , myWindow(nullptr)
{
    getImGuiViewLogger()->info("Creating view");
    subscribeToEvents();
}

ImGuiView::~ImGuiView()
{
    myConnections.disconnectAll();
}

std::shared_ptr<GeometryViewModel> ImGuiView::getGeometryViewModel() const
{
    return std::dynamic_pointer_cast<GeometryViewModel>(myViewModel);
}

void ImGuiView::initialize(GLFWwindow* window)
{
    LOG_FUNCTION_SCOPE(getImGuiViewLogger(), "initialize");
    getImGuiViewLogger()->info("Starting initialization");

    if (window == nullptr) {
        getImGuiViewLogger()->error("Initialization failed - window pointer is null");
        return;
    }
    myWindow = window;

    // 检查OpenGL上下文是否有效
    if (glfwGetCurrentContext() == nullptr) {
        getImGuiViewLogger()->error("Initialization failed - no valid OpenGL context");
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
            getImGuiViewLogger()->error("GLFW backend initialization failed");
            return;
        }

        // 初始化ImGui渲染器后端
        bool gl3InitSuccess = ImGui_ImplOpenGL3_Init("#version 130");
        if (!gl3InitSuccess) {
            getImGuiViewLogger()->error("OpenGL3 backend initialization failed");
            return;
        }

        getImGuiViewLogger()->info("Initialization completed successfully");
    }
    catch (const std::exception& e) {
        getImGuiViewLogger()->error("Exception during initialization: {}", e.what());
    }
    catch (...) {
        getImGuiViewLogger()->error("Unknown exception during initialization");
    }
}

void ImGuiView::newFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiView::render()
{
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

void ImGuiView::shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool ImGuiView::wantCaptureMouse() const
{
    return ImGui::GetIO().WantCaptureMouse;
}

void ImGuiView::renderMainMenu()
{
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
            if (ImGui::MenuItem("Import Model", "Ctrl+I")) {
                executeImportModel();
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
            auto GeometryViewModel = getGeometryViewModel();

            if (GeometryViewModel) {
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

void ImGuiView::renderToolbar()
{
    ImGui::Begin("Toolbar",
                 nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                     | ImGuiWindowFlags_NoScrollbar);

    auto GeometryViewModel = getGeometryViewModel();

    if (GeometryViewModel) {
        if (ImGui::Button("Import")) {
            executeImportModel();
        }
        ImGui::SameLine();
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

void ImGuiView::renderObjectProperties()
{
    ImGui::Begin("Object Properties", &showObjectProperties);

    auto GeometryViewModel = getGeometryViewModel();

    if (GeometryViewModel) {
        renderGeometryProperties();
    }
    else {
        ImGui::Text("Unknown view model type");
    }

    ImGui::End();
}

void ImGuiView::renderGeometryProperties()
{
    auto GeometryViewModel = getGeometryViewModel();
    if (!GeometryViewModel)
        return;

    if (GeometryViewModel->hasSelection()) {
        ImGui::Text("Selected objects: %zu", GeometryViewModel->getSelectedObjects().size());

        // 显示颜色选择器
        Quantity_Color currentColor = GeometryViewModel->getSelectedColor();
        float color[3] = {static_cast<float>(currentColor.Red()),
                          static_cast<float>(currentColor.Green()),
                          static_cast<float>(currentColor.Blue())};

        if (ImGui::ColorEdit3("Color", color)) {
            // 更新颜色
            Quantity_Color newColor(color[0], color[1], color[2], Quantity_TOC_RGB);
            GeometryViewModel->setSelectedColor(newColor);
        }

        // 显示显示模式选择
        int displayMode = GeometryViewModel->displayMode.get();
        const char* displayModes[] = {"Shaded", "Wireframe", "Vertices"};

        if (ImGui::Combo("Display Mode", &displayMode, displayModes, IM_ARRAYSIZE(displayModes))) {
            GeometryViewModel->displayMode = displayMode;
        }
    }
    else {
        ImGui::Text("No objects selected");
    }

    // 显示全局设置
    auto& globalSettings = GeometryViewModel->getGlobalSettings();

    bool isGridVisible = globalSettings.isGridVisible.get();
    if (ImGui::Checkbox("Show Grid", &isGridVisible)) {
        globalSettings.isGridVisible = isGridVisible;
    }

    bool isViewCubeVisible = globalSettings.isViewCubeVisible.get();
    if (ImGui::Checkbox("Show View Cube", &isViewCubeVisible)) {
        globalSettings.isViewCubeVisible = isViewCubeVisible;
    }
}

void ImGuiView::renderObjectTree()
{
    ImGui::Begin("Objects", &showObjectTree);

    auto GeometryViewModel = getGeometryViewModel();

    if (GeometryViewModel) {
        renderGeometryTree();
    }
    else {
        ImGui::Text("Unknown view model type");
    }

    ImGui::End();
}

void ImGuiView::renderGeometryTree()
{
    auto GeometryViewModel = getGeometryViewModel();
    if (!GeometryViewModel)
        return;

    auto model = GeometryViewModel->getGeometryModel();
    if (!model) {
        ImGui::Text("No model available");
        return;
    }

    const auto& entityIds = model->getAllEntityIds();

    ImGui::Text("Objects: %zu", entityIds.size());
    ImGui::Separator();

    // TODO
#if 0
    for (const auto& id : entityIds) {
        try {
            GeometryModel::GeometryType type = model->getGeometryType(id);
            std::string typeStr;

            switch (type) {
                case GeometryModel::GeometryType::SHAPE:
                    typeStr = "CAD";
                    break;
                case GeometryModel::GeometryType::MESH:
                    typeStr = "Mesh";
                    break;
                default:
                    typeStr = "Unknown";
            }

            std::string label = id + " [" + typeStr + "]";
            bool isSelected = std::find(GeometryViewModel->getSelectedObjects().begin(),
                                        GeometryViewModel->getSelectedObjects().end(),
                                        id)
                != GeometryViewModel->getSelectedObjects().end();

            if (ImGui::Selectable(label.c_str(), isSelected)) {
                // TODO: 处理选择
            }
        }
        catch (const std::exception& e) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", e.what());
        }
    }
#endif
}

void ImGuiView::renderStatusBar()
{
    const float height = ImGui::GetFrameHeight();
    const ImVec2 viewportSize = ImGui::GetMainViewport()->Size;

    ImGui::SetNextWindowPos(ImVec2(0, viewportSize.y - height));
    ImGui::SetNextWindowSize(ImVec2(viewportSize.x, height));

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs
        | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin("StatusBar", nullptr, windowFlags)) {
        ImGui::Text("OpenCascade ImGui Demo");
        ImGui::SameLine(ImGui::GetWindowWidth() - 120);

        ImGui::Text(mySelectionMessage.c_str());
    }
    ImGui::End();
}

void ImGuiView::executeCreateBox()
{
    auto GeometryViewModel = getGeometryViewModel();
    if (!GeometryViewModel)
        return;

    // 使用命令模式创建盒子
    Commands::CreateBoxCommand boxCmd(GeometryViewModel, gp_Pnt(0, 0, 0), 10, 10, 10);
    boxCmd.execute();
}

void ImGuiView::executeCreateCone()
{
    auto GeometryViewModel = getGeometryViewModel();
    if (!GeometryViewModel)
        return;

    // 使用命令模式创建圆锥
    Commands::CreateConeCommand coneCmd(GeometryViewModel, gp_Pnt(0, 0, 0), 5, 10);
    coneCmd.execute();
}

void ImGuiView::executeCreateMesh()
{
    auto GeometryViewModel = getGeometryViewModel();
    if (!GeometryViewModel)
        return;

    // 创建一个示例网格
    // 注意：这个方法需要在GeometryViewModel中实现
    GeometryViewModel->createMesh();
}

void ImGuiView::executeDeleteSelected()
{
    if (myViewModel->hasSelection()) {
        // 使用命令模式删除选中对象
        Commands::DeleteSelectedCommand deleteCmd(myViewModel);
        deleteCmd.execute();
    }
}

void ImGuiView::executeImportModel()
{
    getImGuiViewLogger()->info("Executing import model command");

    // 初始化NFD (Native File Dialog)
    NFD_Init();

    nfdchar_t* outPath = nullptr;
    nfdfilteritem_t filterItems[4] = {{"All Files", "step,stp,stl,obj"},
                                      {"STEP Files", "step,stp"},
                                      {"STL Files", "stl"},
                                      {"OBJ Files", "obj"}};

    // 打开文件对话框
    nfdresult_t result = NFD_OpenDialog(&outPath, filterItems, 4, nullptr);

    if (result == NFD_OKAY) {
        getImGuiViewLogger()->info("Selected file: {}", outPath);

        // 获取GeometryViewModel
        auto GeometryViewModel = getGeometryViewModel();
        if (!GeometryViewModel) {
            getImGuiViewLogger()->error("Failed to get GeometryViewModel");
            NFD_FreePath(outPath);
            NFD_Quit();
            return;
        }

        // 创建并执行导入模型命令
        Commands::ImportModelCommand importCmd(GeometryViewModel, outPath);
        importCmd.execute();

        // 释放路径内存
        NFD_FreePath(outPath);
    }
    else if (result == NFD_CANCEL) {
        getImGuiViewLogger()->info("User canceled file dialog");
    }
    else {
        getImGuiViewLogger()->error("Error opening file dialog: {}", NFD_GetError());
    }

    // 清理NFD
    NFD_Quit();
}

void ImGuiView::subscribeToEvents()
{
    getImGuiViewLogger()->info("Subscribing to events");

    // 订阅 SelectionChanged 消息
    // Subscribe to selection changed events
    mySubcriptions = MVVM::MessageBus::getInstance().subscribeWithManager(
        MVVM::MessageBus::MessageType::SelectionChanged,
        [this](const MVVM::MessageBus::Message& message) {
            // Get the selection info
            try {
                const auto& selectionInfo = std::any_cast<MVVM::SelectionInfo>(message.data);
                // TODO  to string
            }
            catch (const std::bad_any_cast& e) {
                getImGuiViewLogger()->error("Failed to cast selection info: {}", e.what());
            }
        });
}