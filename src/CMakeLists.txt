add_executable(OpenGL
    "main.cpp"
    "window/Window.cpp"
    "scenes/Scene.cpp"
    "managers/ResourceManager.cpp"
    "profiling/Profiler.cpp"
    
    ../include/libs/glad/src/glad.c
 "window/Camera.cpp" "profiling/Profiler.cpp")

# Ensure GLAD, GLM, stb_image, and other headers are found by your compiler:
target_include_directories(OpenGL PRIVATE
    ../include/libs/glad/include
    ../include/libs/glm
    ../include/libs
    ../include
)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Link your executable against:
#  - GLFW (the target from 'libs/glfw')
#  - OpenGL (system library)
target_link_libraries(OpenGL PRIVATE glfw OpenGL::GL)

if(MSVC)
    # Disable warnings: C26819 (fallthrough) and C6262 (stack usage)
    target_compile_options(OpenGL PRIVATE /wd26819 /wd6262)
endif()

# Custom command to copy resource files to build directory after build
add_custom_command(TARGET OpenGL POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/../resources
        ${CMAKE_CURRENT_BINARY_DIR}/resources
    COMMENT "Copying resources to build directory..."
)
