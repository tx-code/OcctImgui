#pragma once

#include "PropertyRegistry.h"
#include <fstream>

namespace Core
{

class SerializableObject
{
public:
    SerializableObject()
        : myPropertyRegistry(std::make_shared<PropertyRegistry>())
    {}

    template<typename T>
    void registerProperty(const std::string& name, Property<T>& property)
    {
        myPropertyRegistry->registerProperty(name, property);
    }

    nlohmann::json serialize() const
    {
        return myPropertyRegistry->serialize();
    }

    bool deserialize(const nlohmann::json& json)
    {
        return myPropertyRegistry->deserialize(json);
    }

    bool saveToFile(const std::string& filename) const
    {
        try {
            std::ofstream file(filename);
            if (!file.is_open())
                return false;

            file << serialize().dump(2);
            return true;
        }
        catch (...) {
            return false;
        }
    }

    bool loadFromFile(const std::string& filename)
    {
        try {
            std::ifstream file(filename);
            if (!file.is_open())
                return false;

            nlohmann::json json;
            file >> json;
            return deserialize(json);
        }
        catch (...) {
            return false;
        }
    }

protected:
    std::shared_ptr<PropertyRegistry> myPropertyRegistry;
};

}  // namespace Core
