#ifndef SCENE_HPP
#define SCENE_HPP

#include <memory>
#include <array>
#include <glm/glm.hpp>

// Include full definitions of VertexArray and VertexBuffer
#include "../include/gl/vertex_array.hpp"
#include "../include/gl/buffer.hpp"
#include "../window/Camera.hpp"

// Forward-declare Shader and Texture (we only store them as shared_ptr)
namespace gl {
    class Shader;
    class Texture;
}

class Window; // Forward declaration

class Scene {
public:
    Scene(Window& window);
    ~Scene();

    // Non-copyable
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    void update(float deltaTime);
    void render();

    // Process camera input
    void processInput(float deltaTime);

    // Mouse movement callback
    void mouseCallback(double xoffset, double yoffset);

private:
    void setupShaders();
    void setupCube();
    void setupQuad();
    void setupTexture();

    // Shared pointers for cached resources
    std::shared_ptr<gl::Shader>  shader_;
    std::shared_ptr<gl::Texture> texture_;

    // Unique pointers for objects owned solely by Scene
    std::unique_ptr<gl::VertexArray>  cubeVAO_;
    std::unique_ptr<gl::VertexBuffer> cubeVBO_;
    std::unique_ptr<gl::VertexArray>  quadVAO_;
    std::unique_ptr<gl::VertexBuffer> quadVBO_;

    // For homography decal logic
    std::array<glm::vec2, 4> srcPoints_;

    // Camera
    Camera camera_;
    Window& window_;

    float lastTime_;
    float deltaTime_;

    // Cube rotation control
    bool autoRotate_ = true;
    float cubeRotation_ = 0.0f;
};

#endif // SCENE_HPP