#pragma once

#include "../model/CadModel.h"
#include "../mvvm/Observable.h"
#include <AIS_InteractiveContext.hxx>
#include <AIS_InteractiveObject.hxx>
#include <AIS_Shape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <memory>
#include <set>
#include <string>
#include <map>

class CadViewModel {
public:
    CadViewModel(std::shared_ptr<CadModel> model, 
                Handle(AIS_InteractiveContext) context);
    ~CadViewModel() = default;
    
    // 命令 - 对应UI操作
    void createBox(const gp_Pnt& location, double sizeX, double sizeY, double sizeZ);
    void createCone(const gp_Pnt& location, double radius, double height);
    void deleteSelectedObjects();
    
    // UI状态
    bool hasSelection() const;
    std::vector<std::string> getSelectedObjects() const;
    
    // 可观察属性
    MVVM::Observable<bool> isGridVisible{true};
    MVVM::Observable<bool> isViewCubeVisible{true};
    MVVM::Observable<int> displayMode{0}; // 0: Shaded, 1: Wireframe, 2: Vertices...
    
    // 属性访问与修改
    void setSelectedColor(const Quantity_Color& color);
    Quantity_Color getSelectedColor() const;
    
    // 选择处理
    void processSelection(const Handle(AIS_InteractiveObject)& obj, bool isSelected);
    void clearSelection();
    
    // 访问器
    Handle(AIS_InteractiveContext) getContext() const { return myContext; }
    std::shared_ptr<CadModel> getModel() const { return myModel; }
    Handle(V3d_Viewer) getViewer() const {return myContext->CurrentViewer();}
    
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