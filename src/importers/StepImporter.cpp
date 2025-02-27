#include "StepImporter.h"
#include <spdlog/spdlog.h>

#include <AIS_Shape.hxx>
#include <BRepTools.hxx>
#include <STEPControl_Reader.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Shape.hxx>

bool StepImporter::Import(const char* filePath,
                          const Handle(AIS_InteractiveContext) & context,
                          std::vector<Handle(AIS_InteractiveObject)>& objects)
{
    spdlog::debug("Starting STEP file import: {}", filePath);
    
    STEPControl_Reader reader;
    IFSelect_ReturnStatus status = reader.ReadFile(filePath);
    
    if (status != IFSelect_RetDone) {
        spdlog::error("Failed to read STEP file: {}", filePath);
        return false;
    }
    
    spdlog::debug("STEP file read successfully, preparing conversion");
    
    // 转换
    reader.TransferRoots();
    TopoDS_Shape shape = reader.OneShape();
    
    if (shape.IsNull()) {
        spdlog::error("Failed to create shape from STEP file");
        return false;
    }
    
    spdlog::debug("STEP shape created successfully, adding to interactive context");
    
    // 创建可视化对象并添加到上下文
    Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
    context->Display(aisShape, Standard_False);
    objects.push_back(aisShape);
    
    spdlog::debug("STEP model imported and displayed successfully");
    
    return true;
}