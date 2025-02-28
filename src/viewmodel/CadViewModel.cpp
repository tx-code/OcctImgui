#include "CadViewModel.h"
#include "../mvvm/MessageBus.h"
#include <algorithm>
#include <sstream>

CadViewModel::CadViewModel(std::shared_ptr<CadModel> model, Handle(AIS_InteractiveContext) context)
    : myModel(model), myContext(context) {
    
    // 注册模型变更监听
    model->addChangeListener([this](const std::string& shapeId) {
        onModelChanged(shapeId);
    });
}

void CadViewModel::createBox(const gp_Pnt& location, double sizeX, double sizeY, double sizeZ) {
    // 创建形状
    gp_Ax2 axis;
    axis.SetLocation(location);
    TopoDS_Shape boxShape = BRepPrimAPI_MakeBox(axis, sizeX, sizeY, sizeZ).Shape();
    
    // 生成唯一ID
    std::stringstream ss;
    ss << "box_" << location.X() << "_" << location.Y() << "_" << location.Z() << "_" << myModel->getAllShapeIds().size();
    std::string shapeId = ss.str();
    
    // 添加到模型
    myModel->addShape(shapeId, boxShape);
}

void CadViewModel::createCone(const gp_Pnt& location, double radius, double height) {
    // 创建形状
    gp_Ax2 axis;
    axis.SetLocation(location);
    TopoDS_Shape coneShape = BRepPrimAPI_MakeCone(axis, radius, 0, height).Shape();
    
    // 生成唯一ID
    std::stringstream ss;
    ss << "cone_" << location.X() << "_" << location.Y() << "_" << location.Z() << "_" << myModel->getAllShapeIds().size();
    std::string shapeId = ss.str();
    
    // 添加到模型
    myModel->addShape(shapeId, coneShape);
}

void CadViewModel::deleteSelectedObjects() {
    auto selectedObjs = getSelectedObjects();
    for (const auto& id : selectedObjs) {
        myModel->removeShape(id);
    }
    clearSelection();
}

bool CadViewModel::hasSelection() const {
    return !mySelectedObjects.empty();
}

std::vector<std::string> CadViewModel::getSelectedObjects() const {
    std::vector<std::string> result;
    result.reserve(mySelectedObjects.size());
    
    for (const auto& id : mySelectedObjects) {
        result.push_back(id);
    }
    
    return result;
}

void CadViewModel::setSelectedColor(const Quantity_Color& color) {
    for (const auto& id : mySelectedObjects) {
        myModel->setColor(id, color);
    }
}

Quantity_Color CadViewModel::getSelectedColor() const {
    if (mySelectedObjects.empty()) {
        return Quantity_Color(0.8, 0.8, 0.8, Quantity_TOC_RGB);
    }
    
    // 返回第一个选中对象的颜色
    return myModel->getColor(*mySelectedObjects.begin());
}

void CadViewModel::processSelection(const Handle(AIS_InteractiveObject)& obj, bool isSelected) {
    auto it = myObjectToIdMap.find(obj);
    if (it != myObjectToIdMap.end()) {
        std::string shapeId = it->second;
        
        if (isSelected) {
            mySelectedObjects.insert(shapeId);
        } else {
            mySelectedObjects.erase(shapeId);
        }
        
        // 发布选择变更消息
        MVVM::MessageBus::getInstance().publish(MVVM::MessageBus::Message {
            MVVM::MessageBus::MessageType::SelectionChanged,
            std::make_any<std::vector<std::string>>(getSelectedObjects())
        });
    }
}

void CadViewModel::clearSelection() {
    mySelectedObjects.clear();
    myContext->ClearSelected(Standard_False);
    
    // 发布选择变更消息
    MVVM::MessageBus::getInstance().publish(MVVM::MessageBus::Message {
        MVVM::MessageBus::MessageType::SelectionChanged,
        std::make_any<std::vector<std::string>>(getSelectedObjects())
    });
}

void CadViewModel::updatePresentation(const std::string& shapeId) {
    auto shape = myModel->getShape(shapeId);
    
    if (shape.IsNull()) {
        // 移除表示
        auto it = myIdToObjectMap.find(shapeId);
        if (it != myIdToObjectMap.end()) {
            myContext->Remove(it->second, Standard_False);
            myObjectToIdMap.erase(it->second);
            myIdToObjectMap.erase(it);
        }
    } else {
        // 更新或创建表示
        Handle(AIS_Shape) aisShape;
        auto it = myIdToObjectMap.find(shapeId);
        
        if (it != myIdToObjectMap.end()) {
            aisShape = Handle(AIS_Shape)::DownCast(it->second);
            aisShape->SetShape(shape);
        } else {
            aisShape = new AIS_Shape(shape);
            myIdToObjectMap[shapeId] = aisShape;
            myObjectToIdMap[aisShape] = shapeId;
        }
        
        // 设置颜色
        Quantity_Color color = myModel->getColor(shapeId);
        aisShape->SetColor(color);
        
        // 显示
        myContext->Display(aisShape, Standard_False);
    }
    
    myContext->UpdateCurrentViewer();
}

void CadViewModel::onModelChanged(const std::string& shapeId) {
    updatePresentation(shapeId);
    
    // 发布模型变更消息
    MVVM::MessageBus::getInstance().publish(MVVM::MessageBus::Message {
        MVVM::MessageBus::MessageType::ModelChanged,
        std::make_any<std::string>(shapeId)
    });
} 