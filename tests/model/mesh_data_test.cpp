#define BOOST_TEST_MODULE Model MeshData Tests
#include <boost/test/unit_test.hpp>

#include "model/MeshData.h"
#include <Eigen/Dense>
#include <nlohmann/json.hpp>
#include <vector>

using namespace Model;

BOOST_AUTO_TEST_CASE(mesh_data_constructor_test)
{
    // Test default constructor
    MeshData emptyMesh;
    BOOST_CHECK_EQUAL(emptyMesh.getVertices().rows(), 0);
    BOOST_CHECK_EQUAL(emptyMesh.getVertices().cols(), 0);
    BOOST_CHECK_EQUAL(emptyMesh.getFaces().rows(), 0);
    BOOST_CHECK_EQUAL(emptyMesh.getFaces().cols(), 0);
    BOOST_CHECK_EQUAL(emptyMesh.getNormals().rows(), 0);
    BOOST_CHECK_EQUAL(emptyMesh.getNormals().cols(), 0);

    // Test parameterized constructor
    Eigen::MatrixXd vertices(3, 3);
    vertices << 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0;

    Eigen::MatrixXi faces(1, 3);
    faces << 0, 1, 2;

    Eigen::MatrixXd normals(1, 3);
    normals << 0.0, 0.0, 1.0;

    MeshData mesh(vertices, faces, normals);
    BOOST_CHECK_EQUAL(mesh.getVertices().rows(), 3);
    BOOST_CHECK_EQUAL(mesh.getVertices().cols(), 3);
    BOOST_CHECK_EQUAL(mesh.getFaces().rows(), 1);
    BOOST_CHECK_EQUAL(mesh.getFaces().cols(), 3);
    BOOST_CHECK_EQUAL(mesh.getNormals().rows(), 1);
    BOOST_CHECK_EQUAL(mesh.getNormals().cols(), 3);
}

BOOST_AUTO_TEST_CASE(mesh_data_set_get_value_test)
{
    MeshData mesh;

    Eigen::MatrixXd vertices(3, 3);
    vertices << 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0;

    Eigen::MatrixXi faces(1, 3);
    faces << 0, 1, 2;

    Eigen::MatrixXd normals(1, 3);
    normals << 0.0, 0.0, 1.0;

    // Test setValue
    mesh.setValue(vertices, faces, normals);

    BOOST_CHECK(mesh.getVertices() == vertices);
    BOOST_CHECK(mesh.getFaces() == faces);
    BOOST_CHECK(mesh.getNormals() == normals);

    // Test getValue
    Eigen::MatrixXd outVertices;
    Eigen::MatrixXi outFaces;
    Eigen::MatrixXd outNormals;

    mesh.getValue(outVertices, outFaces, outNormals);

    BOOST_CHECK(outVertices == vertices);
    BOOST_CHECK(outFaces == faces);
    BOOST_CHECK(outNormals == normals);
}

BOOST_AUTO_TEST_CASE(mesh_data_equality_test)
{
    Eigen::MatrixXd vertices1(3, 3);
    vertices1 << 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0;

    Eigen::MatrixXi faces1(1, 3);
    faces1 << 0, 1, 2;

    Eigen::MatrixXd normals1(1, 3);
    normals1 << 0.0, 0.0, 1.0;

    MeshData mesh1(vertices1, faces1, normals1);
    MeshData mesh2(vertices1, faces1, normals1);

    // Test equality operator
    BOOST_CHECK(mesh1 == mesh2);
    BOOST_CHECK(!(mesh1 != mesh2));

    // Modify mesh2 and test inequality
    Eigen::MatrixXd vertices2(3, 3);
    vertices2 << 0.0, 0.0, 0.0, 2.0, 0.0, 0.0,  // Changed from 1.0 to 2.0
        0.0, 1.0, 0.0;

    mesh2.setValue(vertices2, faces1, normals1);
    BOOST_CHECK(mesh1 != mesh2);
    BOOST_CHECK(!(mesh1 == mesh2));
}

BOOST_AUTO_TEST_CASE(mesh_data_json_serialization_test)
{
    // Create a mesh with known data
    Eigen::MatrixXd vertices(3, 3);
    vertices << 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0;

    Eigen::MatrixXi faces(1, 3);
    faces << 0, 1, 2;

    Eigen::MatrixXd normals(1, 3);
    normals << 0.0, 0.0, 1.0;

    MeshData originalMesh(vertices, faces, normals);

    // Serialize to JSON
    nlohmann::json j;
    // Use the member function directly instead of the free function
    originalMesh.to_json(j);

    // Verify JSON structure
    BOOST_CHECK(j.contains("vertices"));
    BOOST_CHECK(j.contains("faces"));
    BOOST_CHECK(j.contains("normals"));
    BOOST_CHECK(j["vertices"].contains("data"));
    BOOST_CHECK(j["faces"].contains("data"));
    BOOST_CHECK(j["normals"].contains("data"));

    // Verify data sizes
    BOOST_CHECK_EQUAL(j["vertices"]["data"].size(), 9);  // 3 vertices * 3 coordinates
    BOOST_CHECK_EQUAL(j["faces"]["data"].size(), 3);     // 1 face * 3 indices
    BOOST_CHECK_EQUAL(j["normals"]["data"].size(), 3);   // 1 normal * 3 coordinates

    // Deserialize from JSON
    MeshData deserializedMesh;
    // Use the member function directly instead of the free function
    deserializedMesh.from_json(j);

    // Verify deserialized mesh matches original
    BOOST_CHECK(deserializedMesh == originalMesh);

    // Check specific values
    BOOST_CHECK_EQUAL(deserializedMesh.getVertices()(0, 0), 0.0);
    BOOST_CHECK_EQUAL(deserializedMesh.getVertices()(1, 0), 1.0);
    BOOST_CHECK_EQUAL(deserializedMesh.getVertices()(2, 0), 0.0);

    BOOST_CHECK_EQUAL(deserializedMesh.getFaces()(0, 0), 0);
    BOOST_CHECK_EQUAL(deserializedMesh.getFaces()(0, 1), 1);
    BOOST_CHECK_EQUAL(deserializedMesh.getFaces()(0, 2), 2);

    BOOST_CHECK_EQUAL(deserializedMesh.getNormals()(0, 0), 0.0);
    BOOST_CHECK_EQUAL(deserializedMesh.getNormals()(0, 1), 0.0);
    BOOST_CHECK_EQUAL(deserializedMesh.getNormals()(0, 2), 1.0);
}

BOOST_AUTO_TEST_CASE(mesh_data_json_edge_cases_test)
{
    // Test empty mesh serialization/deserialization
    MeshData emptyMesh;
    nlohmann::json emptyJson;
    // Use the member function directly instead of the free function
    emptyMesh.to_json(emptyJson);

    MeshData deserializedEmpty;
    // Use the member function directly instead of the free function
    deserializedEmpty.from_json(emptyJson);

    BOOST_CHECK(emptyMesh == deserializedEmpty);

    // Test mesh with only vertices
    Eigen::MatrixXd vertices(3, 3);
    vertices << 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0;

    Eigen::MatrixXi emptyFaces(0, 3);
    Eigen::MatrixXd emptyNormals(0, 3);

    MeshData verticesOnlyMesh;
    verticesOnlyMesh.setValue(vertices, emptyFaces, emptyNormals);

    nlohmann::json verticesOnlyJson;
    // Use the member function directly instead of the free function
    verticesOnlyMesh.to_json(verticesOnlyJson);

    MeshData deserializedVerticesOnly;
    // Use the member function directly instead of the free function
    deserializedVerticesOnly.from_json(verticesOnlyJson);

    BOOST_CHECK(verticesOnlyMesh == deserializedVerticesOnly);
}