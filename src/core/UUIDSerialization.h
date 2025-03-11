#pragma once
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>

// UUID Serialization
namespace nlohmann
{
template<>
struct adl_serializer<boost::uuids::uuid>
{
    static void to_json(json& j, const boost::uuids::uuid& uuid)
    {
        j = boost::uuids::to_string(uuid);
    }

    static void from_json(const json& j, boost::uuids::uuid& uuid)
    {
        boost::uuids::string_generator gen;
        uuid = gen(j.get<std::string>());
    }
};
}  // namespace nlohmann