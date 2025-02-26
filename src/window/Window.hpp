#ifndef WINDOW_HPP
#define WINDOW_HPP

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>
#include <functional>

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    // Non-copyable
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Movable
    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    bool shouldClose() const;
    void swapBuffers() const;
    void pollEvents() const;

    GLFWwindow* getGLFWWindow() const;

    // Set a framebuffer resize callback
    void setFramebufferSizeCallback(GLFWframebuffersizefun callback);

    // Mouse callbacks
    void setMouseCallback(std::function<void(double, double)> callback);

    // Set cursor mode (normal, hidden, disabled)
    void setCursorMode(int mode);

    // Get window dimensions
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    // Process input (e.g. keyboard)
    void processInput(float deltaTime);

    // Force cursor capture (call this when clicking or focusing window)
    void captureCursor();

    // Mouse position and state
    bool firstMouse = true;
    double lastX, lastY;

    // Key state
    bool keys[1024] = { false };

    // Make accessible to our callback wrappers
    int width_;
    int height_;
    std::function<void(double, double)> mouseCallback_;

private:
    GLFWwindow* window_;

    // Removed static callbacks as they are now defined outside the class
    // This allows easier access to the callback wrapper functions
};

#endif // WINDOW_HPP