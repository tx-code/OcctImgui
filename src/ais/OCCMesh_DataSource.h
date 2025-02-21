#pragma once

#include <Standard.hxx>

#include <MeshVS_DataSource.hxx>
#include <MeshVS_EntityType.hxx>
#include <Poly_Triangulation.hxx>
#include <Standard_Integer.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_HArray2OfInteger.hxx>
#include <TColStd_HArray2OfReal.hxx>

class OCCMesh_DataSource;
DEFINE_STANDARD_HANDLE(OCCMesh_DataSource, MeshVS_DataSource)

/**
 * @brief This class is a data source for OCC meshes (Poly_Triangulation).
 * @details This class is used to provide data for OCC meshes. It is used to display meshes in the
 * view.
 */
class OCCMesh_DataSource: public MeshVS_DataSource
{
public:
    /**
     * @brief Constructor.
     * @param aMesh The mesh to display.
     */
    OCCMesh_DataSource(const Handle(Poly_Triangulation) & aMesh);

    //! Returns geometry information about node (if IsElement is False) or element (IsElement is
    //! True) by coordinates. For element this method must return all its nodes coordinates in the
    //! strict order: X, Y, Z and with nodes order is the same as in wire bounding the face or link.
    //! NbNodes is number of nodes of element. It is recommended to return 1 for node. Type is an
    //! element type.
    Standard_Boolean GetGeom(const Standard_Integer ID,
                             const Standard_Boolean IsElement,
                             TColStd_Array1OfReal& Coords,
                             Standard_Integer& NbNodes,
                             MeshVS_EntityType& Type) const override;

    //! This method is similar to GetGeom, but returns only element or node type. This method is
    //! provided for a fine performance.
    Standard_Boolean GetGeomType(const Standard_Integer ID,
                                 const Standard_Boolean IsElement,
                                 MeshVS_EntityType& Type) const override;

    //! This method returns by number an address of any entity which represents element or node data
    //! structure.
    Standard_Address GetAddr(const Standard_Integer ID,
                             const Standard_Boolean IsElement) const override;

    //! This method returns information about what node this element consist of.
    Standard_Boolean GetNodesByElement(const Standard_Integer ID,
                                       TColStd_Array1OfInteger& NodeIDs,
                                       Standard_Integer& NbNodes) const override;

    //! This method returns map of all nodes the object consist of.
    const TColStd_PackedMapOfInteger& GetAllNodes() const override;

    //! This method returns map of all elements the object consist of.
    const TColStd_PackedMapOfInteger& GetAllElements() const override;

    //! This method calculates normal of face, which is using for correct reflection presentation.
    //! There is default method, for advance reflection this method can be redefined.
    Standard_Boolean GetNormal(const Standard_Integer Id,
                               const Standard_Integer Max,
                               Standard_Real& nx,
                               Standard_Real& ny,
                               Standard_Real& nz) const override;

    DEFINE_STANDARD_RTTIEXT(OCCMesh_DataSource, MeshVS_DataSource);

private:
    Handle(Poly_Triangulation) myMesh;
    TColStd_PackedMapOfInteger myNodes;
    TColStd_PackedMapOfInteger myElements;
    Handle(TColStd_HArray2OfInteger) myElemNodes;
    Handle(TColStd_HArray2OfReal) myNodeCoords;
    Handle(TColStd_HArray2OfReal) myElemNormals;
};
