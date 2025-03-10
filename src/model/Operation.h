#pragma once

#include "IOperation.h"
#include "Model.h"
#include <map>
#include <memory>
#include <string>
#include <variant>

namespace Model
{

// Forward declarations
class ToolPath;

/**
 * @class Operation
 * @brief Implementation of the IOperation interface
 *
 * This class represents an operation that can be performed on geometry models,
 * such as machining operations, Boolean operations, etc.
 */
class Operation: public Model, public IOperation
{
public:
    using Variant = std::variant<bool, int, double, std::string>;

    /**
     * @brief Constructor
     * @param name Name of the operation
     * @param type Type of the operation
     */
    Operation(const std::string& name, const std::string& type)
        : Model(name, ModelTypeEnum::Operation)
        , m_type(type)
    {}

    /**
     * @brief Destructor
     */
    ~Operation() override = default;

    // IModel implementation is handled by the Model base class

    // IOperation implementation
    std::string getType() const override
    {
        return m_type;
    }
    bool execute() override;
    std::shared_ptr<GeometryModel> simulate() override;
    const OperationParams& getParameters() const override;
    void setParameters(const OperationParams& params) override;

    /**
     * @brief Generate the tool path for this operation
     * @return True if the tool path was generated successfully, false otherwise
     */
    bool generateToolPath();

    /**
     * @brief Get the generated tool path
     * @return The tool path or nullptr if not generated
     */
    std::shared_ptr<ToolPath> getToolPath() const;

    /**
     * @brief Set a custom property
     * @param key Property key
     * @param value Property value
     */
    void setProperty(const std::string& key, const Variant& value);

    /**
     * @brief Get a custom property
     * @param key Property key
     * @return Property value or empty variant if not found
     */
    Variant getProperty(const std::string& key) const;

    /**
     * @brief Check if a custom property exists
     * @param key Property key
     * @return True if the property exists, false otherwise
     */
    bool hasProperty(const std::string& key) const;

protected:
    /**
     * @brief Execute the operation (implementation)
     *
     * This method should be overridden by derived classes to implement
     * the specific operation.
     */
    virtual bool executeImpl() = 0;

    /**
     * @brief Simulate the operation (implementation)
     *
     * This method should be overridden by derived classes to implement
     * the simulation of the specific operation.
     */
    virtual std::shared_ptr<GeometryModel> simulateImpl() = 0;

private:
    Core::Property<std::string> m_type;
    std::unique_ptr<OperationParams> m_params;
    std::shared_ptr<ToolPath> m_toolPath;
    std::map<std::string, Variant> m_properties;
};

}  // namespace Model