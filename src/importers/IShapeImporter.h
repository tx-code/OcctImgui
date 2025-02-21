#pragma once

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <V3d_View.hxx>
#include <string>
#include <vector>


class IShapeImporter
{
public:
    virtual ~IShapeImporter() = default;

    virtual bool Import(const char* filePath,
                        const Handle(AIS_InteractiveContext) & context,
                        std::vector<Handle(AIS_Shape)>& shapes,
                        const Handle(V3d_View) & view) = 0;

    virtual std::string GetFileExtensions() const = 0;
    virtual std::string GetImporterName() const = 0;
};
