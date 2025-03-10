#pragma once

#include <boost/uuid/uuid.hpp>
#include <memory>
#include <string>


namespace Model
{

/**
 * @enum ModelTypeEnum
 * @brief Enumeration of model types in the system
 */
enum class ModelTypeEnum
{
    Geometry,
    Operation,
    Project,
    ToolPath,
    Unknown
};

/**
 * @class IModel
 * @brief Base interface for all model objects in the system
 *
 * This interface defines the common functionality for all model types.
 * It provides methods for identification and cloning.
 */
class IModel
{
public:
    using UUID = boost::uuids::uuid;

    virtual ~IModel() = default;

    /**
     * @brief Get the model name
     * @return The model name
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Set the model name
     * @param name The new name for the model
     */
    virtual void setName(const std::string& name) = 0;

    /**
     * @brief Get the unique identifier for this model
     * @return The model's UUID
     */
    virtual UUID getId() const = 0;

    /**
     * @brief Get the model type
     * @return The model type enumeration value
     */
    virtual ModelTypeEnum getType() const = 0;

    /**
     * @brief Create a deep copy of this model
     * @return A new instance that is a copy of this model
     */
    virtual std::shared_ptr<IModel> clone() const = 0;
};

}  // namespace Model