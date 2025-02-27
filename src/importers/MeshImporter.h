#pragma once

#include "IShapeImporter.h"

class MeshImporter: public IShapeImporter
{
public:
    bool Import(const char* filePath,
                const Handle(AIS_InteractiveContext) & context,
                std::vector<Handle(AIS_InteractiveObject)>& objects) override;

    std::string GetFileExtensions() const override
    {
        return "obj,stl";
    }
    std::string GetImporterName() const override
    {
        return "Mesh file";
    }
};
