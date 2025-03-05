#pragma once

#include <Standard.hxx>

#include <MeshVS_DataSource.hxx>
#include <MeshVS_EntityType.hxx>
#include <Standard_Integer.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_HArray2OfInteger.hxx>
#include <TColStd_HArray2OfReal.hxx>

#include <Eigen/Dense>

class Mesh_DataSource;
DEFINE_STANDARD_HANDLE(Mesh_DataSource, MeshVS_DataSource)

//! The DataSource for the wrapper of the mesh data.
class Mesh_DataSource: public MeshVS_DataSource
{
public:
    //! Constructor.
    Mesh_DataSource(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F);
    
    //! Constructor with pre-computed normals.
    Mesh_DataSource(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N);

    //! Destructor.
    ~Mesh_DataSource() override;

    //! Returns geometry information about node (if IsElement is False) or element (IsElement is
    //! True) by coordinates. For element this method must return all its nodes coordinates in the
    //! strict order: X, Y, Z and with nodes order is the same as in wire bounding the face or link.
    //! NbNodes is number of nodes of element. It is recommended to return 1 for node. Type is an
    //! element type.
    Standard_Boolean GetGeom(const Standard_Integer ID,
                             const Standard_Boolean IsElement,
                             TColStd_Array1OfReal& Coords,
                             Standard_Integer& NbNodes,
                             MeshVS_EntityType& Type) const Standard_OVERRIDE;

    //! This method is similar to GetGeom, but returns only element or node type. This method is
    //! provided for a fine performance.
    Standard_Boolean GetGeomType(const Standard_Integer ID,
                                 const Standard_Boolean IsElement,
                                 MeshVS_EntityType& Type) const Standard_OVERRIDE;

    //! This method returns by number an address of any entity which represents element or node data
    //! structure.
    Standard_Address GetAddr(const Standard_Integer ID,
                             const Standard_Boolean IsElement) const Standard_OVERRIDE;

    //! This method returns information about what node this element consist of.
    virtual Standard_Boolean GetNodesByElement(const Standard_Integer ID,
                                               TColStd_Array1OfInteger& NodeIDs,
                                               Standard_Integer& NbNodes) const Standard_OVERRIDE;

    //! This method returns map of all nodes the object consist of.
    const TColStd_PackedMapOfInteger& GetAllNodes() const Standard_OVERRIDE;

    //! This method returns map of all elements the object consist of.
    const TColStd_PackedMapOfInteger& GetAllElements() const Standard_OVERRIDE;

    //! This method calculates normal of face, which is using for correct reflection presentation.
    //! There is default method, for advance reflection this method can be redefined.
    virtual Standard_Boolean GetNormal(const Standard_Integer Id,
                                       const Standard_Integer Max,
                                       Standard_Real& nx,
                                       Standard_Real& ny,
                                       Standard_Real& nz) const Standard_OVERRIDE;


    DEFINE_STANDARD_RTTIEXT(Mesh_DataSource, MeshVS_DataSource)

private:
    //! Initialize nodes and elements maps
    void InitMaps();
    
    //! Calculate face normals if not provided
    void CalculateNormals();
    
    TColStd_PackedMapOfInteger myNodes;
    TColStd_PackedMapOfInteger myElements;
    
    Eigen::MatrixXd myV;
    Eigen::MatrixXi myF;
    Eigen::MatrixXd myNormals; // 存储每个面的法向量
};
