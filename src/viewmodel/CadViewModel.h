#pragma once

#include "IViewModel.h"
#include "../model/CadModel.h"
#include "../mvvm/Observable.h"
#include "../mvvm/GlobalSettings.h"
#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <AIS_Shape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <memory>
#include <set>
#include <string>
#include <map>

class CadViewModel : public IViewModel {
public:
    CadViewModel(std::shared_ptr<CadModel> model, 
                Handle(AIS_InteractiveContext) context);
    ~CadViewModel() override = default;
    
    // 命令 - 对应UI操作
    void createBox(const gp_Pnt& location, double sizeX, double sizeY, double sizeZ);
    void createCone(const gp_Pnt& location, double radius, double height);
    
    // IViewModel接口实现
    void deleteSelectedObjects() override;
    bool hasSelection() const override;
    std::vector<std::string> getSelectedObjects() const override;
    void processSelection(const Handle(AIS_InteractiveObject)& obj, bool isSelected) override;
    void clearSelection() override;
    Handle(AIS_InteractiveContext) getContext() const override { return myContext; }
    std::shared_ptr<IModel> getModel() const override { return myModel; }
    
    // 可观察属性
    // 注意: isGridVisible和isViewCubeVisible已移至MVVM::GlobalSettings
    MVVM::Observable<int> displayMode{0}; // 0: Shaded, 1: Wireframe, 2: Vertices...
    
    // 属性访问与修改
    void setSelectedColor(const Quantity_Color& color);
    Quantity_Color getSelectedColor() const;
    
    // CadModel特定访问器
    std::shared_ptr<CadModel> getCadModel() const { return myModel; }
    Handle(V3d_Viewer) getViewer() const {return myContext->CurrentViewer();}
    
    // 全局设置访问器
    MVVM::GlobalSettings& getGlobalSettings() const { return MVVM::GlobalSettings::getInstance(); }
    
private:
    std::shared_ptr<CadModel> myModel;
    Handle(AIS_InteractiveContext) myContext;
    std::set<std::string> mySelectedObjects;
    
    // 模型->视图同步
    void updatePresentation(const std::string& shapeId);
    
    // 映射OCCT对象与模型ID
    std::map<Handle(AIS_InteractiveObject), std::string> myObjectToIdMap;
    std::map<std::string, Handle(AIS_InteractiveObject)> myIdToObjectMap;
    
    // 模型变更监听
    void onModelChanged(const std::string& shapeId);
}; 