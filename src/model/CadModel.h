#pragma once

#include <map>
#include <string>
#include <functional>
#include <vector>
#include <memory>

#include <TopoDS_Shape.hxx>
#include <Quantity_Color.hxx>
#include <gp_Vec.hxx>
#include <gp_Ax1.hxx>

class CadModel {
public:
    CadModel() = default;
    ~CadModel() = default;
    
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
    
    // 事件通知
    void addChangeListener(std::function<void(const std::string&)> listener);
    
private:
    std::map<std::string, TopoDS_Shape> myShapes;
    std::map<std::string, Quantity_Color> myColors;
    std::vector<std::function<void(const std::string&)>> myChangeListeners;
    
    void notifyChange(const std::string& shapeId);
}; 