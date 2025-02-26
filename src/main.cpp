#include "window/Window.hpp"
#include "scenes/Scene.hpp"

#include <glad/glad.h>

#include <iostream>
#include <stdexcept>
#include <Windows.h>

// GLFW error callback
void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

int main() {
    try {
        // Create a console window and redirect stdout
        if (AllocConsole()) {
            FILE* pCout;
            freopen_s(&pCout, "CONOUT$", "w", stdout);
            std::cout.clear();

        }

        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        // RAII for GLFW termination
        struct GLFWTerminator {
            ~GLFWTerminator() {
                glfwTerminate();
            }
        } glfwTerminator;

        // Set GLFW context hints
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        // Force focus on window
        glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

        // Create a window using the Window class
        Window window(800, 600, "PROJECT I");

        // Initialize GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        // Get OpenGL info
        const GLubyte* renderer = glGetString(GL_RENDERER);
        const GLubyte* version = glGetString(GL_VERSION);

        // Configure OpenGL state
        glViewport(0, 0, 800, 600);
        gl::enable(gl::Capability::DepthTest);
        gl::enable(gl::Capability::Blend);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Force cursor capture after window creation but before scene setup
        window.captureCursor();
        glfwFocusWindow(window.getGLFWWindow());

        // Create and initialize the scene with our window
        Scene scene(window);

        // For timing
        float lastFrame = 0.0f;

        // Render loop
        while (!window.shouldClose()) {
            // Calculate delta time
            float currentFrame = static_cast<float>(glfwGetTime());
            float deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // Poll events first
            window.pollEvents();

            // Process window input (updates key states)
            window.processInput(deltaTime);

            // Periodically check if cursor is captured, recapture if necessary
            if (glfwGetInputMode(window.getGLFWWindow(), GLFW_CURSOR) != GLFW_CURSOR_DISABLED &&
                glfwGetWindowAttrib(window.getGLFWWindow(), GLFW_FOCUSED)) {
                window.captureCursor();
            }

            // Then process scene input (uses key states)
            scene.update(deltaTime);

            // Clear the screen
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Render scene
            scene.render();

            // Swap buffers
            window.swapBuffers();
        }

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
}