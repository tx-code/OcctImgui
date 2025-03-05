#include "Mesh_DataSource.h"

#include <Precision.hxx>
#include <Standard_Type.hxx>

IMPLEMENT_STANDARD_RTTIEXT(Mesh_DataSource, MeshVS_DataSource)

//================================================================
// Function : Constructor
// Purpose  :
//================================================================
Mesh_DataSource::Mesh_DataSource(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F)
    : myV(V), myF(F)
{
    if (V.rows() > 0 && F.rows() > 0)
    {
        // 初始化节点和元素映射
        InitMaps();
        
        // 计算法向量
        CalculateNormals();
    }
}

//================================================================
// Function : Constructor with pre-computed normals
// Purpose  :
//================================================================
Mesh_DataSource::Mesh_DataSource(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N)
    : myV(V), myF(F), myNormals(N)
{
    if (V.rows() > 0 && F.rows() > 0)
    {
        // 初始化节点和元素映射
        InitMaps();
        
        // 如果提供的法向量尺寸不匹配，则重新计算
        if (N.rows() != F.rows() || N.cols() != 3)
        {
            CalculateNormals();
        }
        else
        {
            // 确保法向量已归一化
            for (int i = 0; i < N.rows(); ++i)
            {
                double length = N.row(i).norm();
                if (length > Precision::Confusion())
                {
                    myNormals.row(i) = N.row(i) / length;
                }
                else
                {
                    myNormals.row(i) = Eigen::Vector3d::Zero();
                }
            }
        }
    }
}

//================================================================
// Function : InitMaps
// Purpose  : Initialize nodes and elements maps
//================================================================
void Mesh_DataSource::InitMaps()
{
    const Standard_Integer aNbNodes = myV.rows();
    
    // Add all nodes
    for (Standard_Integer i = 1; i <= aNbNodes; i++)
    {
        myNodes.Add(i);
    }
    
    const Standard_Integer aNbTris = myF.rows();
    
    // Add all elements (triangles)
    for (Standard_Integer i = 1; i <= aNbTris; i++)
    {
        myElements.Add(i);
    }
}

//================================================================
// Function : CalculateNormals
// Purpose  : Calculate face normals if not provided
//================================================================
void Mesh_DataSource::CalculateNormals()
{
    const Standard_Integer aNbTris = myF.rows();
    
    // 初始化法向量矩阵
    myNormals = Eigen::MatrixXd::Zero(aNbTris, 3);
    
    // 计算每个面的法向量
    for (Standard_Integer i = 1; i <= aNbTris; i++)
    {
        // Get triangle vertices (Eigen is 0-indexed, OCCT arrays are 1-indexed)
        Standard_Integer V1 = myF(i-1, 0) + 1;
        Standard_Integer V2 = myF(i-1, 1) + 1;
        Standard_Integer V3 = myF(i-1, 2) + 1;
        
        // Calculate normal
        const gp_Pnt aP1(myV(V1-1, 0), myV(V1-1, 1), myV(V1-1, 2));
        const gp_Pnt aP2(myV(V2-1, 0), myV(V2-1, 1), myV(V2-1, 2));
        const gp_Pnt aP3(myV(V3-1, 0), myV(V3-1, 1), myV(V3-1, 2));
        
        gp_Vec aV1(aP1, aP2);
        gp_Vec aV2(aP2, aP3);
        
        gp_Vec aN = aV1.Crossed(aV2);
        if (aN.SquareMagnitude() > Precision::SquareConfusion())
            aN.Normalize();
        else
            aN.SetCoord(0.0, 0.0, 0.0);
        
        // 存储法向量到 Eigen 矩阵
        myNormals(i-1, 0) = aN.X();
        myNormals(i-1, 1) = aN.Y();
        myNormals(i-1, 2) = aN.Z();
    }
}

//================================================================
// Function : Destructor
// Purpose  :
//================================================================
Mesh_DataSource::~Mesh_DataSource()
{
    // Nothing to do here - handles will be released automatically
}

//================================================================
// Function : GetGeom
// Purpose  :
//================================================================
Standard_Boolean Mesh_DataSource::GetGeom(
    const Standard_Integer ID, 
    const Standard_Boolean IsElement,
    TColStd_Array1OfReal& Coords, 
    Standard_Integer& NbNodes,
    MeshVS_EntityType& Type) const
{
    if (myV.rows() == 0 || myF.rows() == 0)
        return Standard_False;
    
    if (IsElement)
    {
        if (ID >= 1 && ID <= myElements.Extent())
        {
            Type = MeshVS_ET_Face;
            NbNodes = 3;
            
            // 获取三角形的三个顶点索引 (注意索引转换)
            Standard_Integer V1 = myF(ID-1, 0) + 1;
            Standard_Integer V2 = myF(ID-1, 1) + 1;
            Standard_Integer V3 = myF(ID-1, 2) + 1;
            
            // 填充坐标数组
            Standard_Integer k = Coords.Lower();
            
            // 第一个顶点
            Coords(k++) = myV(V1-1, 0);
            Coords(k++) = myV(V1-1, 1);
            Coords(k++) = myV(V1-1, 2);
            
            // 第二个顶点
            Coords(k++) = myV(V2-1, 0);
            Coords(k++) = myV(V2-1, 1);
            Coords(k++) = myV(V2-1, 2);
            
            // 第三个顶点
            Coords(k++) = myV(V3-1, 0);
            Coords(k++) = myV(V3-1, 1);
            Coords(k++) = myV(V3-1, 2);
            
            return Standard_True;
        }
        else
            return Standard_False;
    }
    else
    {
        if (ID >= 1 && ID <= myNodes.Extent())
        {
            Type = MeshVS_ET_Node;
            NbNodes = 1;
            
            // 直接从 Eigen 矩阵获取节点坐标 (注意索引转换)
            Standard_Integer k = Coords.Lower();
            Coords(k++) = myV(ID-1, 0);
            Coords(k++) = myV(ID-1, 1);
            Coords(k++) = myV(ID-1, 2);
            
            return Standard_True;
        }
        else
            return Standard_False;
    }
}

//================================================================
// Function : GetGeomType
// Purpose  :
//================================================================
Standard_Boolean Mesh_DataSource::GetGeomType(
    const Standard_Integer,
    const Standard_Boolean IsElement,
    MeshVS_EntityType& Type) const
{
    if (IsElement)
    {
        Type = MeshVS_ET_Face;
        return Standard_True;
    }
    else
    {
        Type = MeshVS_ET_Node;
        return Standard_True;
    }
}

//================================================================
// Function : GetAddr
// Purpose  :
//================================================================
Standard_Address Mesh_DataSource::GetAddr(
    const Standard_Integer,
    const Standard_Boolean) const
{
    return NULL;
}

//================================================================
// Function : GetNodesByElement
// Purpose  :
//================================================================
Standard_Boolean Mesh_DataSource::GetNodesByElement(
    const Standard_Integer ID,
    TColStd_Array1OfInteger& NodeIDs,
    Standard_Integer& NbNodes) const
{
    if (myV.rows() == 0 || myF.rows() == 0)
        return Standard_False;
    
    if (ID >= 1 && ID <= myElements.Extent() && NodeIDs.Length() >= 3)
    {
        Standard_Integer aLow = NodeIDs.Lower();
        
        // 直接从 Eigen 矩阵获取节点索引 (注意索引转换)
        NodeIDs(aLow)     = myF(ID-1, 0) + 1;
        NodeIDs(aLow + 1) = myF(ID-1, 1) + 1;
        NodeIDs(aLow + 2) = myF(ID-1, 2) + 1;
        
        NbNodes = 3;
        return Standard_True;
    }
    return Standard_False;
}

//================================================================
// Function : GetAllNodes
// Purpose  :
//================================================================
const TColStd_PackedMapOfInteger& Mesh_DataSource::GetAllNodes() const
{
    return myNodes;
}

//================================================================
// Function : GetAllElements
// Purpose  :
//================================================================
const TColStd_PackedMapOfInteger& Mesh_DataSource::GetAllElements() const
{
    return myElements;
}

//================================================================
// Function : GetNormal
// Purpose  :
//================================================================
Standard_Boolean Mesh_DataSource::GetNormal(
    const Standard_Integer Id, 
    const Standard_Integer Max,
    Standard_Real& nx, 
    Standard_Real& ny, 
    Standard_Real& nz) const
{
    if (myV.rows() == 0 || myF.rows() == 0)
        return Standard_False;
    
    if (Id >= 1 && Id <= myElements.Extent() && Max >= 3)
    {
        // 从 Eigen 矩阵中获取法向量 (注意索引转换)
        nx = myNormals(Id-1, 0);
        ny = myNormals(Id-1, 1);
        nz = myNormals(Id-1, 2);
        return Standard_True;
    }
    else
        return Standard_False;
}
