#ifndef SCENE_HPP
#define SCENE_HPP

#include <memory>
#include <array>
#include <vector>
#include <glm/glm.hpp>

// Include full definitions of classes
#include "../include/gl/vertex_array.hpp"
#include "../include/gl/buffer.hpp"
#include "../include/gl/framebuffer.hpp"
#include "../window/Camera.hpp"

// Forward declarations
namespace gl {
    class Shader;
    class Texture;
}
class Window;
class ResourceManager;

// Structure for instanced rendering
struct InstanceData {
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 color;
};

class Scene {
public:
    Scene(Window& window, ResourceManager& resourceManager);
    ~Scene();

    // Non-copyable
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    // Main methods
    void update(float deltaTime);
    void render();

    // Window resize handler
    void onWindowResize(int width, int height);

    // Mark projection matrix as dirty (needs recomputation)
    void setProjectionDirty() { projectionDirty_ = true; }

private:
    // Setup methods
    void setupShaders();
    void setupCube();
    void setupQuad();
    void setupTexture();
    void setupFramebuffer();
    void setupInstancedRendering();

    // Render components
    void renderCube();
    void renderQuad();
    void renderInstanced();
    void renderPostProcess();

    // Input handling
    void processInput(float deltaTime);
    void mouseCallback(double xoffset, double yoffset);

    // Shared pointers for cached resources from ResourceManager
    std::shared_ptr<gl::Shader> shader_;
    std::shared_ptr<gl::Shader> instancedShader_;
    std::shared_ptr<gl::Shader> postProcessShader_;
    std::shared_ptr<gl::Texture> texture_;

    // Unique pointers for objects owned solely by Scene
    std::unique_ptr<gl::VertexArray> cubeVAO_;
    std::unique_ptr<gl::VertexBuffer> cubeVBO_;
    std::unique_ptr<gl::VertexArray> quadVAO_;
    std::unique_ptr<gl::VertexBuffer> quadVBO_;
    std::unique_ptr<gl::VertexBuffer> instanceVBO_;
    std::unique_ptr<gl::FrameBuffer> framebuffer_;

    // For homography decal logic
    std::array<glm::vec2, 4> srcPoints_;

    // Instanced rendering data
    std::vector<InstanceData> instances_;

    // Camera
    Camera camera_;
    Window& window_;
    ResourceManager& resourceManager_;

    // Timing
    float lastTime_;
    float deltaTime_;

    // Cube rotation control
    bool autoRotate_ = true;
    float cubeRotation_ = 0.0f;

    // Cached matrices to avoid redundant calculations
    glm::mat4 projectionMatrix_;
    bool projectionDirty_ = true;

    // Homography cache
    glm::mat3 homographyCache_;
    bool homographyDirty_ = true;
    glm::vec3 lastCameraPos_;
    float lastCameraYaw_;
    float lastCameraPitch_;

    // Cached model matrices
    glm::mat4 quadModelMatrix_;

    // Post-processing effect control
    bool usePostProcessing_ = false;
    int currentEffect_ = 0;
};

#endif // SCENE_HPP