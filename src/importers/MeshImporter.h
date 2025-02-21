#pragma once

#include "IShapeImporter.h"
#include <DE_Wrapper.hxx>

class MeshImporter: public IShapeImporter
{
public:
    bool Import(const char* filePath,
                const Handle(AIS_InteractiveContext) & context,
                std::vector<Handle(AIS_Shape)>& shapes,
                const Handle(V3d_View) & view) override;

    std::string GetFileExtensions() const override
    {
        return "obj,stl";
    }
    std::string GetImporterName() const override
    {
        return "Mesh file";
    }

private:
    bool ConfigureSession(Handle(DE_Wrapper) & session, const std::string& ext);
};
