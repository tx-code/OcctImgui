#pragma once

#include "IGUIComponent.h"
#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <memory>
#include <vector>

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

    // 在 ModelControlGui 类的 public 部分添加：
    void setView(const Handle(V3d_View)& view);

private:
    void updateSelectionMode(const Handle(AIS_InteractiveObject)& theObject);
    void showSelectionInfo();
    std::string getShapeTypeString(TopAbs_ShapeEnum theType) const;
    void importFile(const Handle(V3d_View)& view);
    void showSelectionControls();

    std::shared_ptr<ModelManager> myModelManager;
    SelectionModes mySelectionModes;
    std::vector<std::shared_ptr<IShapeImporter>> myImporters;
    bool myIsVisible;
    std::string myComponentId;
    Handle(V3d_View) myView;
}; 