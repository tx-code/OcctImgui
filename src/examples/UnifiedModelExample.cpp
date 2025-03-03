#include "../model/UnifiedModel.h"
#include "../viewmodel/UnifiedViewModel.h"
#include <V3d_Viewer.hxx>
#include <AIS_InteractiveContext.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <memory>
#include <iostream>

// 辅助函数 - 从OCCT形体提取三角网格
Handle(Poly_Triangulation) extractMeshFromShape(const TopoDS_Shape& shape) {
    // 对形体进行网格剖分
    BRepMesh_IncrementalMesh mesh(shape, 0.1);
    
    // 遍历形体的面
    TopExp_Explorer explorer(shape, TopAbs_FACE);
    if (explorer.More()) {
        TopoDS_Face face = TopoDS::Face(explorer.Current());
        TopLoc_Location loc;
        Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, loc);
        return tri;
    }
    
    return nullptr;
}

void runUnifiedModelExample() {
    std::cout << "Running Unified Model Example..." << std::endl;
    
    // 创建模型
    auto model = std::make_shared<UnifiedModel>();
    
    // 创建OpenCascade查看器和交互上下文
    Handle(V3d_Viewer) viewer = new V3d_Viewer(/*...*/);
    Handle(AIS_InteractiveContext) context = new AIS_InteractiveContext(viewer);
    
    // 创建ViewModel
    auto viewModel = std::make_shared<UnifiedViewModel>(model, context);
    
    // 示例1：添加CAD形体
    BRepPrimAPI_MakeBox boxMaker(gp_Pnt(0, 0, 0), 10, 10, 10);
    TopoDS_Shape boxShape = boxMaker.Shape();
    model->addShape("box1", boxShape);
    
    // 设置颜色
    model->setColor("box1", Quantity_Color(1, 0, 0, Quantity_TOC_RGB)); // 红色
    
    // 示例2：添加另一个CAD形体
    BRepPrimAPI_MakeCylinder cylinderMaker(gp_Ax2(gp_Pnt(20, 0, 0), gp_Dir(0, 0, 1)), 5, 15);
    TopoDS_Shape cylinderShape = cylinderMaker.Shape();
    model->addShape("cylinder1", cylinderShape);
    
    // 设置颜色
    model->setColor("cylinder1", Quantity_Color(0, 1, 0, Quantity_TOC_RGB)); // 绿色
    
    // 示例3：从CAD形体提取并添加网格数据
    Handle(Poly_Triangulation) mesh = extractMeshFromShape(boxShape);
    if (!mesh.IsNull()) {
        model->addMesh("mesh1", mesh);
        model->setColor("mesh1", Quantity_Color(0, 0, 1, Quantity_TOC_RGB)); // 蓝色
    }
    
    // 示例4：转换几何体
    gp_Trsf transformation;
    transformation.SetTranslation(gp_Vec(0, 20, 0));
    model->transform("box1", transformation);
    
    // 检索并打印所有几何体ID
    std::cout << "所有几何体ID:" << std::endl;
    for (const auto& id : model->getAllEntityIds()) {
        std::cout << "  - " << id;
        
        // 获取几何类型
        UnifiedModel::GeometryType type = model->getGeometryType(id);
        if (type == UnifiedModel::GeometryType::SHAPE) {
            std::cout << " (CAD形体)";
        } else if (type == UnifiedModel::GeometryType::MESH) {
            std::cout << " (多边形网格)";
        }
        
        // 获取颜色
        Quantity_Color color = model->getColor(id);
        std::cout << " 颜色: RGB(" 
                 << color.Red() << ", " 
                 << color.Green() << ", " 
                 << color.Blue() << ")" 
                 << std::endl;
    }
    
    // 从视图模型访问模型
    std::shared_ptr<UnifiedModel> modelFromVM = viewModel->getUnifiedModel();
    std::cout << "通过ViewModel访问的模型中的几何体数量: " 
             << modelFromVM->getAllEntityIds().size() << std::endl;
    
    std::cout << "Unified Model Example完成." << std::endl;
} 