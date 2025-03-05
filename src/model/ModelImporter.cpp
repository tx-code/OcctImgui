#include "ModelImporter.h"
#include "../utils/Logger.h"

// OpenCASCADE includes for STEP import
#include <STEPControl_Reader.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS_Face.hxx>
#include <TopLoc_Location.hxx>
#include <Poly_Triangulation.hxx>

// libigl includes for mesh import
#include <igl/read_triangle_mesh.h>
#include <igl/readOBJ.h>
#include <igl/per_face_normals.h>

#include <algorithm>
#include <filesystem>
#include <cctype>

// 创建导入器日志记录器
static std::shared_ptr<Utils::Logger>& getImporterLogger() {
    static std::shared_ptr<Utils::Logger> logger = Utils::Logger::getLogger("model.importer");
    return logger;
}

ModelImporter::ModelImporter() {
    // 注册导入函数
    myImportFunctions[".step"] = &ModelImporter::importStepFile;
    myImportFunctions[".stp"] = &ModelImporter::importStepFile;
    myImportFunctions[".stl"] = &ModelImporter::importStlFile;
    myImportFunctions[".obj"] = &ModelImporter::importObjFile;
    
    getImporterLogger()->info("ModelImporter initialized with {} supported formats", myImportFunctions.size());
}

bool ModelImporter::importModel(const std::string& filePath, UnifiedModel& model, const std::string& modelId) {
    // 获取文件扩展名（转为小写）
    std::string extension = getFileExtension(filePath);
    
    // 如果未指定模型ID，则使用文件名作为ID
    std::string effectiveModelId = modelId.empty() ? getFileName(filePath) : modelId;
    
    getImporterLogger()->info("Importing model from '{}' with ID '{}'", filePath, effectiveModelId);
    
    // 查找对应的导入函数
    auto it = myImportFunctions.find(extension);
    if (it == myImportFunctions.end()) {
        getImporterLogger()->error("Unsupported file format: {}", extension);
        return false;
    }
    
    // 调用导入函数
    return (this->*it->second)(filePath, model, effectiveModelId);
}

std::vector<std::string> ModelImporter::getSupportedExtensions() const {
    std::vector<std::string> extensions;
    extensions.reserve(myImportFunctions.size());
    
    for (const auto& pair : myImportFunctions) {
        extensions.push_back(pair.first);
    }
    
    return extensions;
}

bool ModelImporter::importStepFile(const std::string& filePath, UnifiedModel& model, const std::string& modelId) {
    getImporterLogger()->info("Importing STEP file: {}", filePath);
    
    // 使用OpenCASCADE的STEP读取器
    STEPControl_Reader reader;
    IFSelect_ReturnStatus status = reader.ReadFile(filePath.c_str());
    
    if (status != IFSelect_RetDone) {
        getImporterLogger()->error("Failed to read STEP file: {}", filePath);
        return false;
    }
    
    // 转换所有根实体
    reader.TransferRoots();
    TopoDS_Shape shape = reader.OneShape();
    
    if (shape.IsNull()) {
        getImporterLogger()->error("No valid shape in STEP file: {}", filePath);
        return false;
    }
    
    // 添加形体到模型
    model.addShape(modelId, shape);
    getImporterLogger()->info("Successfully imported STEP model with ID: {}", modelId);
    
    return true;
}

bool ModelImporter::importStlFile(const std::string& filePath, UnifiedModel& model, const std::string& modelId) {
    getImporterLogger()->info("Importing STL file: {}", filePath);
    
    // 使用libigl读取STL文件
    Eigen::MatrixXd vertices;
    Eigen::MatrixXi faces;
    
    if (!igl::read_triangle_mesh(filePath, vertices, faces)) {
        getImporterLogger()->error("Failed to read STL file: {}", filePath);
        return false;
    }
    
    // 计算法向量
    Eigen::MatrixXd normals;
    igl::per_face_normals(vertices, faces, Eigen::Vector3d(0, 0, 0), normals);
    
    // 添加网格到模型
    model.addMesh(modelId, vertices, faces, normals);
    getImporterLogger()->info("Successfully imported STL model with ID: {} ({} vertices, {} faces)", 
                             modelId, vertices.rows(), faces.rows());
    
    return true;
}

bool ModelImporter::importObjFile(const std::string& filePath, UnifiedModel& model, const std::string& modelId) {
    getImporterLogger()->info("Importing OBJ file: {}", filePath);
    
    // 使用libigl读取OBJ文件
    Eigen::MatrixXd vertices;
    Eigen::MatrixXi faces;
    
    if (!igl::readOBJ(filePath, vertices, faces)) {
        getImporterLogger()->error("Failed to read OBJ file: {}", filePath);
        return false;
    }
    
    // 计算法向量
    Eigen::MatrixXd normals;
    igl::per_face_normals(vertices, faces, Eigen::Vector3d(0, 0, 0), normals);
    
    // 添加网格到模型
    model.addMesh(modelId, vertices, faces, normals);
    getImporterLogger()->info("Successfully imported OBJ model with ID: {} ({} vertices, {} faces)", 
                             modelId, vertices.rows(), faces.rows());
    
    return true;
}

std::string ModelImporter::getFileExtension(const std::string& filePath) const {
    std::filesystem::path path(filePath);
    std::string extension = path.extension().string();
    
    // 转为小写
    std::transform(extension.begin(), extension.end(), extension.begin(),
                  [](unsigned char c) { return std::tolower(c); });
    
    return extension;
}

std::string ModelImporter::getFileName(const std::string& filePath) const {
    std::filesystem::path path(filePath);
    return path.stem().string();
} 