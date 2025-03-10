#pragma once

#include "IModel.h"
#include <memory>
#include <string>

namespace Model 
{

// Forward declaration
class GeometryModel;

/**
 * @struct OperationParams
 * @brief Base struct for operation parameters
 *
 * This is a POD (Plain Old Data) structure for operation parameters.
 */
struct OperationParams
{
};

/**
 * @enum OperationTypeEnum
 * @brief Enum for operation types
 *
 * This enum defines the different types of operations that can be performed
 * on geometry models.
 */
enum class OperationTypeEnum
{
    Drilling,
    Unknown
};

/**
 * @class IOperation
 * @brief Interface for operation models
 *
 * This interface defines the API for operations that can be performed
 * on geometry models, such as machining operations, Boolean operations, etc.
 */
class IOperation: public IModel
{
public:

    /**
     * @brief Execute the operation
     * @return True if the operation executed successfully, false otherwise
     */
    virtual bool execute() = 0;

    /**
     * @brief Simulate the operation without modifying the actual model
     * @return A geometry model representing the result of the simulation
     */
    virtual std::shared_ptr<GeometryModel> simulate() = 0;

    /**
     * @brief Get the operation parameters
     * @return Reference to the operation parameters
     */
    virtual const OperationParams& getParameters() const = 0;

    /**
     * @brief Set the operation parameters
     * @param params The new parameters
     */
    virtual void setParameters(const OperationParams& params) = 0;

    /**
     * @brief Get the operation type
     * @return String identifier for the operation type
     */
    virtual OperationTypeEnum getType() const = 0;
};

}  // namespace Model