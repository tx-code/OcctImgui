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
find_package(Boost REQUIRED COMPONENTS signals2 unit_test_framework uuid)
find_package(nfd REQUIRED)
find_package(spdlog REQUIRED)
option(LIBIGL_USE_STATIC_LIBRARY "Use libIGL as static library" ON)
find_package(libigl REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(nlohmann_json REQUIRED)

# Enable testing with CTest
include(CTest)
enable_testing()

# Define test data directory
set(TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests/data")
file(MAKE_DIRECTORY ${TEST_DATA_DIR})

# Core Library
add_library(OcctImguiCore STATIC
    src/core/SelectionManager.cpp
    src/utils/Logger.cpp
    src/utils/LoggerManager.cpp
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(OcctImguiCore PUBLIC OCCTIMGUI_DEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_definitions(OcctImguiCore PUBLIC OCCTIMGUI_RELWITHDEBINFO)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(OcctImguiCore PUBLIC OCCTIMGUI_RELEASE)
endif()

target_include_directories(OcctImguiCore
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src # Only keep the src directory in the include path
)

target_link_libraries(OcctImguiCore
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
    Boost::uuid
    nlohmann_json::nlohmann_json
)

add_library(OcctImguiModel STATIC
    src/model/MeshData.cpp
    src/model/GeometryModel.cpp

    # src/model/GeometryModel.cpp
)

target_link_libraries(OcctImguiModel
    PUBLIC
    OcctImguiCore
)

#[[
# Add executable target
add_executable(OcctImgui
    src/main.cpp
)

target_include_directories(OcctImgui
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(OcctImgui
    PRIVATE
    OcctImguiCore
)
target_compile_options(OcctImgui PRIVATE
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Release>:-O3>
)
#]]
option(OCCTIMGUI_BUILD_TESTING "Build tests" ON)

# Tests configuration
if(OCCTIMGUI_BUILD_TESTING)
    # Create a function to add tests easily
    function(add_boost_test TEST_NAME TEST_SOURCE)
        add_executable(${TEST_NAME} ${TEST_SOURCE})
        target_include_directories(${TEST_NAME}
            PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
        )
        target_link_libraries(${TEST_NAME}
            PRIVATE
            OcctImguiCore
            OcctImguiModel
            Boost::unit_test_framework

            # Add other dependencies needed by your tests
        )

        # Define test target
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

        # Tell Boost.Test to use dynamic linking
        target_compile_definitions(${TEST_NAME} PRIVATE
            BOOST_TEST_DYN_LINK
            TEST_DATA_DIR="${TEST_DATA_DIR}"
        )
    endfunction()

    # Add the tests
    add_boost_test(core_signal_test tests/core/signal_test.cpp)
    add_boost_test(core_message_bus_test tests/core/message_bus_test.cpp)
    add_boost_test(core_property_test tests/core/property_test.cpp)
    add_boost_test(core_selection_manager_test tests/core/selection_manager_test.cpp)
    add_boost_test(core_integration_test tests/core/integration_test.cpp)
    add_boost_test(core_serializable_object_test tests/core/serializable_object_test.cpp)
endif()