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
find_package(Boost REQUIRED COMPONENTS signals2)
find_package(nfd REQUIRED)
find_package(spdlog REQUIRED)

# Add executable target
add_executable(OcctImgui
    src/model/IModel.cpp
    src/model/UnifiedModel.cpp
    src/model/ModelFactory.cpp
    src/model/ModelManager.cpp
    src/view/ImGuiView.cpp
    src/view/OcctView.cpp
    src/viewmodel/UnifiedViewModel.cpp
    src/viewmodel/ViewModelManager.cpp
    src/Application.cpp
    src/GlfwOcctWindow.cpp
    src/utils/Logger.cpp
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
    spdlog::spdlog
    nfd::nfd
    Boost::signals2
)

target_compile_options(OcctImgui PRIVATE
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Release>:-O3>
)