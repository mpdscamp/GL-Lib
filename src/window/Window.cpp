#include "Window.hpp"
#include "../core/Scene.hpp"
#include "../gl/logger.hpp"

#include <glad/glad.h>
#include <stdexcept>
#include <algorithm>

// Initialize static member
std::unordered_map<GLFWwindow*, Window*> Window::windowInstances;

// Static callback methods
void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto it = windowInstances.find(window);
    if (it != windowInstances.end() && key >= 0 && key < 1024) {
        Window* windowInstance = it->second;

        if (action == GLFW_PRESS) {
            windowInstance->keys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            windowInstance->keys[key] = false;
        }
    }
}

void Window::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    auto it = windowInstances.find(window);
    if (it != windowInstances.end()) {
        Window* windowInstance = it->second;

        if (windowInstance->firstMouse) {
            windowInstance->lastX = xpos;
            windowInstance->lastY = ypos;
            windowInstance->firstMouse = false;
            return; // Skip first frame to avoid jumps
        }

        double xoffset = xpos - windowInstance->lastX;
        double yoffset = windowInstance->lastY - ypos; // Reversed: y ranges bottom to top

        windowInstance->lastX = xpos;
        windowInstance->lastY = ypos;

        if (windowInstance->mouseCallback_) {
            windowInstance->mouseCallback_(xoffset, yoffset);
        }
    }
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto it = windowInstances.find(window);
    if (it != windowInstances.end()) {
        Window* windowInstance = it->second;
        windowInstance->width_ = width;
        windowInstance->height_ = height;

        // Call all registered resize callbacks
        for (const auto& callback : windowInstance->resizeCallbacks_) {
            callback(width, height);
        }

        // Notify all scenes of resize
        for (Scene* scene : windowInstance->scenes_) {
            if (scene) {
                scene->onWindowResize(width, height);
            }
        }

        gl::logDebug("Window resized to " + std::to_string(width) + "x" + std::to_string(height));
    }

    // Update viewport
    glViewport(0, 0, width, height);
}

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        auto it = windowInstances.find(window);
        if (it != windowInstances.end()) {
            Window* windowInstance = it->second;
            windowInstance->captureCursor();
        }
    }
}

void Window::windowFocusCallback(GLFWwindow* window, int focused) {
    if (focused) {
        auto it = windowInstances.find(window);
        if (it != windowInstances.end()) {
            Window* windowInstance = it->second;
            windowInstance->captureCursor();
        }
    }
}

Window::Window(int width, int height, const std::string& title)
    : width_(width), height_(height), lastX(width / 2.0), lastY(height / 2.0)
{
    // Create the window
    window_ = glfwCreateWindow(width_, height_, title.c_str(), nullptr, nullptr);
    if (!window_) {
        throw std::runtime_error("Failed to create GLFW window");
    }

    // Store this instance for callbacks
    windowInstances[window_] = this;

    // Make context current
    glfwMakeContextCurrent(window_);

    // Set callbacks
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
    glfwSetCursorPosCallback(window_, mouseCallback);
    glfwSetKeyCallback(window_, keyCallback);
    glfwSetMouseButtonCallback(window_, mouseButtonCallback);
    glfwSetWindowFocusCallback(window_, windowFocusCallback);

    // Initialize keys array to all false
    keys.fill(false);
}

Window::~Window() {
    if (window_) {
        // Clean up scenes references to prevent dangling pointers
        scenes_.clear();

        // Remove from static map
        windowInstances.erase(window_);

        // Destroy the window
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
}

Window::Window(Window&& other) noexcept
    : window_(other.window_), width_(other.width_), height_(other.height_),
    lastX(other.lastX), lastY(other.lastY), firstMouse(other.firstMouse),
    mouseCallback_(std::move(other.mouseCallback_)),
    resizeCallbacks_(std::move(other.resizeCallbacks_)),
    scenes_(std::move(other.scenes_))
{
    // Update the static map
    windowInstances[window_] = this;

    // Move the keys array
    keys = std::move(other.keys);

    // Reset the other window pointer
    other.window_ = nullptr;
}

Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
        // Clean up this instance
        if (window_) {
            windowInstances.erase(window_);
            glfwDestroyWindow(window_);
        }

        // Move data from other
        window_ = other.window_;
        width_ = other.width_;
        height_ = other.height_;
        lastX = other.lastX;
        lastY = other.lastY;
        firstMouse = other.firstMouse;
        mouseCallback_ = std::move(other.mouseCallback_);
        resizeCallbacks_ = std::move(other.resizeCallbacks_);
        scenes_ = std::move(other.scenes_);
        keys = std::move(other.keys);

        // Update the static map
        windowInstances[window_] = this;

        // Reset the other window pointer
        other.window_ = nullptr;
    }
    return *this;
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window_);
}

void Window::swapBuffers() const {
    glfwSwapBuffers(window_);
}

void Window::pollEvents() const {
    glfwPollEvents();
}

GLFWwindow* Window::getGLFWWindow() const {
    return window_;
}

void Window::setMouseCallback(std::function<void(double, double)> callback) {
    mouseCallback_ = callback;
}

void Window::registerResizeCallback(std::function<void(int, int)> callback) {
    resizeCallbacks_.push_back(callback);
}

void Window::captureCursor() {
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    firstMouse = true;
    gl::logDebug("Cursor captured");
}

void Window::processInput(float deltaTime) {
    // Check for escape key to close application
    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window_, true);
    }

    // Use TAB to toggle cursor capture/release
    static bool tabPressed = false;
    bool tabCurrentlyPressed = glfwGetKey(window_, GLFW_KEY_TAB) == GLFW_PRESS;

    if (tabCurrentlyPressed) {
        if (!tabPressed) {
            tabPressed = true;
            int cursorMode = glfwGetInputMode(window_, GLFW_CURSOR);
            if (cursorMode == GLFW_CURSOR_DISABLED) {
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                gl::logDebug("Cursor released");
            }
            else {
                captureCursor();
            }
        }
    }
    else {
        tabPressed = false;
    }

    // Only update the keys we actually use
    const int monitored_keys[] = {
        GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,
        GLFW_KEY_Q, GLFW_KEY_E, GLFW_KEY_R, GLFW_KEY_F5,
        GLFW_KEY_P, GLFW_KEY_LEFT_BRACKET, GLFW_KEY_RIGHT_BRACKET,
        GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_UP, GLFW_KEY_DOWN,
        GLFW_KEY_SPACE, GLFW_KEY_TAB
    };

    for (int key : monitored_keys) {
        keys[key] = glfwGetKey(window_, key) == GLFW_PRESS;
    }
}

bool Window::isKeyPressed(int key) const {
    if (key < 0 || key >= 1024) return false;
    return keys[key];
}

bool Window::isKeyHeld(int key) const {
    if (key < 0 || key >= 1024) return false;
    return keys[key];
}

void Window::addScene(Scene* scene) {
    if (scene) {
        // Only add if not already in the list
        if (std::find(scenes_.begin(), scenes_.end(), scene) == scenes_.end()) {
            scenes_.push_back(scene);
            gl::logDebug("Scene added to window");
        }
    }
}

void Window::removeScene(Scene* scene) {
    if (scene) {
        auto it = std::find(scenes_.begin(), scenes_.end(), scene);
        if (it != scenes_.end()) {
            scenes_.erase(it);
            gl::logDebug("Scene removed from window");
        }
    }
}