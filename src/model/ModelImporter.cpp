#include "ModelImporter.h"
#include "utils/Logger.h"

// OpenCASCADE includes for STEP import
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <STEPControl_Reader.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>


// libigl includes for mesh import
#include <igl/per_face_normals.h>
#include <igl/readOBJ.h>
#include <igl/read_triangle_mesh.h>


#include <algorithm>
#include <cctype>
#include <filesystem>
#include <functional>


// 使用宏声明 ModelImporter 类的 logger
DECLARE_LOGGER(ModelImporter)

ModelImporter::ModelImporter()
{
    // 注册导入函数
    myImportFunctions[".step"] = std::bind(&ModelImporter::importStepFile,
                                           this,
                                           std::placeholders::_1,
                                           std::placeholders::_2,
                                           std::placeholders::_3);
    myImportFunctions[".stp"] = myImportFunctions[".step"];
    myImportFunctions[".stl"] = std::bind(&ModelImporter::importStlFile,
                                          this,
                                          std::placeholders::_1,
                                          std::placeholders::_2,
                                          std::placeholders::_3);
    myImportFunctions[".obj"] = std::bind(&ModelImporter::importObjFile,
                                          this,
                                          std::placeholders::_1,
                                          std::placeholders::_2,
                                          std::placeholders::_3);

    getModelImporterLogger()->info("ModelImporter initialized with {} supported formats",
                                   myImportFunctions.size());
}

bool ModelImporter::importModel(const std::string& filePath,
                                GeometryModel& model,
                                const std::string& modelId)
{
    // 获取文件扩展名（转为小写）
    std::string extension = getFileExtension(filePath);

    // 如果未指定模型ID，则使用文件名作为ID
    std::string effectiveModelId = modelId.empty() ? getFileName(filePath) : modelId;

    getModelImporterLogger()->info("Importing model from '{}' with ID '{}'",
                                   filePath,
                                   effectiveModelId);

    // 查找对应的导入函数
    auto it = myImportFunctions.find(extension);
    if (it == myImportFunctions.end()) {
        getModelImporterLogger()->error("Unsupported file format: {}", extension);
        return false;
    }

    // 调用导入函数
    return it->second(filePath, model, effectiveModelId);
}

std::vector<std::string> ModelImporter::getSupportedExtensions() const
{
    std::vector<std::string> extensions;
    extensions.reserve(myImportFunctions.size());

    for (const auto& pair : myImportFunctions) {
        extensions.push_back(pair.first);
    }

    return extensions;
}

bool ModelImporter::importStepFile(const std::string& filePath,
                                   GeometryModel& model,
                                   const std::string& modelId)
{
    getModelImporterLogger()->info("Importing STEP file: {}", filePath);

    // 使用OpenCASCADE的STEP读取器
    STEPControl_Reader reader;
    IFSelect_ReturnStatus status = reader.ReadFile(filePath.c_str());

    if (status != IFSelect_RetDone) {
        getModelImporterLogger()->error("Failed to read STEP file: {}", filePath);
        return false;
    }

    // 转换所有根实体
    reader.TransferRoots();
    TopoDS_Shape shape = reader.OneShape();

    if (shape.IsNull()) {
        getModelImporterLogger()->error("No valid shape in STEP file: {}", filePath);
        return false;
    }

    // 添加形体到模型
    model.addShape(modelId, shape);
    getModelImporterLogger()->info("Successfully imported STEP model with ID: {}", modelId);

    return true;
}

bool ModelImporter::importStlFile(const std::string& filePath,
                                  GeometryModel& model,
                                  const std::string& modelId)
{
    getModelImporterLogger()->info("Importing STL file: {}", filePath);

    // 使用libigl读取STL文件
    Eigen::MatrixXd vertices;
    Eigen::MatrixXi faces;

    if (!igl::read_triangle_mesh(filePath, vertices, faces)) {
        getModelImporterLogger()->error("Failed to read STL file: {}", filePath);
        return false;
    }

    // 计算法向量
    Eigen::MatrixXd normals;
    igl::per_face_normals(vertices, faces, Eigen::Vector3d(0, 0, 0), normals);

    // 添加网格到模型
    model.addMesh(modelId, vertices, faces, normals);
    getModelImporterLogger()->info(
        "Successfully imported STL model with ID: {} ({} vertices, {} faces)",
        modelId,
        vertices.rows(),
        faces.rows());

    return true;
}

bool ModelImporter::importObjFile(const std::string& filePath,
                                  GeometryModel& model,
                                  const std::string& modelId)
{
    getModelImporterLogger()->info("Importing OBJ file: {}", filePath);

    // 使用libigl读取OBJ文件
    Eigen::MatrixXd vertices;
    Eigen::MatrixXi faces;

    if (!igl::readOBJ(filePath, vertices, faces)) {
        getModelImporterLogger()->error("Failed to read OBJ file: {}", filePath);
        return false;
    }

    // 计算法向量
    Eigen::MatrixXd normals;
    igl::per_face_normals(vertices, faces, Eigen::Vector3d(0, 0, 0), normals);

    // 添加网格到模型
    model.addMesh(modelId, vertices, faces, normals);
    getModelImporterLogger()->info(
        "Successfully imported OBJ model with ID: {} ({} vertices, {} faces)",
        modelId,
        vertices.rows(),
        faces.rows());

    return true;
}

std::string ModelImporter::getFileExtension(const std::string& filePath) const
{
    std::filesystem::path path(filePath);
    std::string extension = path.extension().string();

    // 转为小写
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) {
        return std::tolower(c);
    });

    return extension;
}

std::string ModelImporter::getFileName(const std::string& filePath) const
{
    std::filesystem::path path(filePath);
    return path.stem().string();
}