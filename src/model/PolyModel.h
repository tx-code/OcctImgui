#pragma once

#include "IModel.h"
#include <map>
#include <string>
#include <vector>
#include <memory>

#include <Poly_Triangulation.hxx>
#include <Quantity_Color.hxx>
#include <gp_Trsf.hxx>

class PolyModel : public IModel {
public:
    PolyModel() = default;
    ~PolyModel() override = default;
    
    // IModel接口实现
    std::vector<std::string> getAllEntityIds() const override;
    void removeEntity(const std::string& id) override;
    
    // PolyModel特定功能
    Handle(Poly_Triangulation) getMesh(const std::string& id) const;
    void addMesh(const std::string& id, const Handle(Poly_Triangulation)& mesh);
    void removeMesh(const std::string& id);
    std::vector<std::string> getAllMeshIds() const;
    
    // 模型属性
    void setColor(const std::string& meshId, const Quantity_Color& color);
    Quantity_Color getColor(const std::string& meshId) const;
    
    // 网格操作
    void transform(const std::string& meshId, const gp_Trsf& transformation);
    
private:
    std::map<std::string, Handle(Poly_Triangulation)> myMeshes;
    std::map<std::string, Quantity_Color> myColors;
}; 