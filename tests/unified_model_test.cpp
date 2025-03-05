#define BOOST_TEST_MODULE UnifiedModel Tests
#include <boost/test/unit_test.hpp>

#include "model/UnifiedModel.h"
#include <igl/per_face_normals.h>

#include <STEPControl_Reader.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS_Face.hxx>
#include <TopLoc_Location.hxx>
#include <Poly_Triangulation.hxx>
#include <Quantity_Color.hxx>
#include <gp_Trsf.hxx>

#include <iostream>
#include <string>
#include <memory>

// 辅助函数 - 从OCCT形体提取三角网格并转换为libigl格式
std::tuple<Eigen::MatrixXd, Eigen::MatrixXi, Eigen::MatrixXd> extractMeshFromShape(const TopoDS_Shape& shape) {
    // 对形体进行网格剖分
    BRepMesh_IncrementalMesh mesh(shape, 0.1);
    
    // 准备libigl格式的数据结构
    Eigen::MatrixXd V; // 顶点
    Eigen::MatrixXi F; // 面
    Eigen::MatrixXd N; // 法向量
    
    // 遍历形体的面
    TopExp_Explorer explorer(shape, TopAbs_FACE);
    if (explorer.More()) {
        TopoDS_Face face = TopoDS::Face(explorer.Current());
        TopLoc_Location loc;
        Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, loc);
        
        if (!tri.IsNull()) {
            // 分配空间
            V.resize(tri->NbNodes(), 3);
            F.resize(tri->NbTriangles(), 3);
            
            // 复制顶点
            for (int i = 1; i <= tri->NbNodes(); ++i) {
                gp_Pnt p = tri->Node(i).Transformed(loc);
                V(i-1, 0) = p.X();
                V(i-1, 1) = p.Y();
                V(i-1, 2) = p.Z();
            }
            
            // 复制面
            for (int i = 1; i <= tri->NbTriangles(); ++i) {
                int n1, n2, n3;
                tri->Triangle(i).Get(n1, n2, n3);
                // 注意：OCCT使用1-based索引，libigl使用0-based索引
                F(i-1, 0) = n1 - 1;
                F(i-1, 1) = n2 - 1;
                F(i-1, 2) = n3 - 1;
            }
            
            // 计算法向量
            igl::per_face_normals(V, F, Eigen::Vector3d(0, 0, 0), N);
        }
    }
    
    return {V, F, N};
}

// 测试夹具
struct UnifiedModelFixture {
    UnifiedModelFixture() {
        // 创建模型
        model = std::make_shared<UnifiedModel>();
        
        // 加载STEP文件
        std::string step_file_path = MESH_TEST_DATA_DIR "/ANC101.stp";
        
        STEPControl_Reader reader;
        IFSelect_ReturnStatus status = reader.ReadFile(step_file_path.c_str());
        
        if (status != IFSelect_RetDone) {
            BOOST_TEST_MESSAGE("Failed to load STEP file: " << step_file_path);
            return;
        }
        
        // 转换所有根实体
        reader.TransferRoots();
        shape = reader.OneShape();
        
        if (shape.IsNull()) {
            BOOST_TEST_MESSAGE("No valid shape in STEP file");
            return;
        }
        
        // 提取网格数据
        std::tie(vertices, faces, normals) = extractMeshFromShape(shape);
        
        BOOST_TEST_MESSAGE("Loaded STEP file with " 
                          << vertices.rows() << " vertices and " 
                          << faces.rows() << " faces");
    }
    
    ~UnifiedModelFixture() {
        // 清理资源
    }
    
    std::shared_ptr<UnifiedModel> model;
    TopoDS_Shape shape;
    Eigen::MatrixXd vertices;
    Eigen::MatrixXi faces;
    Eigen::MatrixXd normals;
};

// 测试添加CAD形体
BOOST_FIXTURE_TEST_CASE(add_shape_test, UnifiedModelFixture)
{
    // 确保形体有效
    BOOST_REQUIRE(!shape.IsNull());
    
    // 添加形体到模型
    model->addShape("shape1", shape);
    
    // 验证形体已添加
    BOOST_CHECK_EQUAL(model->getAllEntityIds().size(), 1);
    BOOST_CHECK_EQUAL(model->getAllEntityIds()[0], "shape1");
    BOOST_CHECK(model->getGeometryType("shape1") == UnifiedModel::GeometryType::SHAPE);
    
    // 验证可以获取形体
    TopoDS_Shape retrievedShape = model->getShape("shape1");
    BOOST_CHECK(!retrievedShape.IsNull());
}

// 测试添加网格
BOOST_FIXTURE_TEST_CASE(add_mesh_test, UnifiedModelFixture)
{
    // 确保网格数据有效
    BOOST_REQUIRE(vertices.rows() > 0);
    BOOST_REQUIRE(faces.rows() > 0);
    
    // 添加网格到模型
    model->addMesh("mesh1", vertices, faces, normals);
    
    // 验证网格已添加
    BOOST_CHECK_EQUAL(model->getAllEntityIds().size(), 1);
    BOOST_CHECK_EQUAL(model->getAllEntityIds()[0], "mesh1");
    BOOST_CHECK(model->getGeometryType("mesh1") == UnifiedModel::GeometryType::MESH);
    
    // 验证可以获取网格
    const UnifiedModel::MeshData* retrievedMesh = model->getMesh("mesh1");
    BOOST_CHECK(retrievedMesh != nullptr);
    BOOST_CHECK_EQUAL(retrievedMesh->vertices.rows(), vertices.rows());
    BOOST_CHECK_EQUAL(retrievedMesh->faces.rows(), faces.rows());
}

// 测试设置颜色
BOOST_FIXTURE_TEST_CASE(set_color_test, UnifiedModelFixture)
{
    // 添加形体到模型
    model->addShape("shape1", shape);
    
    // 设置颜色
    Quantity_Color color(1.0, 0.0, 0.0, Quantity_TOC_RGB); // 红色
    model->setColor("shape1", color);
    
    // 验证颜色已设置
    Quantity_Color retrievedColor = model->getColor("shape1");
    BOOST_CHECK_CLOSE(retrievedColor.Red(), 1.0, 1e-6);
    BOOST_CHECK_CLOSE(retrievedColor.Green(), 0.0, 1e-6);
    BOOST_CHECK_CLOSE(retrievedColor.Blue(), 0.0, 1e-6);
}

// 测试几何变换
BOOST_FIXTURE_TEST_CASE(transform_test, UnifiedModelFixture)
{
    // 添加网格到模型
    model->addMesh("mesh1", vertices, faces, normals);
    
    // 记录原始位置, 法向量
    const UnifiedModel::MeshData* originalMesh = model->getMesh("mesh1");
    Eigen::Vector3d originalCenter = originalMesh->vertices.colwise().mean();
    Eigen::Vector3d originalNormal = originalMesh->normals.colwise().mean();
    
    // 创建平移变换
    gp_Trsf transformation;
    transformation.SetTranslation(gp_Vec(10.0, 0.0, 0.0)); // X方向平移10个单位
    
    // 应用变换
    model->transform("mesh1", transformation);
    
    // 验证变换已应用
    const UnifiedModel::MeshData* transformedMesh = model->getMesh("mesh1");
    Eigen::Vector3d transformedCenter = transformedMesh->vertices.colwise().mean();
    Eigen::Vector3d transformedNormal = transformedMesh->normals.colwise().mean();

    // 检查中心点是否平移了10个单位
    BOOST_CHECK_CLOSE(transformedCenter.x(), originalCenter.x() + 10.0, 1e-6);
    BOOST_CHECK_CLOSE(transformedCenter.y(), originalCenter.y(), 1e-6);
    BOOST_CHECK_CLOSE(transformedCenter.z(), originalCenter.z(), 1e-6);

    // 法向量应该保持不变
    BOOST_CHECK_CLOSE(transformedNormal.x(), originalNormal.x(), 1e-6);
    BOOST_CHECK_CLOSE(transformedNormal.y(), originalNormal.y(), 1e-6);
    BOOST_CHECK_CLOSE(transformedNormal.z(), originalNormal.z(), 1e-6);
}

// 测试移除几何体
BOOST_FIXTURE_TEST_CASE(remove_geometry_test, UnifiedModelFixture)
{
    // 添加形体和网格到模型
    model->addShape("shape1", shape);
    model->addMesh("mesh1", vertices, faces, normals);
    
    // 验证已添加两个几何体
    BOOST_CHECK_EQUAL(model->getAllEntityIds().size(), 2);
    
    // 移除形体
    model->removeGeometry("shape1");
    
    // 验证形体已移除
    BOOST_CHECK_EQUAL(model->getAllEntityIds().size(), 1);
    BOOST_CHECK_EQUAL(model->getAllEntityIds()[0], "mesh1");
    
    // 移除网格
    model->removeGeometry("mesh1");
    
    // 验证网格已移除
    BOOST_CHECK_EQUAL(model->getAllEntityIds().size(), 0);
}

// 测试获取几何体类型
BOOST_FIXTURE_TEST_CASE(get_geometry_type_test, UnifiedModelFixture)
{
    // 添加形体和网格到模型
    model->addShape("shape1", shape);
    model->addMesh("mesh1", vertices, faces, normals);
    
    // 验证几何体类型
    BOOST_CHECK(model->getGeometryType("shape1") == UnifiedModel::GeometryType::SHAPE);
    BOOST_CHECK(model->getGeometryType("mesh1") == UnifiedModel::GeometryType::MESH);
    
    // 测试获取不存在的几何体类型
    BOOST_CHECK_THROW(model->getGeometryType("nonexistent"), std::runtime_error);
}

// 测试按类型获取几何体ID
BOOST_FIXTURE_TEST_CASE(get_geometry_ids_by_type_test, UnifiedModelFixture)
{
    // 添加形体和网格到模型
    model->addShape("shape1", shape);
    model->addShape("shape2", shape);
    model->addMesh("mesh1", vertices, faces, normals);
    model->addMesh("mesh2", vertices, faces, normals);
    
    // 获取所有形体ID
    std::vector<std::string> shapeIds = model->getGeometryIdsByType(UnifiedModel::GeometryType::SHAPE);
    BOOST_CHECK_EQUAL(shapeIds.size(), 2);
    BOOST_CHECK(std::find(shapeIds.begin(), shapeIds.end(), "shape1") != shapeIds.end());
    BOOST_CHECK(std::find(shapeIds.begin(), shapeIds.end(), "shape2") != shapeIds.end());
    
    // 获取所有网格ID
    std::vector<std::string> meshIds = model->getGeometryIdsByType(UnifiedModel::GeometryType::MESH);
    BOOST_CHECK_EQUAL(meshIds.size(), 2);
    BOOST_CHECK(std::find(meshIds.begin(), meshIds.end(), "mesh1") != meshIds.end());
    BOOST_CHECK(std::find(meshIds.begin(), meshIds.end(), "mesh2") != meshIds.end());
} 