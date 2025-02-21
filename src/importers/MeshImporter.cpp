#include "MeshImporter.h"
#include <Message.hxx>
#include <RWObj_ConfigurationNode.hxx>
#include <RWStl_ConfigurationNode.hxx>
#include <filesystem>
#include <Poly_Triangulation.hxx>
#include <MeshVS_DataSource.hxx>


bool MeshImporter::Import(const char* filePath,
                          const Handle(AIS_InteractiveContext) & context,
                          std::vector<Handle(AIS_Shape)>& shapes,
                          const Handle(V3d_View) & view)
{
    // 获取DE会话
    Handle(DE_Wrapper) aOneTimeSession = DE_Wrapper::GlobalWrapper()->Copy();

    // 获取文件扩展名
    std::string ext = std::filesystem::path(filePath).extension().string().substr(1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (!ConfigureSession(aOneTimeSession, ext)) {
        return false;
    }

    // 读取CAD模型
    TopoDS_Shape aShRes;
    if (!aOneTimeSession->Read(filePath, aShRes)) {
        Message::SendFail() << "Error: Can't read file from " << filePath << "\n";
        return false;
    }

    // 显示新形状
    Handle(AIS_Shape) aShape = new AIS_Shape(aShRes);
    context->Display(aShape, AIS_Shaded, 0, true);
    shapes.push_back(aShape);

    // 调整视图
    view->FitAll();
    view->ZFitAll();
    view->Redraw();

    return true;
}

bool MeshImporter::ConfigureSession(Handle(DE_Wrapper) & session, const std::string& ext)
{
    // TODO， don't know how to configure session for mesh file
    if (ext == "obj") {
        auto aNode = new RWObj_ConfigurationNode;
        return session->Bind(aNode);
    }
    else if (ext == "stl") {
        auto aNode = new RWStl_ConfigurationNode;
        return session->Bind(aNode);
    }
    Message::SendFail() << "Error: Unsupported file extension: " << ext;
    return false;
}