cmake_minimum_required(VERSION 3.15)

if(NOT DEFINED ENV{VCPKG_ROOT})
    set(VCPKG_ROOT "D:/vcpkg") # 默认路径
else()
    set(VCPKG_ROOT $ENV{VCPKG_ROOT})
endif()

# 使用环境变量设置工具链文件
message(STATUS "VCPKG_ROOT: $ENV{VCPKG_ROOT}")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

# Project configuration
project(OcctImgui LANGUAGES CXX)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})

# Include directories
find_package(nfd CONFIG REQUIRED)
find_package(imgui REQUIRED)
find_package(OpenCASCADE 7.8.1 REQUIRED)
find_package(Netgen CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# Add executable target
add_executable(OcctImgui
    src/model/ModelManager.cpp
    src/GlfwOcctView.cpp
    src/GlfwOcctWindow.cpp
    src/gui/ModelTreeGui.cpp
    src/gui/ModelControlGui.cpp
    src/ais/OCCMesh_DataSource.cpp
    src/importers/StepImporter.cpp
    src/importers/MeshImporter.cpp
    src/main.cpp)

target_include_directories(OcctImgui
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(OcctImgui
    PRIVATE
    ${OpenCASCADE_LIBRARIES}
    nglib
    ngcore
    imgui::imgui
    nfd::nfd
    spdlog::spdlog
)