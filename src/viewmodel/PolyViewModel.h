#pragma once

#include "IViewModel.h"
#include "../model/PolyModel.h"
#include "../mvvm/Observable.h"
#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <AIS_Triangulation.hxx>
#include <memory>
#include <set>
#include <string>
#include <map>

class PolyViewModel : public IViewModel {
public:
    PolyViewModel(std::shared_ptr<PolyModel> model, 
                Handle(AIS_InteractiveContext) context);
    ~PolyViewModel() override = default;
    
    // 命令 - 对应UI操作
    void createTriangle(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3);
    void importMesh(const std::string& filePath);
    
    // IViewModel接口实现
    void deleteSelectedObjects() override;
    bool hasSelection() const override;
    std::vector<std::string> getSelectedObjects() const override;
    void processSelection(const Handle(AIS_InteractiveObject)& obj, bool isSelected) override;
    void clearSelection() override;
    Handle(AIS_InteractiveContext) getContext() const override { return myContext; }
    std::shared_ptr<IModel> getModel() const override { return myModel; }
    
    // 可观察属性
    MVVM::Observable<bool> isWireframeVisible{true};
    MVVM::Observable<bool> isVerticesVisible{false};
    MVVM::Observable<int> displayMode{0}; // 0: Shaded, 1: Wireframe, 2: Points...
    
    // 属性访问与修改
    void setSelectedColor(const Quantity_Color& color);
    Quantity_Color getSelectedColor() const;
    
    // PolyModel特定访问器
    std::shared_ptr<PolyModel> getPolyModel() const { return myModel; }
    
private:
    std::shared_ptr<PolyModel> myModel;
    Handle(AIS_InteractiveContext) myContext;
    std::set<std::string> mySelectedObjects;
    
    // 模型->视图同步
    void updatePresentation(const std::string& meshId);
    
    // 映射OCCT对象与模型ID
    std::map<Handle(AIS_InteractiveObject), std::string> myObjectToIdMap;
    std::map<std::string, Handle(AIS_InteractiveObject)> myIdToObjectMap;
    
    // 模型变更监听
    void onModelChanged(const std::string& meshId);
}; 