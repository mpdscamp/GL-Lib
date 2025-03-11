# Cube Decal

A 3D graphics application demonstrating homography projection from a 3D cube to a 2D plane, built with modern C++ and OpenGL.

## Features

- Real-time 3D rendering with OpenGL 4.6
- Component-based architecture for modular, reusable code
- Dynamic homography calculation to project textures from cube faces to a 2D plane
- Interactive camera controls with mouse and keyboard navigation
- Shader hot-reloading for rapid development
- Comprehensive logging system with multiple severity levels
- Configurable rendering options (auto-rotation, speed, etc.)

## Requirements

- C++17 compatible compiler (MSVC, GCC, Clang)
- CMake 3.15+
- OpenGL 4.6 compatible GPU
- GLFW3
- GLM (OpenGL Mathematics)
- stb_image (included)

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Controls

- **WASD**: Move camera position
- **Q/E**: Move camera up/down
- **Mouse**: Look around
- **R**: Toggle cube auto-rotation
- **F5**: Reload shaders
- **Arrow Keys**: Manual cube rotation (when auto-rotation is off)
- **ESC**: Exit application

## Implementation Details

The project demonstrates homography projection, which maps points from one plane to another. In this case, it maps textures from the cube's faces onto a 2D quad in real-time as the cube rotates.

Key technical components:

- **Entity-Component System**: Modular design where entities contain components defining behavior
- **Homography Effect**: Computes and applies a homography matrix to transform texture coordinates
- **OpenGL Abstraction Layer**: Clean C++ wrappers around raw OpenGL calls
- **Resource Management**: Efficient handling of textures, shaders, and other assets

---

*Created as part of the Computer Graphics course at Instituto Militar de Engenharia (IME), 2025*
