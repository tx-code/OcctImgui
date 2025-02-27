#pragma once

#include "IGUIComponent.h"
#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <memory>
#include <vector>
#include <map>
#include <string>

class ModelManager;
class IShapeImporter;

class ModelControlGui : public IGUIComponent {
public:
    // 为不同类型的对象定义选择模式
    struct SelectionModes {
        int topoShapeMode = 0; // 0: Neutral, 1: Vertex, 2: Edge, 3: Wire, 4: Face, 5: Shell, 6: Solid
        int meshMode = 0;      // 0: Neutral
    };

    explicit ModelControlGui(std::shared_ptr<ModelManager> modelManager);
    ~ModelControlGui();

    // 实现IGUIComponent接口
    void render() override;
    bool isVisible() const override { return myIsVisible; }
    void setVisible(bool visible) override { myIsVisible = visible; }
    void onModelEvent(const ModelEventData& eventData) override;

    // 注册导入器
    void registerImporter(std::shared_ptr<IShapeImporter> importer);

    // 获取选择模式
    const SelectionModes& getSelectionModes() const { return mySelectionModes; }

    // 设置视图
    void setView(const Handle(V3d_View)& view);

private:
    void updateSelectionMode(const Handle(AIS_InteractiveObject)& theObject);
    void showSelectionInfo();
    std::string getShapeTypeString(TopAbs_ShapeEnum theType) const;
    void importFile(const Handle(V3d_View)& view);
    void showSelectionControls();
    
    // 模型树功能
    void renderObjectNode(const Handle(AIS_InteractiveObject)& theObject, size_t theIndex);
    void renderObjectProperties(const Handle(AIS_InteractiveObject)& theObject);
    void renderObjectVisibility(const Handle(AIS_InteractiveObject)& theObject);
    void renderDisplayMode(const Handle(AIS_InteractiveObject)& theObject);
    
    // 右键菜单功能
    void showContextMenu(const Handle(AIS_InteractiveObject)& theObject);
    void showGlobalContextMenu();

    std::shared_ptr<ModelManager> myModelManager;
    SelectionModes mySelectionModes;
    std::vector<std::shared_ptr<IShapeImporter>> myImporters;
    bool myIsVisible;
    std::string myComponentId;
    Handle(V3d_View) myView;
    
    // 从ModelTreeGui合并的成员
    std::map<Handle(AIS_InteractiveObject), Standard_Integer> myDisplayModes;
}; 