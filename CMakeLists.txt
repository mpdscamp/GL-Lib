cmake_minimum_required(VERSION 3.10)
project(OpenGL)

# Use C++20 for everything (adjust as you like)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build GLFW from source
add_subdirectory(include/libs/glfw)

# Build your actual application code
add_subdirectory(src)