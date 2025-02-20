# OcctImgui

OpenCASCADE + GLFW + IMGUI Sample.

## Changes

This fork includes several modifications from the original repository:

1. Updated build configurations and dependencies
2. Added support for STEP model meshing functionality
3. Integrated Netgen meshing library
4. Modified CMake configuration to use newer versions of dependencies
5. Enhanced integration between OpenCASCADE and ImGui for mesh visualization
6. Removed Hello ImGui dependency in favor of direct GLFW + ImGui integration

The main goal of these changes is to demonstrate STEP model meshing capabilities using OpenCASCADE and Netgen, with an interactive ImGui-based interface for visualization and control.

![occt imgui](occt-imgui.png "opencascade imgui")

<https://tracker.dev.opencascade.org/view.php?id=33485>

## OpenCASCADE

  <https://dev.opencascade.org/>
  
  <https://github.com/Open-Cascade-SAS/OCCT>

  Open CASCADE Technology (OCCT) a software
development platform providing services for 3D surface and solid modeling, CAD
data exchange, and visualization. Most of OCCT functionality is available in
the form of C++ libraries. OCCT can be best applied in development of software
dealing with 3D modeling (CAD), manufacturing / measuring (CAM) or numerical
simulation (CAE).
  
## IMGUI

  <https://github.com/ocornut/imgui>

  Dear ImGui is a bloat-free graphical user interface library for C++. It outputs optimized vertex buffers that you can render anytime in your 3D-pipeline-enabled application. It is fast, portable, renderer agnostic, and self-contained (no external dependencies).

Dear ImGui is designed to enable fast iterations and to empower programmers to create content creation tools and visualization / debug tools (as opposed to UI for the average end-user). It favors simplicity and productivity toward this goal and lacks certain features commonly found in more high-level libraries.

Dear ImGui is particularly suited to integration in game engines (for tooling), real-time 3D applications, fullscreen applications, embedded applications, or any applications on console platforms where operating system features are non-standard.

## GLFW

  <https://github.com/glfw/glfw>

  GLFW is an Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan application development. It provides a simple, platform-independent API for creating windows, contexts and surfaces, reading input, handling events, etc.

GLFW natively supports Windows, macOS and Linux and other Unix-like systems. On Linux both X11 and Wayland are supported.

GLFW is licensed under the zlib/libpng license.

## Build

Dependencies:

- OpenCASCADE (>= v7.8.1)
- GLFW
- ImGui
- NFD (Native File Dialog)
- Netgen

Use VCPKG to manage 3rd-party libraries (OpenCASCADE, netgen...)

```bash
cmake -DCMAKE_CXX_STANDARD=17 ..
```
