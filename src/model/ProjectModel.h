#pragma once

#include "GeometryModel.h"
#include "Model.h"
#include "Operation.h"
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>


namespace Model
{

/**
 * @class ProjectModel
 * @brief Represents a complete project containing models and operations
 *
 * This class manages the entire project, including geometry models, operations,
 * and the relationships between them. It also handles project serialization and
 * G-code generation.
 */
class ProjectModel: public Model
{
public:
    /**
     * @brief Constructor
     * @param name Name of the project
     */
    explicit ProjectModel(const std::string& name)
        : Model(name, ModelTypeEnum::Project)
    {}

    /**
     * @brief Destructor
     */
    ~ProjectModel() override = default;

    // IModel implementation is handled by the Model base class

    /**
     * @brief Add a geometry model to the project
     * @param model The model to add
     */
    void addGeometryModel(const std::shared_ptr<GeometryModel>& model);

    /**
     * @brief Remove a geometry model from the project
     * @param name Name of the model to remove
     */
    void removeGeometryModel(const std::string& name);

    /**
     * @brief Get a geometry model by name
     * @param name Name of the model to get
     * @return The model or nullptr if not found
     */
    std::shared_ptr<GeometryModel> getGeometryModel(const std::string& name) const;

    /**
     * @brief Set the workpiece model
     * @param model The workpiece model
     */
    void setWorkpiece(const std::shared_ptr<GeometryModel>& model);

    /**
     * @brief Get the workpiece model
     * @return The workpiece model or nullptr if not set
     */
    std::shared_ptr<GeometryModel> getWorkpiece() const;

    /**
     * @brief Set the stock model
     * @param model The stock model
     */
    void setStock(const std::shared_ptr<GeometryModel>& model);

    /**
     * @brief Get the stock model
     * @return The stock model or nullptr if not set
     */
    std::shared_ptr<GeometryModel> getStock() const;

    /**
     * @brief Add an operation to the project
     * @param op The operation to add
     */
    void addOperation(const std::shared_ptr<Operation>& op);

    /**
     * @brief Remove an operation from the project
     * @param index Index of the operation to remove
     */
    void removeOperation(int index);

    /**
     * @brief Get an operation by index
     * @param index Index of the operation to get
     * @return The operation or nullptr if index is out of range
     */
    std::shared_ptr<Operation> getOperation(int index) const;

    /**
     * @brief Move an operation to a new position
     * @param fromIndex Current index of the operation
     * @param toIndex New index for the operation
     */
    void moveOperation(int fromIndex, int toIndex);

    /**
     * @brief Load a project from a file
     * @param filename Path to the project file
     * @return True if loading succeeded, false otherwise
     */
    bool loadProject(const std::string& filename);

    /**
     * @brief Save the project to a file
     * @param filename Path to save the project
     * @return True if saving succeeded, false otherwise
     */
    bool saveProject(const std::string& filename);

    /**
     * @brief Check if the project has been modified
     * @return True if the project has been modified, false otherwise
     */
    bool isModified() const
    {
        return Model::isModified();
    }

    /**
     * @brief Generate tool paths for all operations
     * @return True if all tool paths were generated successfully, false otherwise
     */
    bool generateAllToolPaths();

    /**
     * @brief Generate G-code for all operations
     * @return String containing the complete G-code
     */
    std::string generateGCode() const;

private:
    std::map<std::string, std::shared_ptr<GeometryModel>> m_geometryModels;
    std::shared_ptr<GeometryModel> m_workpiece;
    std::shared_ptr<GeometryModel> m_stock;
    std::vector<std::shared_ptr<Operation>> m_operations;
};

}  // namespace Model