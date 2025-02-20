# Changelog

## Current Changes

This fork includes several modifications from the original repository:

1. Updated build configurations and dependencies
2. Added support for STEP model meshing functionality
3. Integrated Netgen meshing library
4. Modified CMake configuration to use newer versions of dependencies
5. Added example STEP model files in assets/examples/
6. Enhanced integration between OpenCASCADE and ImGui for mesh visualization
7. Removed Hello ImGui dependency in favor of direct GLFW + ImGui integration
8. Added STEP file loading dialog using NFD (Native File Dialog)
9. Improved shape management using std::vector container

The main goal of these changes is to demonstrate STEP model meshing capabilities using OpenCASCADE and Netgen, with an interactive ImGui-based interface for visualization and control.
