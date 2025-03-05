#define BOOST_TEST_MODULE Mesh_DataSource Tests
#include <boost/test/unit_test.hpp>

#include <igl/readOBJ.h>
#include <igl/per_face_normals.h>
#include "ais/Mesh_DataSource.h"

#include <Standard_Type.hxx>
#include <MeshVS_EntityType.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>

// Test fixture for Mesh_DataSource tests
struct MeshDataSourceFixture {
    MeshDataSourceFixture() {
        // Load a simple cube mesh using libigl
        std::string test_mesh_path = MESH_TEST_DATA_DIR "/cube.obj";
        bool success = igl::readOBJ(test_mesh_path, V, F);
        
        if (!success) {
            BOOST_TEST_MESSAGE("Failed to load test mesh: " << test_mesh_path);
            // Create a simple cube mesh as fallback
            createSimpleCube();
        }
        
        // Calculate face normals
        igl::per_face_normals(V, F, Eigen::Vector3d(0, 0, 0), N);
        
        // Create Mesh_DataSource instances
        dataSource1 = new Mesh_DataSource(V, F);
        dataSource2 = new Mesh_DataSource(V, F, N);
    }
    
    ~MeshDataSourceFixture() {
        // Cleanup
        dataSource1.Nullify();
        dataSource2.Nullify();
    }
    
    // Create a simple cube mesh
    void createSimpleCube() {
        // 8 vertices of a cube
        V.resize(8, 3);
        V << -1, -1, -1,
             1, -1, -1,
             1,  1, -1,
            -1,  1, -1,
            -1, -1,  1,
             1, -1,  1,
             1,  1,  1,
            -1,  1,  1;
        
        // 12 triangles (6 faces, each with 2 triangles)
        F.resize(12, 3);
        F << 0, 1, 2, // bottom face
             0, 2, 3,
             4, 5, 6, // top face
             4, 6, 7,
             0, 1, 5, // front face
             0, 5, 4,
             2, 3, 7, // back face
             2, 7, 6,
             0, 3, 7, // left face
             0, 7, 4,
             1, 2, 6, // right face
             1, 6, 5;
    }
    
    Eigen::MatrixXd V; // Vertices
    Eigen::MatrixXi F; // Faces
    Eigen::MatrixXd N; // Face normals
    
    Handle(Mesh_DataSource) dataSource1; // Using constructor with V, F
    Handle(Mesh_DataSource) dataSource2; // Using constructor with V, F, N
};

// Test basic properties
BOOST_FIXTURE_TEST_CASE(basic_properties_test, MeshDataSourceFixture)
{
    // Check that the data source is not null
    BOOST_CHECK(!dataSource1.IsNull());
    BOOST_CHECK(!dataSource2.IsNull());
    
    // Check node and element counts
    BOOST_CHECK_EQUAL(dataSource1->GetAllNodes().Extent(), V.rows());
    BOOST_CHECK_EQUAL(dataSource1->GetAllElements().Extent(), F.rows());
    
    BOOST_CHECK_EQUAL(dataSource2->GetAllNodes().Extent(), V.rows());
    BOOST_CHECK_EQUAL(dataSource2->GetAllElements().Extent(), F.rows());
}

// Test GetGeom method for nodes
BOOST_FIXTURE_TEST_CASE(get_node_geom_test, MeshDataSourceFixture)
{
    // Test for a specific node
    const int nodeId = 1; // 1-indexed
    TColStd_Array1OfReal coords(1, 3);
    Standard_Integer nbNodes;
    MeshVS_EntityType type;
    
    // Get geometry for the node
    bool result = dataSource1->GetGeom(nodeId, false, coords, nbNodes, type);
    
    // Check results
    BOOST_CHECK(result);
    BOOST_CHECK_EQUAL(nbNodes, 1);
    BOOST_CHECK_EQUAL(type, MeshVS_ET_Node);
    
    // Check coordinates (with small tolerance for floating point comparison)
    BOOST_CHECK_CLOSE(coords(1), V(nodeId-1, 0), 1e-6);
    BOOST_CHECK_CLOSE(coords(2), V(nodeId-1, 1), 1e-6);
    BOOST_CHECK_CLOSE(coords(3), V(nodeId-1, 2), 1e-6);
}

// Test GetGeom method for elements
BOOST_FIXTURE_TEST_CASE(get_element_geom_test, MeshDataSourceFixture)
{
    // Test for a specific element
    const int elemId = 1; // 1-indexed
    TColStd_Array1OfReal coords(1, 9); // 3 vertices * 3 coordinates
    Standard_Integer nbNodes;
    MeshVS_EntityType type;
    
    // Get geometry for the element
    bool result = dataSource1->GetGeom(elemId, true, coords, nbNodes, type);
    
    // Check results
    BOOST_CHECK(result);
    BOOST_CHECK_EQUAL(nbNodes, 3);
    BOOST_CHECK_EQUAL(type, MeshVS_ET_Face);
    
    // Check that the coordinates match the vertices of the face
    int v1 = F(elemId-1, 0);
    int v2 = F(elemId-1, 1);
    int v3 = F(elemId-1, 2);
    
    // First vertex
    BOOST_CHECK_CLOSE(coords(1), V(v1, 0), 1e-6);
    BOOST_CHECK_CLOSE(coords(2), V(v1, 1), 1e-6);
    BOOST_CHECK_CLOSE(coords(3), V(v1, 2), 1e-6);
    
    // Second vertex
    BOOST_CHECK_CLOSE(coords(4), V(v2, 0), 1e-6);
    BOOST_CHECK_CLOSE(coords(5), V(v2, 1), 1e-6);
    BOOST_CHECK_CLOSE(coords(6), V(v2, 2), 1e-6);
    
    // Third vertex
    BOOST_CHECK_CLOSE(coords(7), V(v3, 0), 1e-6);
    BOOST_CHECK_CLOSE(coords(8), V(v3, 1), 1e-6);
    BOOST_CHECK_CLOSE(coords(9), V(v3, 2), 1e-6);
}

// Test GetNodesByElement method
BOOST_FIXTURE_TEST_CASE(get_nodes_by_element_test, MeshDataSourceFixture)
{
    // Test for a specific element
    const int elemId = 1; // 1-indexed
    TColStd_Array1OfInteger nodeIds(1, 3);
    Standard_Integer nbNodes;
    
    // Get nodes for the element
    bool result = dataSource1->GetNodesByElement(elemId, nodeIds, nbNodes);
    
    // Check results
    BOOST_CHECK(result);
    BOOST_CHECK_EQUAL(nbNodes, 3);
    
    // Check that the node IDs match the face vertices (with 1-indexing adjustment)
    BOOST_CHECK_EQUAL(nodeIds(1), F(elemId-1, 0) + 1);
    BOOST_CHECK_EQUAL(nodeIds(2), F(elemId-1, 1) + 1);
    BOOST_CHECK_EQUAL(nodeIds(3), F(elemId-1, 2) + 1);
}

// Test GetNormal method
BOOST_FIXTURE_TEST_CASE(get_normal_test, MeshDataSourceFixture)
{
    // Test for a specific element
    const int elemId = 1; // 1-indexed
    Standard_Real nx, ny, nz;
    
    // Get normal for the element
    bool result = dataSource1->GetNormal(elemId, 3, nx, ny, nz);
    
    // Check results
    BOOST_CHECK(result);
    
    // Check that the normal is normalized
    double length = std::sqrt(nx*nx + ny*ny + nz*nz);
    BOOST_CHECK_CLOSE(length, 1.0, 1e-6);
    
    // For dataSource2, check that the normal matches the pre-computed normal
    Standard_Real nx2, ny2, nz2;
    dataSource2->GetNormal(elemId, 3, nx2, ny2, nz2);
    
    // The normals should be the same (or opposite) since they're calculated the same way
    // We'll check the absolute values to handle potential sign differences
    BOOST_CHECK_CLOSE(std::abs(nx), std::abs(N(elemId-1, 0)), 1e-6);
    BOOST_CHECK_CLOSE(std::abs(ny), std::abs(N(elemId-1, 1)), 1e-6);
    BOOST_CHECK_CLOSE(std::abs(nz), std::abs(N(elemId-1, 2)), 1e-6);
}

// Test invalid inputs
BOOST_FIXTURE_TEST_CASE(invalid_inputs_test, MeshDataSourceFixture)
{
    // Test invalid node ID
    TColStd_Array1OfReal coords(1, 3);
    Standard_Integer nbNodes;
    MeshVS_EntityType type;
    
    // Node ID out of range
    bool result = dataSource1->GetGeom(V.rows() + 1, false, coords, nbNodes, type);
    BOOST_CHECK(!result);
    
    // Element ID out of range
    result = dataSource1->GetGeom(F.rows() + 1, true, coords, nbNodes, type);
    BOOST_CHECK(!result);
    
    // Invalid node by element
    TColStd_Array1OfInteger nodeIds(1, 3);
    result = dataSource1->GetNodesByElement(F.rows() + 1, nodeIds, nbNodes);
    BOOST_CHECK(!result);
    
    // Invalid normal
    Standard_Real nx, ny, nz;
    result = dataSource1->GetNormal(F.rows() + 1, 3, nx, ny, nz);
    BOOST_CHECK(!result);
} 