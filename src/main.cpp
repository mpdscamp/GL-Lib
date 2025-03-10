#include "window/Window.hpp"
#include "scenes/Scene.hpp"
#include "managers/ResourceManager.hpp"
#include "profiling/Profiler.hpp"
#include "gl/logger.hpp" // Add logger header

#include <glad/glad.h>

#include <stdexcept>
#include <Windows.h>

// GLFW error callback
void glfw_error_callback(int error, const char* description) {
    gl::logError("GLFW Error " + std::to_string(error) + ": " + description);
}

int main() {
    try {
        // Create a console window and redirect stdout
        if (AllocConsole()) {
            FILE* pCout;
            freopen_s(&pCout, "CONOUT$", "w", stdout);
            std::cout.clear();
        }

        // Initialize logger
        gl::setLogLevel(gl::LogLevel::Info);
        gl::setLogFile("application.log");
        gl::logInfo("Application starting");

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
        // Enable debug context in debug builds
#ifdef _DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
        // Force focus on window
        glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

        // Create window and resource manager
        Window window(800, 600, "PROJECT I");
        ResourceManager resourceManager;

        // Initialize GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        // Enable vsync
        glfwSwapInterval(1);

        // Set up debug callback in debug builds
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

            // Message verbosity
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
        }
#endif

        // Get OpenGL info
        const GLubyte* renderer = glGetString(GL_RENDERER);
        const GLubyte* version = glGetString(GL_VERSION);
        gl::logInfo("Renderer: " + std::string(reinterpret_cast<const char*>(renderer)));
        gl::logInfo("OpenGL version: " + std::string(reinterpret_cast<const char*>(version)));

        // Configure OpenGL state
        glViewport(0, 0, 800, 600);
        gl::enable(gl::Capability::DepthTest);
        gl::enable(gl::Capability::Blend);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Force cursor capture after window creation but before scene setup
        window.captureCursor();
        glfwFocusWindow(window.getGLFWWindow());

        // Create and initialize the scene
        Scene scene(window, resourceManager);
        gl::logInfo("Scene initialized");

        // Setup profiler
        Profiler profiler;

        // Timing variables for FPS counter
        float lastFrame = 0.0f;
        float lastFPSUpdate = 0.0f;
        int frameCount = 0;
        float fps = 0.0f;

        // Shader hot-reload key state
        bool f5Pressed = false;

        gl::logInfo("Entering main loop");

        // Render loop
        while (!window.shouldClose()) {
            profiler.beginFrame();

            // Calculate delta time
            float currentFrame = static_cast<float>(glfwGetTime());
            float deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // Calculate FPS
            frameCount++;
            if (currentFrame - lastFPSUpdate >= 0.5f) { // Update every half second
                fps = frameCount / (currentFrame - lastFPSUpdate);
                frameCount = 0;
                lastFPSUpdate = currentFrame;

                // Update window title with FPS
                std::string title = "PROJECT I | FPS: " + std::to_string(static_cast<int>(fps));
                glfwSetWindowTitle(window.getGLFWWindow(), title.c_str());

                // Log FPS at debug level
                gl::logDebug("FPS: " + std::to_string(static_cast<int>(fps)));
            }

            // Poll events
            profiler.beginSection("Input");
            window.pollEvents();
            window.processInput(deltaTime);

            // Check for shader hot-reload key (F5)
            if (glfwGetKey(window.getGLFWWindow(), GLFW_KEY_F5) == GLFW_PRESS) {
                if (!f5Pressed) {
                    f5Pressed = true;
                    resourceManager.reloadShaders();
                    gl::logInfo("Shaders reloaded");
                }
            }
            else {
                f5Pressed = false;
            }

            // Periodically check if cursor is captured, recapture if necessary
            if (glfwGetInputMode(window.getGLFWWindow(), GLFW_CURSOR) != GLFW_CURSOR_DISABLED &&
                glfwGetWindowAttrib(window.getGLFWWindow(), GLFW_FOCUSED)) {
                window.captureCursor();
            }
            profiler.endSection("Input");

            // Update scene
            profiler.beginSection("Update");
            scene.update(deltaTime);
            profiler.endSection("Update");

            // Render
            profiler.beginSection("Render");
            // Clear the screen
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Render scene
            scene.render();
            profiler.endSection("Render");

            // Swap buffers
            profiler.beginSection("SwapBuffers");
            window.swapBuffers();
            profiler.endSection("SwapBuffers");

            profiler.endFrame();

            // Print profiling stats periodically
            static double lastStatTime = 0.0;
            if (currentFrame - lastStatTime > 5.0) {
                profiler.printStats();
                lastStatTime = currentFrame;
            }
        }

        gl::logInfo("Application shutting down normally");
        return 0;
    }
    catch (const std::exception& e) {
        gl::logError("FATAL ERROR: " + std::string(e.what()));
        return -1;
    }
}