#pragma once

#include "../mvvm/Command.h"
#include "CadViewModel.h"
#include <memory>
#include <gp_Pnt.hxx>

namespace Commands {

class CreateBoxCommand : public MVVM::Command {
public:
    CreateBoxCommand(std::shared_ptr<CadViewModel> viewModel,
                    const gp_Pnt& location, 
                    double sizeX, double sizeY, double sizeZ)
        : myViewModel(viewModel), myLocation(location), 
          mySizeX(sizeX), mySizeY(sizeY), mySizeZ(sizeZ) {}
    
    void execute() override {
        myViewModel->createBox(myLocation, mySizeX, mySizeY, mySizeZ);
    }
    
private:
    std::shared_ptr<CadViewModel> myViewModel;
    gp_Pnt myLocation;
    double mySizeX, mySizeY, mySizeZ;
};

class CreateConeCommand : public MVVM::Command {
public:
    CreateConeCommand(std::shared_ptr<CadViewModel> viewModel,
                     const gp_Pnt& location, 
                     double radius, double height)
        : myViewModel(viewModel), myLocation(location), 
          myRadius(radius), myHeight(height) {}
    
    void execute() override {
        myViewModel->createCone(myLocation, myRadius, myHeight);
    }
    
private:
    std::shared_ptr<CadViewModel> myViewModel;
    gp_Pnt myLocation;
    double myRadius, myHeight;
};

class DeleteSelectedCommand : public MVVM::Command {
public:
    DeleteSelectedCommand(std::shared_ptr<CadViewModel> viewModel)
        : myViewModel(viewModel) {}
    
    void execute() override {
        myViewModel->deleteSelectedObjects();
    }
    
    bool canExecute() const override {
        return myViewModel->hasSelection();
    }
    
private:
    std::shared_ptr<CadViewModel> myViewModel;
};

class SetColorCommand : public MVVM::Command {
public:
    SetColorCommand(std::shared_ptr<CadViewModel> viewModel,
                   const Quantity_Color& color)
        : myViewModel(viewModel), myColor(color) {}
    
    void execute() override {
        myViewModel->setSelectedColor(myColor);
    }
    
    bool canExecute() const override {
        return myViewModel->hasSelection();
    }
    
private:
    std::shared_ptr<CadViewModel> myViewModel;
    Quantity_Color myColor;
};

} // namespace Commands 