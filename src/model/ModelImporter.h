/**
 * @file ModelImporter.h
 * @brief Defines the ModelImporter class for importing various 3D model formats.
 * 
 * The ModelImporter provides a unified interface for importing different 3D model formats
 * including STEP (CAD) files using OpenCASCADE and mesh files (STL, OBJ) using libigl.
 */
#pragma once

#include "UnifiedModel.h"
#include <string>
#include <memory>
#include <functional>
#include <map>

/**
 * @class ModelImporter
 * @brief A class that provides a unified interface for importing various 3D model formats.
 * 
 * This class handles the import of different 3D model formats based on file extensions.
 * It uses OpenCASCADE for STEP files and libigl for mesh files (STL, OBJ).
 */
class ModelImporter {
public:
    /**
     * @brief Default constructor
     */
    ModelImporter();
    
    /**
     * @brief Destructor
     */
    ~ModelImporter() = default;
    
    /**
     * @brief Imports a model from a file and adds it to the given UnifiedModel
     * 
     * @param filePath The path to the model file
     * @param model The UnifiedModel to add the imported model to
     * @param modelId The ID to assign to the imported model (if empty, the filename will be used)
     * @return bool True if import was successful, false otherwise
     */
    bool importModel(const std::string& filePath, UnifiedModel& model, const std::string& modelId = "");
    
    /**
     * @brief Gets the supported file extensions
     * 
     * @return std::vector<std::string> List of supported file extensions (e.g., ".step", ".stl", ".obj")
     */
    std::vector<std::string> getSupportedExtensions() const;
    
private:
    /**
     * @brief Imports a STEP file using OpenCASCADE
     * 
     * @param filePath The path to the STEP file
     * @param model The UnifiedModel to add the imported model to
     * @param modelId The ID to assign to the imported model
     * @return bool True if import was successful, false otherwise
     */
    bool importStepFile(const std::string& filePath, UnifiedModel& model, const std::string& modelId);
    
    /**
     * @brief Imports an STL file using libigl
     * 
     * @param filePath The path to the STL file
     * @param model The UnifiedModel to add the imported model to
     * @param modelId The ID to assign to the imported model
     * @return bool True if import was successful, false otherwise
     */
    bool importStlFile(const std::string& filePath, UnifiedModel& model, const std::string& modelId);
    
    /**
     * @brief Imports an OBJ file using libigl
     * 
     * @param filePath The path to the OBJ file
     * @param model The UnifiedModel to add the imported model to
     * @param modelId The ID to assign to the imported model
     * @return bool True if import was successful, false otherwise
     */
    bool importObjFile(const std::string& filePath, UnifiedModel& model, const std::string& modelId);
    
    /**
     * @brief Gets the file extension from a file path
     * 
     * @param filePath The file path
     * @return std::string The file extension (lowercase, including the dot)
     */
    std::string getFileExtension(const std::string& filePath) const;
    
    /**
     * @brief Gets the file name without extension from a file path
     * 
     * @param filePath The file path
     * @return std::string The file name without extension
     */
    std::string getFileName(const std::string& filePath) const;
    
    // 定义成员函数指针类型
    using ImportFunction = std::function<bool(const std::string&, UnifiedModel&, const std::string&)>;
    
    // Map of file extensions to import functions
    std::map<std::string, ImportFunction> myImportFunctions;
}; 