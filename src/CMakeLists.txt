add_executable(OpenGL
    "main.cpp"
    "window/Window.cpp"
    "window/Camera.cpp"
    "core/Scene.cpp"
    "core/Entity.cpp"
    "managers/ResourceManager.cpp"
    "profiling/Profiler.cpp"
    
    # Component files
    "components/camera/CameraComponent.cpp"
    "components/geometry/MeshComponent.cpp"
    "components/rendering/MeshRenderer.cpp"
    
    "components/rendering/PostProcessor.cpp"
    "components/effects/HomographyEffect.cpp"
    "components/input/InputHandler.cpp"
    
    # Third-party libraries
    "../include/libs/glad/src/glad.c"
)

# Ensure GLAD, GLM, stb_image, and other headers are found by the compiler:
target_include_directories(OpenGL PRIVATE
    ../include/libs/glad/include
    ../include/libs/glm
    ../include/libs
    ../include
)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Link the executable against:
#  - GLFW
#  - OpenGL (system library)
target_link_libraries(OpenGL PRIVATE glfw OpenGL::GL)

if(MSVC)
    # Disable warnings: C26819 (fallthrough) and C6262 (stack usage)
    target_compile_options(OpenGL PRIVATE /wd26819 /wd6262)
    # Add secure CRT define to avoid warnings about ctime
    target_compile_definitions(OpenGL PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Custom command to copy resource files to build directory after build
add_custom_command(TARGET OpenGL POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/../resources
        ${CMAKE_CURRENT_BINARY_DIR}/resources
    COMMENT "Copying resources to build directory..."
)