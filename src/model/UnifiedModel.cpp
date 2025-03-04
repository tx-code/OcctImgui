#include "UnifiedModel.h"
#include <algorithm>
#include <stdexcept>

// IModel接口实现
std::vector<std::string> UnifiedModel::getAllEntityIds() const {
    std::vector<std::string> ids;
    ids.reserve(myGeometries.size());
    
    for (const auto& pair : myGeometries) {
        ids.push_back(pair.first);
    }
    
    return ids;
}

void UnifiedModel::removeEntity(const std::string& id) {
    removeGeometry(id);
}

// 几何数据管理 - CAD形体
TopoDS_Shape UnifiedModel::getShape(const std::string& id) const {
    auto it = myGeometries.find(id);
    if (it != myGeometries.end() && it->second.type == GeometryType::SHAPE) {
        return std::get<TopoDS_Shape>(it->second.geometry);
    }
    return TopoDS_Shape();
}

void UnifiedModel::addShape(const std::string& id, const TopoDS_Shape& shape) {
    myGeometries.emplace(id, GeometryData(shape));
    notifyChange(id);
}

// 几何数据管理 - 多边形网格
const UnifiedModel::MeshData* UnifiedModel::getMesh(const std::string& id) const {
    auto it = myGeometries.find(id);
    if (it != myGeometries.end() && it->second.type == GeometryType::MESH) {
        return &std::get<MeshData>(it->second.geometry);
    }
    return nullptr;
}

void UnifiedModel::addMesh(const std::string& id, const Eigen::MatrixXd& vertices, const Eigen::MatrixXi& faces) {
    myGeometries.emplace(id, GeometryData(vertices, faces));
    notifyChange(id);
}

void UnifiedModel::addMesh(const std::string& id, const Eigen::MatrixXd& vertices, const Eigen::MatrixXi& faces, const Eigen::MatrixXd& normals) {
    myGeometries.emplace(id, GeometryData(vertices, faces, normals));
    notifyChange(id);
}

// 通用几何数据管理
void UnifiedModel::removeGeometry(const std::string& id) {
    myGeometries.erase(id);
    notifyChange(id);
}

UnifiedModel::GeometryType UnifiedModel::getGeometryType(const std::string& id) const {
    auto it = myGeometries.find(id);
    if (it != myGeometries.end()) {
        return it->second.type;
    }
    throw std::runtime_error("Geometry ID not found: " + id);
}

const UnifiedModel::GeometryData* UnifiedModel::getGeometryData(const std::string& id) const {
    auto it = myGeometries.find(id);
    if (it != myGeometries.end()) {
        return &(it->second);
    }
    return nullptr;
}

std::vector<std::string> UnifiedModel::getGeometryIdsByType(GeometryType type) const {
    std::vector<std::string> ids;
    
    for (const auto& pair : myGeometries) {
        if (pair.second.type == type) {
            ids.push_back(pair.first);
        }
    }
    
    return ids;
}

// 颜色属性
void UnifiedModel::setColor(const std::string& id, const Quantity_Color& color) {
    auto it = myGeometries.find(id);
    if (it != myGeometries.end()) {
        it->second.color = color;
        notifyChange(id);
    }
}

Quantity_Color UnifiedModel::getColor(const std::string& id) const {
    auto it = myGeometries.find(id);
    if (it != myGeometries.end()) {
        return it->second.color;
    }
    return Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB); // 默认灰色
}

// 几何变换 - 通用接口
void UnifiedModel::transform(const std::string& id, const gp_Trsf& transformation) {
    // 此处需要根据几何类型实现不同的变换逻辑
    auto it = myGeometries.find(id);
    if (it == myGeometries.end()) {
        return;
    }
    
    // 根据几何类型选择合适的变换方法
    if (it->second.type == GeometryType::SHAPE) {
        // 对CAD形体应用变换
        // 注意：这里需要实际使用OpenCascade的BRepBuilderAPI_Transform等API
        // 以下代码仅示意，需要根据实际需求实现
        // TopoDS_Shape& shape = std::get<TopoDS_Shape>(it->second.geometry);
        // shape = BRepBuilderAPI_Transform(shape, transformation).Shape();
    }
    else if (it->second.type == GeometryType::MESH) {
        // 对网格应用变换
        MeshData& mesh = std::get<MeshData>(it->second.geometry);
        
        // 应用变换到顶点
        for (int i = 0; i < mesh.vertices.rows(); ++i) {
            gp_XYZ pnt(mesh.vertices(i, 0), mesh.vertices(i, 1), mesh.vertices(i, 2));
            transformation.Transforms(pnt);
            mesh.vertices(i, 0) = pnt.X();
            mesh.vertices(i, 1) = pnt.Y();
            mesh.vertices(i, 2) = pnt.Z();
        }

        // 对法向量，只应用旋转部分（不包括平移）
        if (mesh.normals.rows() > 0) {
            // 提取变换的线性部分（旋转和缩放）
            gp_Mat rotMat = transformation.VectorialPart();
            
            for (int i = 0; i < mesh.normals.rows(); ++i) {
                gp_XYZ normal(mesh.normals(i, 0), mesh.normals(i, 1), mesh.normals(i, 2));
                
                // 只应用旋转部分
                normal.Multiply(rotMat);
                
                // 重新归一化法向量（如果有非均匀缩放，这一步很重要）
                double len = sqrt(normal.X()*normal.X() + normal.Y()*normal.Y() + normal.Z()*normal.Z());
                if (len > 1e-10) {
                    normal.Divide(len);
                }
                
                mesh.normals(i, 0) = normal.X();
                mesh.normals(i, 1) = normal.Y();
                mesh.normals(i, 2) = normal.Z();
            }
        }
    }
    
    notifyChange(id);
} 