#include "PolyModel.h"
#include <algorithm>

// IModel接口实现
std::vector<std::string> PolyModel::getAllEntityIds() const {
    return getAllMeshIds();
}

void PolyModel::removeEntity(const std::string& id) {
    removeMesh(id);
}

// PolyModel特定功能实现
Handle(Poly_Triangulation) PolyModel::getMesh(const std::string& id) const {
    auto it = myMeshes.find(id);
    if (it != myMeshes.end()) {
        return it->second;
    }
    return nullptr;
}

void PolyModel::addMesh(const std::string& id, const Handle(Poly_Triangulation)& mesh) {
    myMeshes[id] = mesh;
    
    // 默认颜色
    if (myColors.find(id) == myColors.end()) {
        myColors[id] = Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB);
    }
    
    notifyChange(id);
}

void PolyModel::removeMesh(const std::string& id) {
    myMeshes.erase(id);
    myColors.erase(id);
    notifyChange(id);
}

std::vector<std::string> PolyModel::getAllMeshIds() const {
    std::vector<std::string> ids;
    ids.reserve(myMeshes.size());
    
    for (const auto& pair : myMeshes) {
        ids.push_back(pair.first);
    }
    
    return ids;
}

void PolyModel::setColor(const std::string& meshId, const Quantity_Color& color) {
    if (myMeshes.find(meshId) != myMeshes.end()) {
        myColors[meshId] = color;
        notifyChange(meshId);
    }
}

Quantity_Color PolyModel::getColor(const std::string& meshId) const {
    auto it = myColors.find(meshId);
    if (it != myColors.end()) {
        return it->second;
    }
    return Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB); // 默认灰色
}

void PolyModel::transform(const std::string& meshId, const gp_Trsf& transformation) {
    // 在实际应用中，需要实现网格变换逻辑
    // 此处仅作为示例，实际实现需要遍历并变换网格的所有顶点
    
    notifyChange(meshId);
} 