#pragma once

#include "IShapeImporter.h"
#include <DE_Wrapper.hxx>

class StepImporter: public IShapeImporter
{
public:
    bool Import(const char* filePath,
                const Handle(AIS_InteractiveContext) & context,
                std::vector<Handle(AIS_InteractiveObject)>& objects) override;

    std::string GetFileExtensions() const override
    {
        return "stp,step";
    }
    std::string GetImporterName() const override
    {
        return "STEP file";
    }
};
