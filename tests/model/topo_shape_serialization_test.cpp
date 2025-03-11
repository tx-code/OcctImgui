#define BOOST_TEST_MODULE Model TopoShapeSerialization Tests
#include <boost/test/unit_test.hpp>

#include "model/TopoShapeSerialization.h"
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <STEPControl_Reader.hxx>
#include <TopoDS_Shape.hxx>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <vector>


using namespace nlohmann;

// Helper function to load a STEP file
TopoDS_Shape loadSTEPFile(const std::string& filename)
{
    STEPControl_Reader reader;
    IFSelect_ReturnStatus status = reader.ReadFile(filename.c_str());
    BOOST_CHECK_EQUAL(status, IFSelect_RetDone);

    // Check if any shapes were loaded
    BOOST_CHECK(reader.NbRootsForTransfer() > 0);

    // Transfer all shapes
    reader.TransferRoots();

    // Get the shape
    BOOST_CHECK(reader.NbShapes() > 0);

    // Return the first shape
    return reader.OneShape();
}

// Test fixture to manage test resources
struct TopoShapeSerializationFixture
{
    std::string stepFilePath;

    TopoShapeSerializationFixture()
    {
        // Path to test STEP file
        stepFilePath = TEST_DATA_DIR "/ANC101.stp";
    }
};

BOOST_FIXTURE_TEST_CASE(topo_shape_serialization_test, TopoShapeSerializationFixture)
{
    // Load the STEP file
    TopoDS_Shape originalShape = loadSTEPFile(stepFilePath);
    BOOST_CHECK(!originalShape.IsNull());

    // Serialize to JSON
    json j = originalShape;

    // Verify JSON structure
    BOOST_CHECK(j.contains("TopoDS_Shape"));
    BOOST_CHECK(j["TopoDS_Shape"].contains("format"));
    BOOST_CHECK_EQUAL(j["TopoDS_Shape"]["format"], "BREP");
    BOOST_CHECK(j["TopoDS_Shape"].contains("data"));
    BOOST_CHECK(!j["TopoDS_Shape"]["data"].get<std::string>().empty());

    // Deserialize from JSON
    TopoDS_Shape deserializedShape = j;

    // Verify the shape is not null
    BOOST_CHECK(!deserializedShape.IsNull());

    // Since we can't directly compare the shapes (OpenCASCADE doesn't provide a reliable equality
    // operator), we'll serialize both shapes to BREP format and compare the string representations

    // Serialize original shape to string
    std::ostringstream originalStream;
    BRepTools::Write(originalShape, originalStream);
    std::string originalStr = originalStream.str();

    // Serialize deserialized shape to string
    std::ostringstream deserializedStream;
    BRepTools::Write(deserializedShape, deserializedStream);
    std::string deserializedStr = deserializedStream.str();

    // Compare the string representations
    BOOST_CHECK_EQUAL(originalStr, deserializedStr);
}

BOOST_AUTO_TEST_CASE(topo_shape_empty_test)
{
    // Test with an empty shape
    TopoDS_Shape emptyShape;
    BOOST_CHECK(emptyShape.IsNull());

    // Serialize to JSON
    json j = emptyShape;

    // Verify JSON structure
    BOOST_CHECK(j.contains("TopoDS_Shape"));
    BOOST_CHECK(j["TopoDS_Shape"].contains("format"));
    BOOST_CHECK_EQUAL(j["TopoDS_Shape"]["format"], "BREP");
    BOOST_CHECK(j["TopoDS_Shape"].contains("data"));

    // Deserialize from JSON
    TopoDS_Shape deserializedShape = j;

    // Verify the shape is still null (empty)
    BOOST_CHECK(deserializedShape.IsNull());
}