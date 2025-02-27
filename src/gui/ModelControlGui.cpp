#include "ModelControlGui.h"
#include "../importers/IShapeImporter.h"
#include "../model/ModelManager.h"
#include <AIS_Shape.hxx>
#include <MeshVS_Mesh.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <V3d_View.hxx>
#include <imgui.h>
#include <nfd.h>
#include <sstream>


ModelControlGui::ModelControlGui(std::shared_ptr<ModelManager> modelManager)
    : myModelManager(modelManager)
    , myIsVisible(true)
{
    // 生成唯一的组件ID
    std::stringstream ss;
    ss << "ModelControlGui_" << this;
    myComponentId = ss.str();

    // 注册到事件系统
    ModelEventSystem::getInstance().registerListener(this, myComponentId);
}

ModelControlGui::~ModelControlGui()
{
    // 从事件系统取消注册
    ModelEventSystem::getInstance().unregisterListener(myComponentId);
}

void ModelControlGui::render()
{
    if (!myIsVisible)
        return;

    ImGui::Begin("Model Tree", &myIsVisible);
    
    // 添加重要的控制功能
    if (ImGui::Button("Import Model")) {
        importFile(myView);
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Reset View")) {
        if (!myView.IsNull()) {
            myView->Reset();
            myView->Update();
        }
    }
    
    // 显示选择控制
    ImGui::Separator();
    showSelectionControls();
    
    ImGui::Separator();
    
    // 获取交互上下文中的所有对象
    Handle(AIS_InteractiveContext) context = myModelManager->getContext();
    if (context.IsNull()) {
        ImGui::Text("Invalid interactive context");
        ImGui::End();
        return;
    }

    // 显示对象列表
    AIS_ListOfInteractive objectList;
    context->DisplayedObjects(objectList);

    ImGui::Text("Model objects: %d", objectList.Extent());
    ImGui::Separator();

    // 遍历并显示所有对象
    size_t index = 0;
    for (AIS_ListIteratorOfListOfInteractive it(objectList); it.More(); it.Next(), ++index) {
        renderObjectNode(it.Value(), index);
    }

    // 显示右键菜单（空白区域）
    if (ImGui::BeginPopupContextWindow("TreeContextMenu")) {
        showGlobalContextMenu();
        ImGui::EndPopup();
    }
    
    // 如果选中了对象，显示选择信息
    ImGui::Separator();
    showSelectionInfo();

    ImGui::End();
}

void ModelControlGui::renderObjectNode(const Handle(AIS_InteractiveObject)& theObject, size_t theIndex)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    // 构建对象名称
    std::string nodeName = "Object #" + std::to_string(theIndex);

    // 尝试从对象中获取名称（如果有的话）
    TCollection_ExtendedString objectName;
    bool hasName = false;

    if (theObject->DynamicType() == STANDARD_TYPE(AIS_Shape)) {
        Handle(AIS_Shape) shape = Handle(AIS_Shape)::DownCast(theObject);
        objectName = "Shape";  // 使用一个简单的标识
        hasName = true;
    }

    if (hasName) {
        nodeName = "Shape " + std::to_string(theIndex);
    }

    bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)theIndex, flags, "%s", nodeName.c_str());

    // 处理右键菜单
    if (ImGui::BeginPopupContextItem()) {
        showContextMenu(theObject);
        ImGui::EndPopup();
    }

    if (nodeOpen) {
        renderObjectProperties(theObject);
        renderObjectVisibility(theObject);
        renderDisplayMode(theObject);
        ImGui::TreePop();
    }
}

void ModelControlGui::showContextMenu(const Handle(AIS_InteractiveObject)& theObject)
{
    // 对象的右键菜单
    if (ImGui::MenuItem("Hide")) {
        myModelManager->getContext()->Erase(theObject, Standard_False);
    }

    if (ImGui::MenuItem("Delete")) {
        myModelManager->getContext()->Remove(theObject, Standard_False);
    }

    if (ImGui::MenuItem("Set as Active Object")) {
        myModelManager->getContext()->SetSelected(theObject, Standard_False);
    }

    ImGui::Separator();

    // 重用全局菜单中的导入功能
    if (ImGui::MenuItem("Import Model...")) {
        importFile(myView);
    }
}

void ModelControlGui::showGlobalContextMenu()
{
    // 全局右键菜单（在空白区域点击右键）
    if (ImGui::MenuItem("Import Model...")) {
        importFile(myView);
    }

    if (ImGui::MenuItem("Clear All Objects")) {
        myModelManager->getContext()->RemoveAll(Standard_False);
    }

    if (ImGui::MenuItem("Reset View")) {
        if (!myView.IsNull()) {
            myView->Reset();
            myView->Update();
        }
    }
}

void ModelControlGui::renderObjectProperties(const Handle(AIS_InteractiveObject)& theObject)
{
    ImGui::TextColored(ImVec4(0.5f, 0.7f, 1.0f, 1.0f), "Properties");
    ImGui::Separator();
    
    // 显示对象类型
    ImGui::Text("Type: %s", theObject->DynamicType()->Name());
    
    // 如果是形状对象，显示更多细节
    if (theObject->DynamicType() == STANDARD_TYPE(AIS_Shape)) {
        Handle(AIS_Shape) shape = Handle(AIS_Shape)::DownCast(theObject);
        TopAbs_ShapeEnum shapeType = shape->Shape().ShapeType();
        ImGui::Text("Shape Type: %s", getShapeTypeString(shapeType).c_str());
        
        // 显示其他属性，如位置、尺寸等
        TopLoc_Location loc = shape->Shape().Location();
        gp_Trsf trsf = loc.Transformation();
        gp_XYZ translation = trsf.TranslationPart();
        
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", 
                   translation.X(), translation.Y(), translation.Z());
    }
    // 如果是网格对象，显示其特有属性
    else if (theObject->DynamicType() == STANDARD_TYPE(MeshVS_Mesh)) {
        Handle(MeshVS_Mesh) mesh = Handle(MeshVS_Mesh)::DownCast(theObject);
        ImGui::Text("Mesh Object");
        // 可以添加网格特有的属性显示
    }
}

void ModelControlGui::renderObjectVisibility(const Handle(AIS_InteractiveObject)& theObject)
{
    ImGui::TextColored(ImVec4(0.5f, 0.7f, 1.0f, 1.0f), "Visibility");
    ImGui::Separator();
    
    Handle(AIS_InteractiveContext) context = myModelManager->getContext();
    if (context.IsNull())
        return;
    
    // 显示/隐藏控制
    bool isVisible = context->IsDisplayed(theObject);
    if (ImGui::Checkbox("Display", &isVisible)) {
        if (isVisible) {
            context->Display(theObject, Standard_False);
        } else {
            context->Erase(theObject, Standard_False);
        }
        context->UpdateCurrentViewer();
    }
    
    // 透明度控制
    Standard_Real transparency = theObject->Transparency();
    float transp = static_cast<float>(transparency);
    if (ImGui::SliderFloat("Transparency", &transp, 0.0f, 1.0f, "%.2f")) {
        theObject->SetTransparency(static_cast<Standard_Real>(transp));
        context->UpdateCurrentViewer();
    }
}

void ModelControlGui::renderDisplayMode(const Handle(AIS_InteractiveObject)& theObject)
{
    ImGui::TextColored(ImVec4(0.5f, 0.7f, 1.0f, 1.0f), "Display Mode");
    ImGui::Separator();
    
    Handle(AIS_InteractiveContext) context = myModelManager->getContext();
    if (context.IsNull())
        return;
    
    // 获取当前显示模式
    Standard_Integer currentMode = context->DisplayMode();
    
    // 存储到 map 中，用于保持状态
    auto it = myDisplayModes.find(theObject);
    if (it == myDisplayModes.end()) {
        myDisplayModes[theObject] = currentMode;
    } else {
        currentMode = it->second;
    }
    
    // 显示模式选项
    const char* modes[] = {"Wire Frame", "Hidden Wire Frame", "Shaded"};
    int modeIndex = 0;
    
    // 根据 OCCT 显示模式值映射到索引
    switch (currentMode) {
        case 0: modeIndex = 0; break; // AIS_WireFrame
        case 1: modeIndex = 2; break; // AIS_Shaded
        case 3: modeIndex = 1; break; // AIS_ShadedWithEdges
        default: modeIndex = 0;
    }
    
    if (ImGui::Combo("Mode", &modeIndex, modes, IM_ARRAYSIZE(modes))) {
        // 将索引映射回 OCCT 显示模式值
        Standard_Integer newMode = 0;
        switch (modeIndex) {
            case 0: newMode = 0; break; // AIS_WireFrame
            case 1: newMode = 3; break; // AIS_ShadedWithEdges
            case 2: newMode = 1; break; // AIS_Shaded
        }
        
        // 更新显示模式
        context->SetDisplayMode(theObject, newMode, Standard_False);
        myDisplayModes[theObject] = newMode; // 更新存储的模式
        context->UpdateCurrentViewer();
    }
}

void ModelControlGui::registerImporter(std::shared_ptr<IShapeImporter> importer)
{
    if (importer) {
        myImporters.push_back(importer);
    }
}

void ModelControlGui::setView(const Handle(V3d_View) & view)
{
    myView = view;
}

void ModelControlGui::updateSelectionMode(const Handle(AIS_InteractiveObject) & theObject)
{
    if (theObject.IsNull())
        return;

    // 根据对象类型设置选择模式
    if (theObject->DynamicType() == STANDARD_TYPE(AIS_Shape)) {
        myModelManager->setSelectionMode(theObject, mySelectionModes.topoShapeMode);
    }
    else if (theObject->DynamicType() == STANDARD_TYPE(MeshVS_Mesh)) {
        myModelManager->setSelectionMode(theObject, mySelectionModes.meshMode);
    }
}

void ModelControlGui::showSelectionInfo()
{
    Handle(AIS_InteractiveContext) context = myModelManager->getContext();
    if (context.IsNull())
        return;

    ImGui::Text("Selection Information:");

    // 显示当前选中的对象信息
    context->InitSelected();
    if (context->MoreSelected()) {
        Handle(AIS_InteractiveObject) selected = context->SelectedInteractive();
        if (!selected.IsNull()) {
            ImGui::Text("Selected Object Type: %s", selected->DynamicType()->Name());

            if (selected->DynamicType() == STANDARD_TYPE(AIS_Shape)) {
                Handle(AIS_Shape) shape = Handle(AIS_Shape)::DownCast(selected);
                TopAbs_ShapeEnum shapeType = shape->Shape().ShapeType();
                ImGui::Text("Shape Type: %s", getShapeTypeString(shapeType).c_str());
            }
        }
    }
    else {
        ImGui::Text("No object selected");
    }
}

std::string ModelControlGui::getShapeTypeString(TopAbs_ShapeEnum theType) const
{
    switch (theType) {
        case TopAbs_COMPOUND:
            return "Compound";
        case TopAbs_COMPSOLID:
            return "CompSolid";
        case TopAbs_SOLID:
            return "Solid";
        case TopAbs_SHELL:
            return "Shell";
        case TopAbs_FACE:
            return "Face";
        case TopAbs_WIRE:
            return "Wire";
        case TopAbs_EDGE:
            return "Edge";
        case TopAbs_VERTEX:
            return "Vertex";
        case TopAbs_SHAPE:
            return "Shape";
        default:
            return "Unknown";
    }
}

void ModelControlGui::importFile(const Handle(V3d_View) & view)
{
    if (myImporters.empty()) {
        ImGui::OpenPopup("Error");
        return;
    }

    nfdchar_t* outPath = nullptr;
    nfdfilteritem_t filterItem[2] = {{"CAD Files", "step,stp,obj,stl"}, {"All Files", "*"}};

    nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 2, nullptr);

    if (result == NFD_OKAY) {
        std::string filePath(outPath);

        // 根据文件扩展名选择导入器
        std::string ext = filePath.substr(filePath.find_last_of(".") + 1);
        for (size_t i = 0; i < ext.length(); ++i) {
            ext[i] = std::tolower(ext[i]);
        }

        for (auto& importer : myImporters) {
            if (myModelManager->importModel(filePath, importer)) {
                if (!view.IsNull()) {
                    view->FitAll();
                }
                break;
            }
        }

        NFD_FreePath(outPath);
    }
}

void ModelControlGui::showSelectionControls()
{
    bool hasTopoShapes = false;
    bool hasMeshes = false;

    // 检查是否存在不同类型的对象
    const auto& objects = myModelManager->getObjects();
    for (const auto& obj : objects) {
        if (obj->DynamicType() == STANDARD_TYPE(AIS_Shape)) {
            hasTopoShapes = true;
        }
        else if (obj->DynamicType() == STANDARD_TYPE(MeshVS_Mesh)) {
            hasMeshes = true;
        }
    }

    // TopoShape选择模式
    if (hasTopoShapes) {
        ImGui::Text("CAD Model Selection Mode:");
        static const char* topoItems[] =
            {"Neutral", "Vertex", "Edge", "Wire", "Face", "Shell", "Solid"};
        int previousMode = mySelectionModes.topoShapeMode;

        if (ImGui::Combo("##TopoMode",
                         &mySelectionModes.topoShapeMode,
                         topoItems,
                         IM_ARRAYSIZE(topoItems))) {
            if (previousMode != mySelectionModes.topoShapeMode) {
                for (const auto& obj : objects) {
                    if (obj->DynamicType() == STANDARD_TYPE(AIS_Shape)) {
                        updateSelectionMode(obj);
                    }
                }
            }
        }
    }

    // Mesh选择模式
    if (hasMeshes) {
        ImGui::Text("Mesh Selection Mode:");
        static const char* meshItems[] = {"Neutral", "Node", "Element"};
        int previousMode = mySelectionModes.meshMode;

        if (ImGui::Combo("##MeshMode",
                         &mySelectionModes.meshMode,
                         meshItems,
                         IM_ARRAYSIZE(meshItems))) {
            if (previousMode != mySelectionModes.meshMode) {
                for (const auto& obj : objects) {
                    if (obj->DynamicType() == STANDARD_TYPE(MeshVS_Mesh)) {
                        updateSelectionMode(obj);
                    }
                }
            }
        }
    }
}

void ModelControlGui::onModelEvent(const ModelEventData& eventData)
{
    switch (eventData.type) {
        case ModelEventType::ObjectAdded: {
            // 为新添加的对象设置选择模式
            updateSelectionMode(eventData.object);
            break;
        }
        default:
            // 其他事件处理
            break;
    }
}