#pragma once

#include "Property.h"

#include <functional>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>


namespace Core
{

class PropertyRegistry
{
    using Json = nlohmann::json;

public:
    using SerializeFn = std::function<Json()>;
    using DeserializeFn = std::function<bool(const Json&)>;

    void
    registerProperty(const std::string& name, SerializeFn serializeFn, DeserializeFn deserializeFn)
    {
        myProperties[name] = {serializeFn, deserializeFn};
    }

    template<typename T>
    void registerProperty(const std::string& name, Property<T>& property)
    {
        registerProperty(
            name,
            [&property]() {
                return property.toJson();
            },
            [&property](const Json& json) {
                return property.fromJson(json);
            });
    }

    Json serialize() const
    {
        Json result;
        for (const auto& [name, handlers] : myProperties) {
            result[name] = handlers.serializeFn();
        }
        return result;
    }

    bool deserialize(const Json& json)
    {
        if (!json.is_object())
            return false;

        bool success = true;
        for (const auto& [name, handlers] : myProperties) {
            if (json.contains(name)) {
                success &= handlers.deserializeFn(json[name]);
            }
        }
        return success;
    }

private:
    struct PropertyHandlers
    {
        SerializeFn serializeFn;
        DeserializeFn deserializeFn;
    };

    std::unordered_map<std::string, PropertyHandlers> myProperties;
};
}  // namespace Core
