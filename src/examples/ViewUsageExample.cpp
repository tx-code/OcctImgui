#include "../model/ModelManager.h"
#include "../model/CadModel.h"
#include "../model/PolyModel.h"
#include "../viewmodel/ViewModelManager.h"
#include "../viewmodel/CadViewModel.h"
#include "../viewmodel/PolyViewModel.h"
#include "../view/ViewManager.h"
#include "../view/ImGuiView.h"

#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <iostream>

// 这个函数展示了如何使用新的视图框架
void viewUsageExample(GLFWwindow* window, Handle(AIS_InteractiveContext) context) {
    // 1. 创建模型
    auto& modelManager = ModelManager::instance();
    auto cadModel = modelManager.createModel<CadModel>("main_cad_model");
    auto polyModel = modelManager.createModel<PolyModel>("main_poly_model");
    
    // 2. 创建视图模型
    auto& viewModelManager = ViewModelManager::instance();
    auto cadViewModel = viewModelManager.createViewModel<CadViewModel, CadModel>(
        "main_cad_viewmodel", "main_cad_model", context);
    auto polyViewModel = viewModelManager.createViewModel<PolyViewModel, PolyModel>(
        "main_poly_viewmodel", "main_poly_model", context);
    
    // 3. 创建视图
    auto& viewManager = ViewManager::instance();
    auto cadView = viewManager.createView<ImGuiView>("cad_view", "main_cad_viewmodel");
    auto polyView = viewManager.createView<ImGuiView>("poly_view", "main_poly_viewmodel");
    
    // 4. 初始化视图
    viewManager.initializeAll(window);
    
    // 5. 使用模型和视图模型
    std::cout << "创建一些示例对象..." << std::endl;
    
    // 创建CAD对象
    cadViewModel->createBox(gp_Pnt(0, 0, 0), 10, 10, 10);
    cadViewModel->createCone(gp_Pnt(20, 0, 0), 5, 10);
    
    // 创建多边形对象
    polyViewModel->createTriangle(
        gp_Pnt(0, 0, 20),
        gp_Pnt(10, 0, 20),
        gp_Pnt(5, 10, 20)
    );
    
    // 6. 主循环（在实际应用中）
    std::cout << "在实际应用中，您会在主循环中调用viewManager.renderAll()" << std::endl;
    
    // 7. 关闭视图
    std::cout << "关闭视图..." << std::endl;
    viewManager.shutdownAll();
    
    std::cout << "视图框架示例完成" << std::endl;
} 