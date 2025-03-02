#pragma once

#include "IModel.h"
#include <map>
#include <string>
#include <vector>
#include <memory>

#include <TopoDS_Shape.hxx>
#include <Quantity_Color.hxx>
#include <gp_Vec.hxx>
#include <gp_Ax1.hxx>

class CadModel : public IModel {
public:
    CadModel() = default;
    ~CadModel() override = default;
    
    // IModel接口实现
    std::vector<std::string> getAllEntityIds() const override;
    void removeEntity(const std::string& id) override;
    
    // 基础几何数据管理
    TopoDS_Shape getShape(const std::string& id) const;
    void addShape(const std::string& id, const TopoDS_Shape& shape);
    void removeShape(const std::string& id);
    std::vector<std::string> getAllShapeIds() const;
    
    // 模型属性
    void setColor(const std::string& shapeId, const Quantity_Color& color);
    Quantity_Color getColor(const std::string& shapeId) const;
    
    // 模型操作
    void translate(const std::string& shapeId, const gp_Vec& vector);
    void rotate(const std::string& shapeId, const gp_Ax1& axis, double angle);
    
private:
    std::map<std::string, TopoDS_Shape> myShapes;
    std::map<std::string, Quantity_Color> myColors;
}; 