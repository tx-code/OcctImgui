#include "PolyViewModel.h"
#include "../mvvm/MessageBus.h"
#include <algorithm>
#include <sstream>
#include <TColgp_Array1OfPnt.hxx>
#include <Poly_Array1OfTriangle.hxx>
#include <Poly_Triangulation.hxx>
#include <AIS_Triangulation.hxx>

PolyViewModel::PolyViewModel(std::shared_ptr<PolyModel> model, Handle(AIS_InteractiveContext) context)
    : myModel(model), myContext(context) {
    
    // 注册模型变更监听
    model->addChangeListener([this](const std::string& meshId) {
        onModelChanged(meshId);
    });
}

void PolyViewModel::createTriangle(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3) {
    // 创建三角网格
    Handle(Poly_Triangulation) mesh = new Poly_Triangulation(3, 1, false);
    
    // 设置顶点
    auto nodes = mesh->MapNodeArray();
    nodes->SetValue(1, p1);
    nodes->SetValue(2, p2);
    nodes->SetValue(3, p3);
    
    // 设置三角形
    Poly_Array1OfTriangle& triangles = mesh->ChangeTriangles();
    triangles(1) = Poly_Triangle(1, 2, 3);
    
    // 生成唯一ID
    std::stringstream ss;
    ss << "triangle_" << myModel->getAllMeshIds().size();
    std::string meshId = ss.str();
    
    // 添加到模型
    myModel->addMesh(meshId, mesh);
}

void PolyViewModel::importMesh(const std::string& filePath) {
    // 实际应用中，这里应该实现从文件导入网格的逻辑
    // 此处仅作为示例
    
    // 创建一个简单的网格作为示例
    Handle(Poly_Triangulation) mesh = new Poly_Triangulation(4, 2, false);
    
    // 设置顶点
    auto nodes = mesh->MapNodeArray();
    nodes->SetValue(1, gp_Pnt(0, 0, 0));
    nodes->SetValue(2, gp_Pnt(10, 0, 0));
    nodes->SetValue(3, gp_Pnt(10, 10, 0));
    nodes->SetValue(4, gp_Pnt(0, 10, 0));
    
    // 设置三角形
    Poly_Array1OfTriangle& triangles = mesh->ChangeTriangles();
    triangles(1) = Poly_Triangle(1, 2, 3);
    triangles(2) = Poly_Triangle(1, 3, 4);
    
    // 生成唯一ID
    std::string meshId = "imported_mesh_" + std::to_string(myModel->getAllMeshIds().size());
    
    // 添加到模型
    myModel->addMesh(meshId, mesh);
}

void PolyViewModel::deleteSelectedObjects() {
    auto selectedObjs = getSelectedObjects();
    for (const auto& id : selectedObjs) {
        myModel->removeMesh(id);
    }
    clearSelection();
}

bool PolyViewModel::hasSelection() const {
    return !mySelectedObjects.empty();
}

std::vector<std::string> PolyViewModel::getSelectedObjects() const {
    return std::vector<std::string>(mySelectedObjects.begin(), mySelectedObjects.end());
}

void PolyViewModel::setSelectedColor(const Quantity_Color& color) {
    for (const auto& id : mySelectedObjects) {
        myModel->setColor(id, color);
    }
}

Quantity_Color PolyViewModel::getSelectedColor() const {
    if (mySelectedObjects.empty()) {
        return Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB); // 默认灰色
    }
    
    // 返回第一个选中对象的颜色
    return myModel->getColor(*mySelectedObjects.begin());
}

void PolyViewModel::processSelection(const Handle(AIS_InteractiveObject)& obj, bool isSelected) {
    auto it = myObjectToIdMap.find(obj);
    if (it != myObjectToIdMap.end()) {
        if (isSelected) {
            mySelectedObjects.insert(it->second);
        } else {
            mySelectedObjects.erase(it->second);
        }
    }
}

void PolyViewModel::clearSelection() {
    mySelectedObjects.clear();
    myContext->ClearSelected(false);
}

void PolyViewModel::updatePresentation(const std::string& meshId) {
    // 获取网格
    Handle(Poly_Triangulation) mesh = myModel->getMesh(meshId);
    if (mesh.IsNull()) return;
    
    // 检查是否已有表示
    auto it = myIdToObjectMap.find(meshId);
    if (it != myIdToObjectMap.end()) {
        // 更新现有表示
        myContext->Remove(it->second, false);
        myObjectToIdMap.erase(it->second);
        myIdToObjectMap.erase(it);
    }
    
    // 创建新的表示
    Handle(AIS_Triangulation) aisTriangulation = new AIS_Triangulation(mesh);
    
    // 设置颜色
    Quantity_Color color = myModel->getColor(meshId);
    aisTriangulation->SetColor(color);
    
    // 添加到上下文
    myContext->Display(aisTriangulation, false);
    
    // 更新映射
    myObjectToIdMap[aisTriangulation] = meshId;
    myIdToObjectMap[meshId] = aisTriangulation;
}

void PolyViewModel::onModelChanged(const std::string& meshId) {
    updatePresentation(meshId);
} 