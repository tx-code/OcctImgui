#define BOOST_TEST_MODULE ModelImporter Tests
#include <boost/test/unit_test.hpp>

#include "model/ModelImporter.h"
#include "model/UnifiedModel.h"

#include <string>
#include <memory>
#include <filesystem>

BOOST_AUTO_TEST_CASE(supported_extensions_test)
{
    ModelImporter importer;
    auto extensions = importer.getSupportedExtensions();
    
    // 验证支持的扩展名
    BOOST_CHECK(std::find(extensions.begin(), extensions.end(), ".step") != extensions.end());
    BOOST_CHECK(std::find(extensions.begin(), extensions.end(), ".stp") != extensions.end());
    BOOST_CHECK(std::find(extensions.begin(), extensions.end(), ".stl") != extensions.end());
    BOOST_CHECK(std::find(extensions.begin(), extensions.end(), ".obj") != extensions.end());
    
    // 验证扩展名数量
    BOOST_CHECK_EQUAL(extensions.size(), 4);
}

BOOST_AUTO_TEST_CASE(import_step_file_test)
{
    // 创建模型和导入器
    auto model = std::make_shared<UnifiedModel>();
    ModelImporter importer;
    
    // 导入STEP文件
    std::string step_file_path = MESH_TEST_DATA_DIR "/ANC101.stp";
    
    // 检查文件是否存在
    if (!std::filesystem::exists(step_file_path)) {
        BOOST_TEST_MESSAGE("STEP test file not found: " << step_file_path);
        BOOST_CHECK(false);
        return;
    }
    
    // 导入模型
    bool result = importer.importModel(step_file_path, *model);
    
    // 验证导入结果
    BOOST_CHECK(result);
    BOOST_CHECK_EQUAL(model->getAllEntityIds().size(), 1);
    
    // 验证导入的模型类型
    std::string modelId = model->getAllEntityIds()[0];
    BOOST_CHECK(model->getGeometryType(modelId) == UnifiedModel::GeometryType::SHAPE);
    
    // 验证导入的模型ID是文件名
    BOOST_CHECK_EQUAL(modelId, "ANC101");
}

BOOST_AUTO_TEST_CASE(import_stl_file_test)
{
    // 创建模型和导入器
    auto model = std::make_shared<UnifiedModel>();
    ModelImporter importer;
    
    // 导入STL文件
    std::string stl_file_path = MESH_TEST_DATA_DIR "/cube.stl";
    
    // 检查文件是否存在
    if (!std::filesystem::exists(stl_file_path)) {
        BOOST_TEST_MESSAGE("STL test file not found: " << stl_file_path);
        BOOST_CHECK(false);
        return;
    }
    
    // 导入模型
    bool result = importer.importModel(stl_file_path, *model);
    
    // 验证导入结果
    BOOST_CHECK(result);
    BOOST_CHECK_EQUAL(model->getAllEntityIds().size(), 1);
    
    // 验证导入的模型类型
    std::string modelId = model->getAllEntityIds()[0];
    BOOST_CHECK(model->getGeometryType(modelId) == UnifiedModel::GeometryType::MESH);
    
    // 验证导入的模型ID是文件名
    BOOST_CHECK_EQUAL(modelId, "cube");
    
    // 验证导入的网格数据
    const UnifiedModel::MeshData* mesh = model->getMesh(modelId);
    BOOST_CHECK(mesh != nullptr);
    BOOST_CHECK(mesh->vertices.rows() > 0);
    BOOST_CHECK(mesh->faces.rows() > 0);
    BOOST_CHECK(mesh->normals.rows() > 0);
}

BOOST_AUTO_TEST_CASE(import_obj_file_test)
{
    // 创建模型和导入器
    auto model = std::make_shared<UnifiedModel>();
    ModelImporter importer;
    
    // 导入OBJ文件
    std::string obj_file_path = MESH_TEST_DATA_DIR "/bunny.obj";
    
    // 检查文件是否存在
    if (!std::filesystem::exists(obj_file_path)) {
        BOOST_TEST_MESSAGE("OBJ test file not found: " << obj_file_path);
        BOOST_CHECK(false);
        return;
    }
    
    // 导入模型
    bool result = importer.importModel(obj_file_path, *model);
    
    // 验证导入结果
    BOOST_CHECK(result);
    BOOST_CHECK_EQUAL(model->getAllEntityIds().size(), 1);
    
    // 验证导入的模型类型
    std::string modelId = model->getAllEntityIds()[0];
    BOOST_CHECK(model->getGeometryType(modelId) == UnifiedModel::GeometryType::MESH);
    
    // 验证导入的模型ID是文件名
    BOOST_CHECK_EQUAL(modelId, "bunny");
    
    // 验证导入的网格数据
    const UnifiedModel::MeshData* mesh = model->getMesh(modelId);
    BOOST_CHECK(mesh != nullptr);
    BOOST_CHECK(mesh->vertices.rows() > 0);
    BOOST_CHECK(mesh->faces.rows() > 0);
    BOOST_CHECK(mesh->normals.rows() > 0);
}

BOOST_AUTO_TEST_CASE(import_with_custom_id_test)
{
    // 创建模型和导入器
    auto model = std::make_shared<UnifiedModel>();
    ModelImporter importer;
    
    // 导入STEP文件，指定自定义ID
    std::string step_file_path = MESH_TEST_DATA_DIR "/ANC101.stp";
    
    // 检查文件是否存在
    if (!std::filesystem::exists(step_file_path)) {
        BOOST_TEST_MESSAGE("STEP test file not found: " << step_file_path);
        BOOST_CHECK(false);
        return;
    }
    
    // 导入模型，指定自定义ID
    bool result = importer.importModel(step_file_path, *model, "custom_id");
    
    // 验证导入结果
    BOOST_CHECK(result);
    BOOST_CHECK_EQUAL(model->getAllEntityIds().size(), 1);
    
    // 验证导入的模型ID是自定义ID
    BOOST_CHECK_EQUAL(model->getAllEntityIds()[0], "custom_id");
}

BOOST_AUTO_TEST_CASE(import_unsupported_format_test)
{
    // 创建模型和导入器
    auto model = std::make_shared<UnifiedModel>();
    ModelImporter importer;
    
    // 尝试导入不支持的格式
    std::string unsupported_file_path = MESH_TEST_DATA_DIR "/unsupported.xyz";
    
    // 导入模型
    bool result = importer.importModel(unsupported_file_path, *model);
    
    // 验证导入失败
    BOOST_CHECK(!result);
    BOOST_CHECK_EQUAL(model->getAllEntityIds().size(), 0);
} 