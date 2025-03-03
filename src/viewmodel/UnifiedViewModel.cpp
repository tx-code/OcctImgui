#include "UnifiedViewModel.h"
#include <AIS_Shape.hxx>
#include <AIS_Triangulation.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <TopoDS_Builder.hxx>
#include <Precision.hxx>
#include <algorithm>
#include <random>

#include <iostream>

// 构造函数
UnifiedViewModel::UnifiedViewModel(std::shared_ptr<UnifiedModel> model, 
                                Handle(AIS_InteractiveContext) context,
                                MVVM::GlobalSettings& globalSettings)
    : myModel(model), myContext(context), myGlobalSettings(globalSettings) {
    
    // 注册模型变更监听器
    model->addChangeListener([this](const std::string& id) {
        this->onModelChanged(id);
    });
    
    // 初始化现有几何体的显示
    for (const std::string& id : model->getAllEntityIds()) {
        updatePresentation(id);
    }
}

// 命令 - CAD几何操作
void UnifiedViewModel::createBox(const gp_Pnt& location, double sizeX, double sizeY, double sizeZ) {
    // 生成唯一ID
    static int boxCounter = 0;
    std::string id = "box_" + std::to_string(++boxCounter);
    
    // 创建盒子几何体
    BRepPrimAPI_MakeBox boxMaker(location, sizeX, sizeY, sizeZ);
    TopoDS_Shape boxShape = boxMaker.Shape();
    
    // 添加到模型
    myModel->addShape(id, boxShape);
}

void UnifiedViewModel::createCone(const gp_Pnt& location, double radius, double height) {
    // 生成唯一ID
    static int coneCounter = 0;
    std::string id = "cone_" + std::to_string(++coneCounter);
    
    // 创建圆锥几何体
    gp_Ax2 axis(location, gp_Dir(0, 0, 1));
    BRepPrimAPI_MakeCone coneMaker(axis, radius, 0, height);
    TopoDS_Shape coneShape = coneMaker.Shape();
    
    // 添加到模型
    myModel->addShape(id, coneShape);
}

void UnifiedViewModel::createMesh(/* 网格创建参数 */) {
    // 这个方法需要根据实际的网格创建需求来实现
    // 以下为示例代码，应该根据实际情况修改
    
    /*
    // 生成唯一ID
    static int meshCounter = 0;
    std::string id = "mesh_" + std::to_string(++meshCounter);
    
    // 创建三角形网格 (示例)
    Handle(Poly_Triangulation) mesh = new Poly_Triangulation(numVertices, numTriangles, false);
    
    // 设置顶点和三角形
    // ... 填充网格数据 ...
    
    // 添加到模型
    myModel->addMesh(id, mesh);
    */
}

// IViewModel接口实现
void UnifiedViewModel::deleteSelectedObjects() {
    std::vector<std::string> objectsToDelete(mySelectedObjects.begin(), mySelectedObjects.end());
    
    for (const std::string& id : objectsToDelete) {
        myModel->removeEntity(id);
    }
    
    mySelectedObjects.clear();
}

bool UnifiedViewModel::hasSelection() const {
    return !mySelectedObjects.empty();
}

std::vector<std::string> UnifiedViewModel::getSelectedObjects() const {
    return std::vector<std::string>(mySelectedObjects.begin(), mySelectedObjects.end());
}

void UnifiedViewModel::processSelection(const Handle(AIS_InteractiveObject)& obj, bool isSelected) {
    auto it = myObjectToIdMap.find(obj);
    if (it != myObjectToIdMap.end()) {
        if (isSelected) {
            mySelectedObjects.insert(it->second);
        } else {
            mySelectedObjects.erase(it->second);
        }
    }
}

void UnifiedViewModel::clearSelection() {
    mySelectedObjects.clear();
    myContext->ClearSelected(Standard_True);
}

// 属性访问与修改
void UnifiedViewModel::setSelectedColor(const Quantity_Color& color) {
    for (const std::string& id : mySelectedObjects) {
        myModel->setColor(id, color);
    }
}

Quantity_Color UnifiedViewModel::getSelectedColor() const {
    if (mySelectedObjects.empty()) {
        return Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB); // 默认灰色
    }
    
    // 返回第一个选中对象的颜色
    return myModel->getColor(*mySelectedObjects.begin());
}

// 私有方法
void UnifiedViewModel::updatePresentation(const std::string& id) {
    // 删除现有表示
    auto it = myIdToObjectMap.find(id);
    if (it != myIdToObjectMap.end()) {
        myContext->Remove(it->second, false);
        myObjectToIdMap.erase(it->second);
        myIdToObjectMap.erase(it);
    }
    
    // 获取几何数据
    const UnifiedModel::GeometryData* data = myModel->getGeometryData(id);
    if (!data) {
        return;
    }
    
    // 创建新表示
    Handle(AIS_InteractiveObject) aisObj = createPresentationForGeometry(id, data);
    if (aisObj.IsNull()) {
        return;
    }
    
    // 显示对象
    myContext->Display(aisObj, false);
    
    // 更新映射
    myIdToObjectMap[id] = aisObj;
    myObjectToIdMap[aisObj] = id;
}

Handle(AIS_InteractiveObject) UnifiedViewModel::createPresentationForGeometry(
    const std::string& id, const UnifiedModel::GeometryData* data) {
    
    if (!data) {
        return nullptr;
    }
    
    Handle(AIS_InteractiveObject) aisObj;
    
    // 根据几何类型创建适当的AIS对象
    if (data->type == UnifiedModel::GeometryType::SHAPE) {
        // 为CAD形体创建AIS_Shape
        const TopoDS_Shape& shape = std::get<TopoDS_Shape>(data->geometry);
        Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
        aisShape->SetColor(data->color);
        
        // 根据displayMode设置显示模式
        switch (displayMode.get()) {
            case 0: // Shaded
                aisShape->SetDisplayMode(AIS_Shaded);
                break;
            case 1: // Wireframe
                aisShape->SetDisplayMode(AIS_WireFrame);
                break;
            // 可以添加更多显示模式
        }
        
        aisObj = aisShape;
    }
    else if (data->type == UnifiedModel::GeometryType::MESH) {
        // 为网格创建AIS_Triangulation
        const Handle(Poly_Triangulation)& mesh = std::get<Handle(Poly_Triangulation)>(data->geometry);
        Handle(AIS_Triangulation) aisTriangulation = new AIS_Triangulation(mesh);
        aisTriangulation->SetColor(data->color);
        
        // 设置网格特定的显示属性
        // ...
        
        aisObj = aisTriangulation;
    }
    
    return aisObj;
}

void UnifiedViewModel::onModelChanged(const std::string& id) {
    updatePresentation(id);
} 