#include "../model/ModelManager.h"
#include "../model/CadModel.h"
#include "../model/PolyModel.h"
#include "../viewmodel/ViewModelManager.h"
#include "../viewmodel/CadViewModel.h"
#include "../viewmodel/PolyViewModel.h"

#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <Poly_Triangulation.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <Poly_Array1OfTriangle.hxx>
#include <iostream>

// 这个函数展示了如何使用新的MVVM框架
void modelUsageExample(Handle(AIS_InteractiveContext) context) {
    // 1. 使用ModelManager创建模型
    auto& modelManager = ModelManager::instance();
    
    // 创建CAD模型
    auto cadModel = modelManager.createModel<CadModel>("main_cad_model");
    
    // 创建多边形模型
    auto polyModel = modelManager.createModel<PolyModel>("main_poly_model");
    
    // 2. 使用ViewModelManager创建ViewModel
    auto& viewModelManager = ViewModelManager::instance();
    
    // 创建CAD ViewModel
    auto cadViewModel = viewModelManager.createViewModel<CadViewModel, CadModel>(
        "main_cad_viewmodel", "main_cad_model", context);
    
    // 创建Poly ViewModel
    auto polyViewModel = viewModelManager.createViewModel<PolyViewModel, PolyModel>(
        "main_poly_viewmodel", "main_poly_model", context);
    
    // 3. 使用模型和视图模型
    
    // 3.1 使用CAD模型和视图模型
    std::cout << "创建CAD对象..." << std::endl;
    
    // 创建一个盒子
    cadViewModel->createBox(gp_Pnt(0, 0, 0), 10, 10, 10);
    
    // 创建一个圆锥
    cadViewModel->createCone(gp_Pnt(20, 0, 0), 5, 15);
    
    // 获取所有形状ID
    auto shapeIds = cadModel->getAllShapeIds();
    std::cout << "CAD模型中的形状数量: " << shapeIds.size() << std::endl;
    
    // 3.2 使用Poly模型和视图模型
    std::cout << "创建多边形对象..." << std::endl;
    
    // 创建一个三角形
    polyViewModel->createTriangle(
        gp_Pnt(0, 0, 20),
        gp_Pnt(10, 0, 20),
        gp_Pnt(5, 10, 20)
    );
    
    // 导入一个网格
    polyViewModel->importMesh("dummy_path.stl");
    
    // 获取所有网格ID
    auto meshIds = polyModel->getAllMeshIds();
    std::cout << "Poly模型中的网格数量: " << meshIds.size() << std::endl;
    
    // 4. 使用通用接口
    
    // 获取所有模型ID
    auto modelIds = modelManager.getAllModelIds();
    std::cout << "模型管理器中的模型数量: " << modelIds.size() << std::endl;
    
    // 获取所有视图模型ID
    auto viewModelIds = viewModelManager.getAllViewModelIds();
    std::cout << "视图模型管理器中的视图模型数量: " << viewModelIds.size() << std::endl;
    
    // 通过ID获取模型
    auto model = modelManager.getModel("main_cad_model");
    if (model) {
        std::cout << "成功获取模型: main_cad_model" << std::endl;
        std::cout << "实体数量: " << model->getAllEntityIds().size() << std::endl;
    }
    
    // 通过ID获取视图模型
    auto viewModel = viewModelManager.getViewModel("main_poly_viewmodel");
    if (viewModel) {
        std::cout << "成功获取视图模型: main_poly_viewmodel" << std::endl;
        
        // 转换为特定类型
        auto specificViewModel = std::dynamic_pointer_cast<PolyViewModel>(viewModel);
        if (specificViewModel) {
            std::cout << "成功转换为PolyViewModel" << std::endl;
        }
    }
    
    // 5. 事件监听示例
    std::cout << "添加事件监听器..." << std::endl;
    
    // 为CAD模型添加监听器
    cadModel->addChangeListener([](const std::string& entityId) {
        std::cout << "CAD模型变更通知: " << entityId << std::endl;
    });
    
    // 为Poly模型添加监听器
    polyModel->addChangeListener([](const std::string& entityId) {
        std::cout << "Poly模型变更通知: " << entityId << std::endl;
    });
    
    // 触发变更事件
    if (!shapeIds.empty()) {
        std::cout << "修改CAD对象颜色..." << std::endl;
        cadModel->setColor(shapeIds[0], Quantity_Color(1, 0, 0, Quantity_TOC_RGB));
    }
    
    if (!meshIds.empty()) {
        std::cout << "修改Poly对象颜色..." << std::endl;
        polyModel->setColor(meshIds[0], Quantity_Color(0, 0, 1, Quantity_TOC_RGB));
    }
    
    std::cout << "MVVM框架示例完成" << std::endl;
} 