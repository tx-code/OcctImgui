#include "OCCMesh_DataSource.h"

#include <Message.hxx>
#include <Precision.hxx>
#include <Standard_Type.hxx>
#include <TColStd_DataMapOfIntegerReal.hxx>


IMPLEMENT_STANDARD_RTTIEXT(OCCMesh_DataSource, MeshVS_DataSource)

OCCMesh_DataSource::OCCMesh_DataSource(const Handle(Poly_Triangulation) & aMesh)
    : myMesh(aMesh)
{
    if (!myMesh.IsNull()) {
        const auto aNbNodes = myMesh->NbNodes();
        myNodeCoords = new TColStd_HArray2OfReal(1, aNbNodes, 1, 3);
        Message::SendInfo() << "Nodes: " << aNbNodes;

        for (Standard_Integer i = 1; i <= aNbNodes; i++) {
            myNodes.Add(i);
            gp_Pnt xyz = myMesh->Node(i);

            myNodeCoords->SetValue(i, 1, xyz.X());
            myNodeCoords->SetValue(i, 2, xyz.Y());
            myNodeCoords->SetValue(i, 3, xyz.Z());
        }

        const auto aNbTris = myMesh->NbTriangles();
        myElemNormals = new TColStd_HArray2OfReal(1, aNbTris, 1, 3);
        myElemNodes = new TColStd_HArray2OfInteger(1, aNbTris, 1, 3);

        Message::SendInfo() << "Elements: " << aNbTris;

        for (Standard_Integer i = 1; i <= aNbTris; i++) {
            myElements.Add(i);

            const Poly_Triangle aTri = myMesh->Triangle(i);

            std::array<Standard_Integer, 3> V;
            aTri.Get(V[0], V[1], V[2]);

            const auto aP1 = myMesh->Node(V[0]);
            const auto aP2 = myMesh->Node(V[1]);
            const auto aP3 = myMesh->Node(V[2]);

            gp_Vec aV1(aP1, aP2);
            gp_Vec aV2(aP2, aP3);

            gp_Vec aN = aV1.Crossed(aV2);
            if (aN.SquareMagnitude() > Precision::SquareConfusion()) {
                aN.Normalize();
            }
            else {
                aN.SetCoord(0., 0., 0.);
            }

            for (Standard_Integer j = 0; j < 3; j++) {
                myElemNodes->SetValue(i, j + 1, V[j]);
            }

            myElemNormals->SetValue(i, 1, aN.X());
            myElemNormals->SetValue(i, 2, aN.Y());
            myElemNormals->SetValue(i, 3, aN.Z());
        }
    }


    Message::SendInfo() << "OCCMesh_DataSource created";
}

Standard_Boolean OCCMesh_DataSource::GetGeom(const Standard_Integer ID,
                                             const Standard_Boolean IsElement,
                                             TColStd_Array1OfReal& Coords,
                                             Standard_Integer& NbNodes,
                                             MeshVS_EntityType& Type) const
{
    if (myMesh.IsNull())
        return Standard_False;

    if (IsElement) {
        if (ID >= 1 && ID <= myElements.Extent()) {
            Type = MeshVS_ET_Face;
            NbNodes = 3;

            for (Standard_Integer i = 1, k = 1; i <= 3; i++) {
                Standard_Integer IdxNode = myElemNodes->Value(ID, i);
                for (Standard_Integer j = 1; j <= 3; j++, k++)
                    Coords(k) = myNodeCoords->Value(IdxNode, j);
            }

            return Standard_True;
        }
        else
            return Standard_False;
    }
    else if (ID >= 1 && ID <= myNodes.Extent()) {
        Type = MeshVS_ET_Node;
        NbNodes = 1;

        Coords(1) = myNodeCoords->Value(ID, 1);
        Coords(2) = myNodeCoords->Value(ID, 2);
        Coords(3) = myNodeCoords->Value(ID, 3);
        return Standard_True;
    }
    else
        return Standard_False;
}

Standard_Boolean OCCMesh_DataSource::GetGeomType(const Standard_Integer ID,
                                                 const Standard_Boolean IsElement,
                                                 MeshVS_EntityType& Type) const
{
    if (IsElement) {
        Type = MeshVS_ET_Face;
        return Standard_True;
    }
    else {
        Type = MeshVS_ET_Node;
        return Standard_True;
    }
}

Standard_Address OCCMesh_DataSource::GetAddr(const Standard_Integer ID,
                                             const Standard_Boolean IsElement) const
{
    // TODO: Implement
    return nullptr;
}

Standard_Boolean OCCMesh_DataSource::GetNodesByElement(const Standard_Integer ID,
                                                       TColStd_Array1OfInteger& NodeIDs,
                                                       Standard_Integer& NbNodes) const
{
    if (myMesh.IsNull())
        return Standard_False;

    if (ID >= 1 && ID <= myElements.Extent() && NodeIDs.Length() >= 3) {
        Standard_Integer aLow = NodeIDs.Lower();
        NodeIDs(aLow) = myElemNodes->Value(ID, 1);
        NodeIDs(aLow + 1) = myElemNodes->Value(ID, 2);
        NodeIDs(aLow + 2) = myElemNodes->Value(ID, 3);
        return Standard_True;
    }
    return Standard_False;
}

const TColStd_PackedMapOfInteger& OCCMesh_DataSource::GetAllNodes() const
{
    return myNodes;
}

const TColStd_PackedMapOfInteger& OCCMesh_DataSource::GetAllElements() const
{
    return myElements;
}

Standard_Boolean OCCMesh_DataSource::GetNormal(const Standard_Integer Id,
                                               const Standard_Integer Max,
                                               Standard_Real& nx,
                                               Standard_Real& ny,
                                               Standard_Real& nz) const
{
    if (myMesh.IsNull()) {
        return Standard_False;
    }

    if (Id >= 1 && Id <= myElements.Extent() && Max >= 3) {
        nx = myElemNormals->Value(Id, 1);
        ny = myElemNormals->Value(Id, 2);
        nz = myElemNormals->Value(Id, 3);
        return Standard_True;
    }

    return Standard_False;
}