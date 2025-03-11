#pragma once

#include <TopoDS_Shape.hxx>
#include <nlohmann/json.hpp>

// JSON serialization for TopoDS_Shape
namespace nlohmann
{
template<>
struct adl_serializer<TopoDS_Shape>
{
    static void to_json(nlohmann::json& j, const TopoDS_Shape& shape);
    static void from_json(const nlohmann::json& j, TopoDS_Shape& shape);
};
}  // namespace nlohmann