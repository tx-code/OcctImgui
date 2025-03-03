#pragma once

#include "IModel.h"
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <variant>

#include <TopoDS_Shape.hxx>
#include <Poly_Triangulation.hxx>
#include <Quantity_Color.hxx>
#include <gp_Trsf.hxx>

class UnifiedModel : public IModel {
public:
    // 几何数据类型枚举
    enum class GeometryType {
        SHAPE,  // CAD模型 (TopoDS_Shape)
        MESH    // 多边形网格 (Poly_Triangulation)
    };
    
    // 几何数据容器
    struct GeometryData {
        std::variant<TopoDS_Shape, Handle(Poly_Triangulation)> geometry;
        Quantity_Color color;
        GeometryType type;
        
        // 默认构造函数
        GeometryData() : geometry(TopoDS_Shape()), color(Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB)), type(GeometryType::SHAPE) {}
        
        // 构造函数 - CAD形体
        GeometryData(const TopoDS_Shape& shape, const Quantity_Color& color = Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB))
            : geometry(shape), color(color), type(GeometryType::SHAPE) {}
        
        // 构造函数 - 多边形网格
        GeometryData(const Handle(Poly_Triangulation)& mesh, const Quantity_Color& color = Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB))
            : geometry(mesh), color(color), type(GeometryType::MESH) {}
    };
    
    UnifiedModel() = default;
    ~UnifiedModel() override = default;
    
    // IModel接口实现
    std::vector<std::string> getAllEntityIds() const override;
    void removeEntity(const std::string& id) override;
    
    // 几何数据管理 - CAD形体
    TopoDS_Shape getShape(const std::string& id) const;
    void addShape(const std::string& id, const TopoDS_Shape& shape);
    
    // 几何数据管理 - 多边形网格
    Handle(Poly_Triangulation) getMesh(const std::string& id) const;
    void addMesh(const std::string& id, const Handle(Poly_Triangulation)& mesh);
    
    // 通用几何数据管理
    void removeGeometry(const std::string& id);
    GeometryType getGeometryType(const std::string& id) const;
    const GeometryData* getGeometryData(const std::string& id) const;
    std::vector<std::string> getGeometryIdsByType(GeometryType type) const;
    
    // 颜色属性
    void setColor(const std::string& id, const Quantity_Color& color);
    Quantity_Color getColor(const std::string& id) const;
    
    // 几何变换 - 通用接口
    void transform(const std::string& id, const gp_Trsf& transformation);
    
private:
    std::map<std::string, GeometryData> myGeometries;
}; 