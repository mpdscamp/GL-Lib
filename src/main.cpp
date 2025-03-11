#include "window/Window.hpp"
#include "core/Scene.hpp"
#include "managers/ResourceManager.hpp"
#include "profiling/Profiler.hpp"
#include "gl/logger.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <string>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

// GLFW error callback
void glfw_error_callback(int error, const char* description) {
    gl::logError("GLFW Error " + std::to_string(error) + ": " + description);
}

int main() {
    try {
        // Create console for output on Windows
#ifdef _WIN32
        if (AllocConsole()) {
            FILE* pCout;
            freopen_s(&pCout, "CONOUT$", "w", stdout);
            std::cout.clear();
        }
#endif

        // Initialize logger
        gl::setLogLevel(gl::LogLevel::Info);
        gl::setLogFile("application.log");
        gl::logInfo("Application starting");

        // Initialize GLFW
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        // RAII for GLFW termination
        struct GLFWTerminator {
            ~GLFWTerminator() {
                glfwTerminate();
                gl::logInfo("GLFW terminated");
            }
        } glfwTerminator;

        // Set GLFW window hints
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        // Enable debug context in debug builds
#ifdef _DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

        // Create window
        Window window(800, 600, "3D Graphics Demo");
        gl::logInfo("Window created: 800x600");

        // Initialize GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw std::runtime_error("Failed to initialize GLAD");
        }
        gl::logInfo("GLAD initialized");

        // Enable VSync
        glfwSwapInterval(1);

        // Setup OpenGL debug callback in debug builds
#ifdef _DEBUG
        GLint flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(
                [](GLenum source, GLenum type, GLuint id, GLenum severity,
                    GLsizei length, const GLchar* message, const void* userParam) {
                        // Filter out non-significant error/warning codes
                        if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

                        std::string msg = "Debug message (" + std::to_string(id) + "): " + message;

                        switch (severity) {
                        case GL_DEBUG_SEVERITY_HIGH:
                            gl::logError("GL ERROR: " + std::string(message));
                            break;
                        case GL_DEBUG_SEVERITY_MEDIUM:
                            gl::logWarning("GL WARNING: " + std::string(message));
                            break;
                        case GL_DEBUG_SEVERITY_LOW:
                            gl::logInfo(msg);
                            break;
                        case GL_DEBUG_SEVERITY_NOTIFICATION:
                            gl::logDebug(msg);
                            break;
                        }
                }, nullptr);

            // Reduce message verbosity
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
            gl::logDebug("OpenGL debug output enabled");
        }
#endif

        // Log OpenGL information
        const GLubyte* renderer = glGetString(GL_RENDERER);
        const GLubyte* version = glGetString(GL_VERSION);
        gl::logInfo("Renderer: " + std::string(reinterpret_cast<const char*>(renderer)));
        gl::logInfo("OpenGL version: " + std::string(reinterpret_cast<const char*>(version)));

        // Configure OpenGL state
        glViewport(0, 0, 800, 600);
        gl::enable(gl::Capability::DepthTest);
        gl::enable(gl::Capability::Blend);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        gl::logDebug("OpenGL state configured");

        // Create resource manager
        ResourceManager resourceManager;
        gl::logInfo("Resource manager created");

        // Capture cursor for camera control
        window.captureCursor();
        gl::logDebug("Cursor captured");

        // Create and initialize scene
        Scene scene(window, resourceManager);
        scene.init();
        gl::logInfo("Scene initialized");

        // Create profiler
        Profiler profiler;
        gl::logDebug("Profiler created");

        // Timing variables
        float lastFrame = 0.0f;
        float lastFPSUpdate = 0.0f;
        float lastProfilerUpdate = 0.0f;
        int frameCount = 0;

        gl::logInfo("Entering main loop");

        // Main render loop
        while (!window.shouldClose()) {
            // Start frame profiling
            profiler.beginFrame();

            // Calculate delta time
            float currentTime = static_cast<float>(glfwGetTime());
            float deltaTime = currentTime - lastFrame;
            lastFrame = currentTime;

            // Calculate FPS
            frameCount++;
            if (currentTime - lastFPSUpdate >= 0.5f) {
                float fps = frameCount / (currentTime - lastFPSUpdate);
                frameCount = 0;
                lastFPSUpdate = currentTime;

                // Update window title with FPS
                std::string title = "3D Graphics Demo | FPS: " + std::to_string(static_cast<int>(fps));
                glfwSetWindowTitle(window.getGLFWWindow(), title.c_str());
                gl::logDebug("FPS: " + std::to_string(static_cast<int>(fps)));
            }

            // Process input
            profiler.beginSection("Input");
            window.pollEvents();
            profiler.endSection("Input");

            // Update scene
            profiler.beginSection("Update");
            scene.update(deltaTime);
            profiler.endSection("Update");

            // Render scene
            profiler.beginSection("Render");
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            scene.render();
            profiler.endSection("Render");

            // Swap buffers
            profiler.beginSection("SwapBuffers");
            window.swapBuffers();
            profiler.endSection("SwapBuffers");

            // End frame profiling
            profiler.endFrame();

            // Print profiler stats periodically
            if (currentTime - lastProfilerUpdate >= 5.0f) {
                profiler.printStats();
                lastProfilerUpdate = currentTime;
            }

            // Check if ESC was pressed to exit
            if (glfwGetKey(window.getGLFWWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                window.setShouldClose(true);
            }

            // Periodically check if cursor is captured, recapture if necessary
            if (glfwGetInputMode(window.getGLFWWindow(), GLFW_CURSOR) != GLFW_CURSOR_DISABLED &&
                glfwGetWindowAttrib(window.getGLFWWindow(), GLFW_FOCUSED)) {
                window.captureCursor();
            }
        }

        gl::logInfo("Main loop exited");
        gl::logInfo("Application shutting down normally");
        return 0;
    }
    catch (const std::exception& e) {
        gl::logError("FATAL ERROR: " + std::string(e.what()));
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return -1;
    }
}