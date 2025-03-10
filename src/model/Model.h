#pragma once

#include "IModel.h"
#include "../core/SerializableObject.h"
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <memory>

namespace Model
{

/**
 * @class Model
 * @brief Base implementation of the IModel interface
 *
 * This class provides common functionality for all model types, including:
 * - UUID generation and management
 * - Property management using SerializableObject
 * - Basic name handling
 */
class Model: public virtual IModel, public Core::SerializableObject
{
public:
    /**
     * @brief Constructor
     * @param name Name of the model
     * @param type Type of the model
     */
    Model(const std::string& name, ModelTypeEnum type)
        : m_type(type)
    {
        m_id = boost::uuids::random_generator()();
        m_name.set(name);
        
        // Register properties for serialization
        registerProperty("name", m_name);
        registerProperty("id", m_id);
        registerProperty("type", m_type);
    }

    /**
     * @brief Virtual destructor
     */
    virtual ~Model() = default;

    // IModel implementation
    std::string getName() const override
    {
        return m_name.get();
    }
    
    void setName(const std::string& name) override
    {
        m_name.set(name);
    }
    
    UUID getId() const override
    {
        return m_id;
    }
    
    ModelTypeEnum getType() const override
    {
        return m_type;
    }

    /**
     * @brief Get signal for name changes
     * @return Reference to the name changed signal
     */
    Core::Signal<const std::string&, const std::string&>& nameChanged()
    {
        return m_name.valueChanged;
    }

protected:
    Core::Property<UUID> m_id;
    Core::Property<std::string> m_name;
    Core::Property<ModelTypeEnum> m_type;
};

}  // namespace Model