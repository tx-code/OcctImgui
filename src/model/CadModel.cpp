#include "CadModel.h"
#include <algorithm>

// IModel接口实现
std::vector<std::string> CadModel::getAllEntityIds() const {
    return getAllShapeIds();
}

void CadModel::removeEntity(const std::string& id) {
    removeShape(id);
}

// 原有CadModel实现
TopoDS_Shape CadModel::getShape(const std::string& id) const {
    auto it = myShapes.find(id);
    if (it != myShapes.end()) {
        return it->second;
    }
    return TopoDS_Shape();
}

void CadModel::addShape(const std::string& id, const TopoDS_Shape& shape) {
    myShapes[id] = shape;
    
    // 默认颜色
    if (myColors.find(id) == myColors.end()) {
        myColors[id] = Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB);
    }
    
    notifyChange(id);
}

void CadModel::removeShape(const std::string& id) {
    myShapes.erase(id);
    myColors.erase(id);
    notifyChange(id);
}

std::vector<std::string> CadModel::getAllShapeIds() const {
    std::vector<std::string> ids;
    ids.reserve(myShapes.size());
    
    for (const auto& pair : myShapes) {
        ids.push_back(pair.first);
    }
    
    return ids;
}

void CadModel::setColor(const std::string& shapeId, const Quantity_Color& color) {
    if (myShapes.find(shapeId) != myShapes.end()) {
        myColors[shapeId] = color;
        notifyChange(shapeId);
    }
}

Quantity_Color CadModel::getColor(const std::string& shapeId) const {
    auto it = myColors.find(shapeId);
    if (it != myColors.end()) {
        return it->second;
    }
    return Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB); // 默认灰色
}

void CadModel::translate(const std::string& shapeId, const gp_Vec& vector) {
    // 在实际应用中，需要实现几何变换逻辑
    // 此处仅作为示例，实际实现需要使用BRepBuilderAPI_Transform等OCCT API
    
    notifyChange(shapeId);
}

void CadModel::rotate(const std::string& shapeId, const gp_Ax1& axis, double angle) {
    // 在实际应用中，需要实现几何变换逻辑
    // 此处仅作为示例，实际实现需要使用BRepBuilderAPI_Transform等OCCT API
    
    notifyChange(shapeId);
} 