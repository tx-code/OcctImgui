#pragma once

#include "IViewModel.h"
#include "UnifiedViewModel.h"
#include <Quantity_Color.hxx>
#include <gp_Pnt.hxx>
#include <memory>

namespace Commands {

// 基础命令类
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
};

// 通用命令 - 适用于任何ViewModel

// 删除选中对象命令
class DeleteSelectedCommand : public Command {
public:
    DeleteSelectedCommand(std::shared_ptr<IViewModel> viewModel)
        : myViewModel(viewModel) {}
    
    void execute() override {
        myViewModel->deleteSelectedObjects();
    }
    
private:
    std::shared_ptr<IViewModel> myViewModel;
};

// 设置颜色命令 - 需要根据ViewModel类型分别处理
class SetColorCommand : public Command {
public:
    SetColorCommand(std::shared_ptr<IViewModel> viewModel, const Quantity_Color& color)
        : myViewModel(viewModel), myColor(color) {}
    
    void execute() override {
        auto unifiedViewModel = std::dynamic_pointer_cast<UnifiedViewModel>(myViewModel);
        if (unifiedViewModel) {
            unifiedViewModel->setSelectedColor(myColor);
            return;
        }
    }
    
private:
    std::shared_ptr<IViewModel> myViewModel;
    Quantity_Color myColor;
};

// CadViewModel特定命令

// 创建盒子命令
class CreateBoxCommand : public Command {
public:
    CreateBoxCommand(std::shared_ptr<UnifiedViewModel> viewModel, 
                    const gp_Pnt& location, 
                    double sizeX, double sizeY, double sizeZ)
        : myViewModel(viewModel), myLocation(location), 
          mySizeX(sizeX), mySizeY(sizeY), mySizeZ(sizeZ) {}
    
    void execute() override {
        myViewModel->createBox(myLocation, mySizeX, mySizeY, mySizeZ);
    }
    
private:
    std::shared_ptr<UnifiedViewModel> myViewModel;
    gp_Pnt myLocation;
    double mySizeX, mySizeY, mySizeZ;
};

// 创建圆锥命令
class CreateConeCommand : public Command {
public:
    CreateConeCommand(std::shared_ptr<UnifiedViewModel> viewModel, 
                     const gp_Pnt& location, 
                     double radius, double height)
        : myViewModel(viewModel), myLocation(location), 
          myRadius(radius), myHeight(height) {}
    
    void execute() override {
        myViewModel->createCone(myLocation, myRadius, myHeight);
    }
    
private:
    std::shared_ptr<UnifiedViewModel> myViewModel;
    gp_Pnt myLocation;
    double myRadius, myHeight;
};

// 导入模型命令 - 支持多种格式（STEP, STL, OBJ等）
class ImportModelCommand : public Command {
public:
    ImportModelCommand(std::shared_ptr<UnifiedViewModel> viewModel, 
                      const std::string& filePath,
                      const std::string& modelId = "")
        : myViewModel(viewModel), myFilePath(filePath), myModelId(modelId) {}
    
    void execute() override {
        myViewModel->importModel(myFilePath, myModelId);
    }
    
private:
    std::shared_ptr<UnifiedViewModel> myViewModel;
    std::string myFilePath;
    std::string myModelId;
};

} // namespace Commands 