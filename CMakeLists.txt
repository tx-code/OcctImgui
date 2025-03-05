cmake_minimum_required(VERSION 3.15)

# Project configuration
project(OcctImgui LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})

# Include directories
find_package(imgui REQUIRED)
find_package(OpenCASCADE REQUIRED)
find_package(Netgen CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS signals2 unit_test_framework)
find_package(nfd REQUIRED)
find_package(spdlog REQUIRED)
option(LIBIGL_USE_STATIC_LIBRARY "Use libIGL as static library" ON)
find_package(libigl REQUIRED)
find_package(Eigen3 REQUIRED)

# Enable testing with CTest
include(CTest)
enable_testing()

# Define test data directory
set(MESH_TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests/data")
file(MAKE_DIRECTORY ${MESH_TEST_DATA_DIR})

# Create a library for shared components that will be used in both the main app and tests
add_library(OcctImguiLib STATIC
    src/ais/Mesh_DataSource.cpp
    src/model/IModel.cpp
    src/model/UnifiedModel.cpp
    src/model/ModelFactory.cpp
    src/model/ModelManager.cpp
    src/model/ModelImporter.cpp
    src/view/ImGuiView.cpp
    src/view/OcctView.cpp
    src/viewmodel/UnifiedViewModel.cpp
    src/viewmodel/ViewModelManager.cpp
    src/Application.cpp
    src/GlfwOcctWindow.cpp
    src/utils/Logger.cpp
)

target_include_directories(OcctImguiLib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(OcctImguiLib
    PUBLIC
    ${OpenCASCADE_LIBRARIES}
    igl::igl_core
    Eigen3::Eigen
    nglib
    ngcore
    imgui::imgui
    spdlog::spdlog
    nfd::nfd
    igl_copyleft::igl_copyleft_cgal
    Boost::signals2
)

# Add executable target
add_executable(OcctImgui
    src/main.cpp
)

target_include_directories(OcctImgui
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(OcctImgui
    PRIVATE
    OcctImguiLib
)

target_compile_options(OcctImgui PRIVATE
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Release>:-O3>
)

option(BUILD_TESTING "Build tests" OFF)

# Tests configuration
if(BUILD_TESTING)
    # Create a function to add tests easily
    function(add_boost_test TEST_NAME TEST_SOURCE)
        add_executable(${TEST_NAME} ${TEST_SOURCE})
        target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
        target_link_libraries(${TEST_NAME} 
            PRIVATE 
            OcctImguiLib
            Boost::unit_test_framework
            # Add other dependencies needed by your tests
        )
        # Define test target
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
        # Tell Boost.Test to use dynamic linking
        target_compile_definitions(${TEST_NAME} PRIVATE 
            BOOST_TEST_DYN_LINK
            MESH_TEST_DATA_DIR="${MESH_TEST_DATA_DIR}"
        )
    endfunction()

    # Add the tests
    add_boost_test(model_test tests/model_test.cpp)
    add_boost_test(model_manager_test tests/model_manager_test.cpp)
    add_boost_test(mesh_datasource_test tests/mesh_datasource_test.cpp)
    add_boost_test(unified_model_test tests/unified_model_test.cpp)
    add_boost_test(model_importer_test tests/model_importer_test.cpp)
endif()