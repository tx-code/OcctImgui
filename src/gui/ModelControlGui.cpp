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

    ImGui::Begin("Model Control", &myIsVisible);

    // 导入功能按钮
    if (ImGui::Button("Import Model")) {
        importFile(myView);
    }

    ImGui::Separator();

    // 选择控制区域
    showSelectionControls();

    ImGui::Separator();

    // 显示选择信息
    showSelectionInfo();

    ImGui::End();
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
    nfdfilteritem_t filterItem[2] = {{"CAD Files", "step,stp,iges,igs,stl"}, {"All Files", "*"}};

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