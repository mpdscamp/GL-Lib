#include "Window.hpp"

#include <glad/glad.h>
#include <stdexcept>
#include <iostream>
#include <unordered_map>

// Map to store Window instances associated with each GLFWwindow
static std::unordered_map<GLFWwindow*, Window*> windowInstances;

// Static callback functions that delegate to the appropriate Window instance
static void mouseCallbackWrapper(GLFWwindow* window, double xpos, double ypos) {
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
            // Debug output
            // std::cout << "Mouse moved: " << xoffset << ", " << yoffset << std::endl;
            windowInstance->mouseCallback_(xoffset, yoffset);
        }
    }
}

static void keyCallbackWrapper(GLFWwindow* window, int key, int scancode, int action, int mode) {
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

static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);

    // Update window size if needed
    auto it = windowInstances.find(window);
    if (it != windowInstances.end()) {
        Window* windowInstance = it->second;
        windowInstance->width_ = width;
        windowInstance->height_ = height;
    }
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        auto it = windowInstances.find(window);
        if (it != windowInstances.end()) {
            Window* windowInstance = it->second;
            windowInstance->captureCursor();
        }
    }
}

static void windowFocusCallback(GLFWwindow* window, int focused) {
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
    window_ = glfwCreateWindow(width_, height_, title.c_str(), nullptr, nullptr);
    if (!window_) {
        throw std::runtime_error("Failed to create GLFW window");
    }

    // Store this instance for callbacks
    windowInstances[window_] = this;

    glfwMakeContextCurrent(window_);

    // Set callbacks directly here - this is important!
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
    glfwSetCursorPosCallback(window_, mouseCallbackWrapper);
    glfwSetKeyCallback(window_, keyCallbackWrapper);
    glfwSetMouseButtonCallback(window_, mouseButtonCallback);
    glfwSetWindowFocusCallback(window_, windowFocusCallback);

    // Initialize keys array
    for (int i = 0; i < 1024; i++) {
        keys[i] = false;
    }
}

Window::~Window() {
    if (window_) {
        windowInstances.erase(window_);
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
}

Window::Window(Window&& other) noexcept
    : window_(other.window_), width_(other.width_), height_(other.height_),
    lastX(other.lastX), lastY(other.lastY), firstMouse(other.firstMouse),
    mouseCallback_(std::move(other.mouseCallback_))
{
    windowInstances[window_] = this;
    for (int i = 0; i < 1024; i++) {
        keys[i] = other.keys[i];
    }
    other.window_ = nullptr;
}

Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
        if (window_) {
            windowInstances.erase(window_);
            glfwDestroyWindow(window_);
        }
        window_ = other.window_;
        width_ = other.width_;
        height_ = other.height_;
        lastX = other.lastX;
        lastY = other.lastY;
        firstMouse = other.firstMouse;
        mouseCallback_ = std::move(other.mouseCallback_);

        for (int i = 0; i < 1024; i++) {
            keys[i] = other.keys[i];
        }

        windowInstances[window_] = this;
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

void Window::setFramebufferSizeCallback(GLFWframebuffersizefun callback) {
    glfwSetFramebufferSizeCallback(window_, callback);
}

void Window::setMouseCallback(std::function<void(double, double)> callback) {
    mouseCallback_ = callback;
}

void Window::setCursorMode(int mode) {
    glfwSetInputMode(window_, GLFW_CURSOR, mode);
}

void Window::captureCursor() {
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    firstMouse = true;
}

void Window::processInput(float deltaTime) {
    // Check for escape key to close window
    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window_, true);
    }

    // Manual toggle for cursor capture using TAB key
    static bool tabPressed = false;
    if (glfwGetKey(window_, GLFW_KEY_TAB) == GLFW_PRESS) {
        if (!tabPressed) {
            tabPressed = true;
            int currentMode = glfwGetInputMode(window_, GLFW_CURSOR);
            if (currentMode == GLFW_CURSOR_DISABLED) {
                glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            else {
                captureCursor();
            }
        }
    }
    else {
        tabPressed = false;
    }

    // Update key state directly for more accurate results
    keys[GLFW_KEY_W] = glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS;
    keys[GLFW_KEY_A] = glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS;
    keys[GLFW_KEY_S] = glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS;
    keys[GLFW_KEY_D] = glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS;
    keys[GLFW_KEY_Q] = glfwGetKey(window_, GLFW_KEY_Q) == GLFW_PRESS;
    keys[GLFW_KEY_E] = glfwGetKey(window_, GLFW_KEY_E) == GLFW_PRESS;
    keys[GLFW_KEY_R] = glfwGetKey(window_, GLFW_KEY_R) == GLFW_PRESS;
    keys[GLFW_KEY_LEFT] = glfwGetKey(window_, GLFW_KEY_LEFT) == GLFW_PRESS;
    keys[GLFW_KEY_RIGHT] = glfwGetKey(window_, GLFW_KEY_RIGHT) == GLFW_PRESS;
}