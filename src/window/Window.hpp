#ifndef WINDOW_HPP
#define WINDOW_HPP

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>
#include <functional>
#include <vector>
#include <array>
#include <memory>

class InputManager;
class Scene;

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

    // Input handling
    void processInput(float deltaTime);
    void captureCursor();

    // Mouse handling
    void setMouseCallback(std::function<void(double, double)> callback);

    // Window resize handling
    void registerResizeCallback(std::function<void(int, int)> callback);

    // Get window dimensions
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    // Input state access
    bool isKeyPressed(int key) const;
    bool isKeyHeld(int key) const;

    // Add a scene for notification purposes
    void addScene(Scene* scene);
    void removeScene(Scene* scene);

private:
    GLFWwindow* window_;
    int width_;
    int height_;

    // Input state
    std::array<bool, 1024> keys = {}; // All initialized to false

    // Mouse state
    bool firstMouse = true;
    double lastX, lastY;
    std::function<void(double, double)> mouseCallback_;

    // Resize callbacks
    std::vector<std::function<void(int, int)>> resizeCallbacks_;

    // Scenes to notify of changes
    std::vector<Scene*> scenes_;

    // GLFW callbacks (static methods that dispatch to the appropriate window)
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void windowFocusCallback(GLFWwindow* window, int focused);

    // Static map of window instances for callbacks
    static std::unordered_map<GLFWwindow*, Window*> windowInstances;

    // Allow our static callbacks to access private members
    friend void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    friend void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    friend void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    friend void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    friend void windowFocusCallback(GLFWwindow* window, int focused);
};

#endif // WINDOW_HPP