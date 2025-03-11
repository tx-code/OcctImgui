#include "TopoShapeSerialization.h"
#include "utils/base64.h"

#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <sstream>


using namespace nlohmann;

void adl_serializer<TopoDS_Shape>::to_json(json& j, const TopoDS_Shape& shape)
{
    std::ostringstream oss;
    BRepTools::Write(shape, oss);
    auto str = oss.str();

    j["TopoDS_Shape"] = json {{"format", "BREP"}, {"data", base64_encode(str)}};
}

void adl_serializer<TopoDS_Shape>::from_json(const json& j, TopoDS_Shape& shape)
{
    if (j.contains("TopoDS_Shape")) {
        auto& topoDSShape = j["TopoDS_Shape"];
        if (topoDSShape.contains("format") && topoDSShape["format"] == "BREP") {
            std::istringstream iss(base64_decode(topoDSShape["data"].get<std::string>()));
            BRep_Builder builder;
            BRepTools::Read(shape, iss, builder);
        }
    }
}
