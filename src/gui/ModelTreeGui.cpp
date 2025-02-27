#include "ModelTreeGui.h"
#include "../model/ModelManager.h"
#include <AIS_Shape.hxx>
#include <BRepBndLib.hxx>
#include <TopExp_Explorer.hxx>
#include <Bnd_Box.hxx>
#include <imgui.h>
#include <sstream>

ModelTreeGui::ModelTreeGui(std::shared_ptr<ModelManager> modelManager)
    : myModelManager(modelManager)
    , myIsVisible(true)
{
    // 生成唯一的组件ID
    std::stringstream ss;
    ss << "ModelTreeGui_" << this;
    myComponentId = ss.str();
    
    // 注册到事件系统
    ModelEventSystem::getInstance().registerListener(this, myComponentId);
}

ModelTreeGui::~ModelTreeGui()
{
    // 从事件系统取消注册
    ModelEventSystem::getInstance().unregisterListener(myComponentId);
}

void ModelTreeGui::render()
{
    if (!myIsVisible)
        return;
        
    ImGui::Begin("Model Tree", &myIsVisible);
    
    const auto& objects = myModelManager->getObjects();
    for (size_t i = 0; i < objects.size(); ++i) {
        renderObjectNode(objects[i], i);
    }
    
    ImGui::End();
}

void ModelTreeGui::renderObjectNode(const Handle(AIS_InteractiveObject)& theObject, size_t theIndex)
{
    if (theObject.IsNull())
        return;
        
    // 构建对象名称
    std::string objectName = "Object_" + std::to_string(theIndex);
    if (theObject->DynamicType() == STANDARD_TYPE(AIS_Shape)) {
        objectName = "Shape_" + std::to_string(theIndex);
    }
    
    // 创建树节点
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    
    // 检查是否被选中
    Handle(AIS_InteractiveContext) context = myModelManager->getContext();
    if (!context.IsNull() && context->IsSelected(theObject)) {
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
    }
    
    bool isOpen = ImGui::TreeNodeEx((void*)(intptr_t)theIndex, nodeFlags, "%s", objectName.c_str());
    
    // 处理点击选择
    if (ImGui::IsItemClicked()) {
        myModelManager->selectObject(theObject);
    }
    
    // 右键菜单
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Focus")) {
            // TODO: 实现焦点功能
        }
        if (ImGui::MenuItem("Delete")) {
            myModelManager->removeObject(theObject);
            ImGui::EndPopup();
            if (isOpen) ImGui::TreePop();
            return;
        }
        ImGui::EndPopup();
    }
    
    // 显示对象内容
    if (isOpen) {
        renderObjectVisibility(theObject);
        renderDisplayMode(theObject);
        renderObjectProperties(theObject);
        ImGui::TreePop();
    }
}

void ModelTreeGui::renderObjectProperties(const Handle(AIS_InteractiveObject)& theObject)
{
    if (theObject.IsNull())
        return;
        
    ImGui::Text("Properties:");
    
    if (theObject->DynamicType() == STANDARD_TYPE(AIS_Shape)) {
        auto shape = Handle(AIS_Shape)::DownCast(theObject);
        // 显示形状特有的属性
        TopExp_Explorer faceExp(shape->Shape(), TopAbs_FACE);
        TopExp_Explorer edgeExp(shape->Shape(), TopAbs_EDGE);
        TopExp_Explorer vertExp(shape->Shape(), TopAbs_VERTEX);
        int nFaces = 0, nEdges = 0, nVertices = 0;
        for (; faceExp.More(); faceExp.Next())
            nFaces++;
        for (; edgeExp.More(); edgeExp.Next())
            nEdges++;
        for (; vertExp.More(); vertExp.Next())
            nVertices++;

        ImGui::Text("Topology:");
        ImGui::Text("  Faces: %d", nFaces);
        ImGui::Text("  Edges: %d", nEdges);
        ImGui::Text("  Vertices: %d", nVertices);

        Bnd_Box bbox;
        BRepBndLib::Add(shape->Shape(), bbox);
        Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
        bbox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

        ImGui::Text("Dimensions:");
        ImGui::Text("  Size: %.2f x %.2f x %.2f", xmax - xmin, ymax - ymin, zmax - zmin);
        ImGui::Text("  Center: (%.2f, %.2f, %.2f)",
                    (xmax + xmin) / 2,
                    (ymax + ymin) / 2,
                    (zmax + zmin) / 2);
    }
}

void ModelTreeGui::renderObjectVisibility(const Handle(AIS_InteractiveObject)& theObject)
{
    if (theObject.IsNull())
        return;
        
    Handle(AIS_InteractiveContext) context = myModelManager->getContext();
    if (context.IsNull())
        return;
        
    bool isVisible = context->IsDisplayed(theObject);
    if (ImGui::Checkbox("Visible", &isVisible)) {
        myModelManager->setVisibility(theObject, isVisible);
    }
}

void ModelTreeGui::renderDisplayMode(const Handle(AIS_InteractiveObject)& theObject)
{
    if (theObject.IsNull())
        return;
        
    Handle(AIS_InteractiveContext) context = myModelManager->getContext();
    if (context.IsNull())
        return;
        
    // 获取当前显示模式
    auto it = myDisplayModes.find(theObject);
    int currentMode = 0;
    if (it != myDisplayModes.end()) {
        currentMode = it->second;
    } else {
        currentMode = context->DisplayMode();
        myDisplayModes[theObject] = currentMode;
    }
    
    // 显示模式选择
    const char* displayModes[] = {"Wireframe", "Shaded", "HLR"};
    int displayMode = currentMode;
    
    if (ImGui::Combo("Display Mode", &displayMode, displayModes, IM_ARRAYSIZE(displayModes))) {
        myModelManager->setDisplayMode(theObject, displayMode);
        myDisplayModes[theObject] = displayMode;
    }
}

void ModelTreeGui::onModelEvent(const ModelEventData& eventData)
{
    switch (eventData.type) {
        case ModelEventType::ObjectRemoved: {
            // 从显示模式缓存中移除
            myDisplayModes.erase(eventData.object);
            break;
        }
        case ModelEventType::DisplayModeChanged: {
            // 更新显示模式缓存
            myDisplayModes[eventData.object] = eventData.intValue;
            break;
        }
        default:
            // 其他事件处理
            break;
    }
} 