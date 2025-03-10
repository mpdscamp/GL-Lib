#include "Scene.hpp"
#include "../managers/ResourceManager.hpp"
#include "../window/Window.hpp"
#include "gl/logger.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <GLFW/glfw3.h>

#include "../include/gl/homography.hpp"

Scene::Scene(Window& window, ResourceManager& resourceManager)
    : camera_(glm::vec3(0.0f, 0.0f, 3.0f)),
    window_(window),
    resourceManager_(resourceManager),
    lastTime_(0.0f),
    deltaTime_(0.0f)
{
    // Register this scene with the window
    window_.addScene(this);

    // Initialize components
    setupShaders();
    setupCube();
    setupQuad();
    setupTexture();
    setupFramebuffer();
    setupInstancedRendering();

    // Define the source points (unit square)
    srcPoints_ = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f)
    };

    lastTime_ = static_cast<float>(glfwGetTime());

    // Initialize the projection matrix
    projectionMatrix_ = glm::perspective(
        glm::radians(camera_.Zoom),
        static_cast<float>(window_.getWidth()) / static_cast<float>(window_.getHeight()),
        0.1f, 100.0f
    );
    projectionDirty_ = false;

    // Set up mouse callback
    window_.setMouseCallback([this](double xoffset, double yoffset) {
        this->mouseCallback(xoffset, yoffset);
        });

    // Cache initial quad model matrix
    quadModelMatrix_ = glm::translate(glm::mat4(1.0f),
        glm::vec3(0.7f, -0.7f, 0.0f));
    quadModelMatrix_ = glm::scale(quadModelMatrix_, glm::vec3(0.5f, 0.5f, 1.0f));
}

Scene::~Scene()
{
    // Unregister from window
    window_.removeScene(this);

    // Smart pointers clean up automatically
}

void Scene::setupShaders()
{
    // Load and cache shaders via ResourceManager
    shader_ = resourceManager_.loadShader(
        "cube",
        "resources/shaders/cube/cube.vert",
        "resources/shaders/cube/cube.frag"
    );

    instancedShader_ = resourceManager_.loadShader(
        "instanced",
        "resources/shaders/instanced/instanced.vert",
        "resources/shaders/instanced/instanced.frag"
    );

    postProcessShader_ = resourceManager_.loadShader(
        "postprocess",
        "resources/shaders/postprocess/postprocess.vert",
        "resources/shaders/postprocess/postprocess.frag"
    );

    // Set default uniforms
    shader_->use();
    shader_->setInt("texture1", 0);

    instancedShader_->use();
    instancedShader_->setInt("texture1", 0);

    postProcessShader_->use();
    postProcessShader_->setInt("screenTexture", 0);
    postProcessShader_->setInt("effect", 0);
}

void Scene::setupCube()
{
    float cubeVertices[] = {
        // positions            // texture coords
        // front face
        -0.5f, -0.5f,  0.5f,     0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,     1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,     1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,     1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,     0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,     0.0f, 0.0f,

        // back face
        -0.5f, -0.5f, -0.5f,     1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,     0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,     0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,     0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,     1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,     1.0f, 0.0f,

        // left face
        -0.5f,  0.5f,  0.5f,     1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,     1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,     0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,     0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,     0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,     1.0f, 0.0f,

        // right face
         0.5f,  0.5f,  0.5f,     1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,     1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,     0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,     0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,     0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,     1.0f, 0.0f,

         // bottom face
         -0.5f, -0.5f, -0.5f,     0.0f, 1.0f,
          0.5f, -0.5f, -0.5f,     1.0f, 1.0f,
          0.5f, -0.5f,  0.5f,     1.0f, 0.0f,
          0.5f, -0.5f,  0.5f,     1.0f, 0.0f,
         -0.5f, -0.5f,  0.5f,     0.0f, 0.0f,
         -0.5f, -0.5f, -0.5f,     0.0f, 1.0f,

         // top face
         -0.5f,  0.5f, -0.5f,     0.0f, 1.0f,
          0.5f,  0.5f, -0.5f,     1.0f, 1.0f,
          0.5f,  0.5f,  0.5f,     1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,     1.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,     0.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,     0.0f, 1.0f
    };

    cubeVAO_ = std::make_unique<gl::VertexArray>();
    cubeVBO_ = std::make_unique<gl::VertexBuffer>();

    cubeVAO_->bind();
    cubeVBO_->bind();

    std::vector<float> cubeData(cubeVertices, cubeVertices + sizeof(cubeVertices) / sizeof(float));
    cubeVBO_->setData(cubeData, gl::BufferUsage::StaticDraw);

    int stride = 5 * sizeof(float);
    // Position attribute
    cubeVAO_->setVertexAttribute(0, 3, gl::DataType::Float, false, stride, 0);
    // Texture coordinate attribute
    cubeVAO_->setVertexAttribute(1, 2, gl::DataType::Float, false, stride, 3 * sizeof(float));

    cubeVBO_->unbind();
    cubeVAO_->unbind();
}

void Scene::setupQuad()
{
    float quadVertices[] = {
        // positions         // texture coords
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f
    };

    quadVAO_ = std::make_unique<gl::VertexArray>();
    quadVBO_ = std::make_unique<gl::VertexBuffer>();

    quadVAO_->bind();
    quadVBO_->bind();

    std::vector<float> quadData(quadVertices, quadVertices + sizeof(quadVertices) / sizeof(float));
    quadVBO_->setData(quadData, gl::BufferUsage::StaticDraw);

    int stride = 5 * sizeof(float);
    // Position attribute
    quadVAO_->setVertexAttribute(0, 3, gl::DataType::Float, false, stride, 0);
    // Texture coordinate attribute
    quadVAO_->setVertexAttribute(1, 2, gl::DataType::Float, false, stride, 3 * sizeof(float));

    quadVBO_->unbind();
    quadVAO_->unbind();
}

void Scene::setupTexture()
{
    // Load the texture via ResourceManager
    texture_ = resourceManager_.loadTexture("shrek", "resources/textures/shrek.png");
    texture_->setFilterParameters(gl::TextureFilter::Linear, gl::TextureFilter::Linear);
}

void Scene::setupFramebuffer()
{
    // Create a framebuffer for post-processing
    framebuffer_ = std::make_unique<gl::FrameBuffer>(window_.getWidth(), window_.getHeight());
}

void Scene::setupInstancedRendering()
{
    // Create instance data for a grid of cubes
    instances_.resize(100); // 10x10 grid
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int index = i * 10 + j;
            float x = (i - 5) * 2.0f + 1.0f;
            float z = (j - 5) * 2.0f + 1.0f;

            // Set instance data
            instances_[index].position = glm::vec3(x, -1.0f, z);
            instances_[index].scale = glm::vec3(0.2f);

            // Create a gradient of colors
            float r = i / 9.0f;
            float g = 0.4f;
            float b = j / 9.0f;
            instances_[index].color = glm::vec3(r, g, b);
        }
    }

    // Create and setup instance buffer
    instanceVBO_ = std::make_unique<gl::VertexBuffer>();

    cubeVAO_->bind();
    instanceVBO_->bind();
    instanceVBO_->setData(instances_, gl::BufferUsage::DynamicDraw);

    // Position attribute (per instance)
    cubeVAO_->setVertexAttribute(2, 3, gl::DataType::Float, false,
        sizeof(InstanceData), offsetof(InstanceData, position));
    cubeVAO_->setAttributeDivisor(2, 1); // Advance once per instance

    // Scale attribute (per instance)
    cubeVAO_->setVertexAttribute(3, 3, gl::DataType::Float, false,
        sizeof(InstanceData), offsetof(InstanceData, scale));
    cubeVAO_->setAttributeDivisor(3, 1);

    // Color attribute (per instance)
    cubeVAO_->setVertexAttribute(4, 3, gl::DataType::Float, false,
        sizeof(InstanceData), offsetof(InstanceData, color));
    cubeVAO_->setAttributeDivisor(4, 1);

    instanceVBO_->unbind();
    cubeVAO_->unbind();
}

void Scene::update(float deltaTime)
{
    deltaTime_ = deltaTime;

    // Process keyboard input for camera movement
    processInput(deltaTime);

    // Update projection matrix if needed
    if (projectionDirty_) {
        projectionMatrix_ = glm::perspective(
            glm::radians(camera_.Zoom),
            static_cast<float>(window_.getWidth()) / static_cast<float>(window_.getHeight()),
            0.1f, 100.0f
        );
        projectionDirty_ = false;
    }

    // Update cube rotation if auto-rotate is enabled
    if (autoRotate_) {
        cubeRotation_ += deltaTime * 30.0f; // Rotate 30 degrees per second
        if (cubeRotation_ >= 360.0f) {
            cubeRotation_ -= 360.0f;
        }
    }

    // Update homography flag when camera moves significantly
    float positionDelta = glm::distance(camera_.Position, lastCameraPos_);
    float rotationDelta = std::abs(camera_.Yaw - lastCameraYaw_) +
        std::abs(camera_.Pitch - lastCameraPitch_);

    if (positionDelta > 0.1f || rotationDelta > 0.5f) {
        homographyDirty_ = true;
    }
}

void Scene::processInput(float deltaTime)
{
    // Get the GLFW window for input
    GLFWwindow* window = window_.getGLFWWindow();

    // Process camera movement using Window's key state
    if (window_.isKeyPressed(GLFW_KEY_W)) {
        camera_.ProcessKeyboard(Camera::Movement::FORWARD, deltaTime);
    }
    if (window_.isKeyPressed(GLFW_KEY_S)) {
        camera_.ProcessKeyboard(Camera::Movement::BACKWARD, deltaTime);
    }
    if (window_.isKeyPressed(GLFW_KEY_A)) {
        camera_.ProcessKeyboard(Camera::Movement::LEFT, deltaTime);
    }
    if (window_.isKeyPressed(GLFW_KEY_D)) {
        camera_.ProcessKeyboard(Camera::Movement::RIGHT, deltaTime);
    }
    if (window_.isKeyPressed(GLFW_KEY_Q)) {
        camera_.ProcessKeyboard(Camera::Movement::UP, deltaTime);
    }
    if (window_.isKeyPressed(GLFW_KEY_E)) {
        camera_.ProcessKeyboard(Camera::Movement::DOWN, deltaTime);
    }

    // Toggle auto-rotation with R key
    static bool rKeyPressed = false;
    bool rKeyCurrentlyPressed = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;

    if (rKeyCurrentlyPressed) {
        if (!rKeyPressed) {
            autoRotate_ = !autoRotate_;
            rKeyPressed = true;
            gl::logInfo("Auto-rotation " + std::string(autoRotate_ ? "enabled" : "disabled"));
        }
    }
    else {
        rKeyPressed = false;
    }

    // Toggle post-processing with P key
    static bool pKeyPressed = false;
    bool pKeyCurrentlyPressed = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;

    if (pKeyCurrentlyPressed) {
        if (!pKeyPressed) {
            usePostProcessing_ = !usePostProcessing_;
            pKeyPressed = true;
            gl::logInfo("Post-processing " + std::string(usePostProcessing_ ? "enabled" : "disabled"));
        }
    }
    else {
        pKeyPressed = false;
    }

    // Cycle post-processing effects with [ and ] keys
    static bool leftBracketPressed = false;
    static bool rightBracketPressed = false;
    bool leftBracketCurrentlyPressed = glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS;
    bool rightBracketCurrentlyPressed = glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS;

    if (leftBracketCurrentlyPressed) {
        if (!leftBracketPressed) {
            currentEffect_ = (currentEffect_ - 1 + 5) % 5; // 5 effects, cycle backwards
            leftBracketPressed = true;
            gl::logInfo("Post-processing effect: " + std::to_string(currentEffect_));
        }
    }
    else {
        leftBracketPressed = false;
    }

    if (rightBracketCurrentlyPressed) {
        if (!rightBracketPressed) {
            currentEffect_ = (currentEffect_ + 1) % 5; // 5 effects, cycle forwards
            rightBracketPressed = true;
            gl::logInfo("Post-processing effect: " + std::to_string(currentEffect_));
        }
    }
    else {
        rightBracketPressed = false;
    }

    // Manual rotation with arrow keys when auto-rotation is off
    if (!autoRotate_) {
        bool rightKeyPressed = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
        bool leftKeyPressed = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;

        if (rightKeyPressed) {
            cubeRotation_ += deltaTime * 60.0f; // Rotate faster with arrow keys
            if (cubeRotation_ >= 360.0f) {
                cubeRotation_ -= 360.0f;
            }
        }
        if (leftKeyPressed) {
            cubeRotation_ -= deltaTime * 60.0f;
            if (cubeRotation_ < 0.0f) {
                cubeRotation_ += 360.0f;
            }
        }
    }
}

void Scene::mouseCallback(double xoffset, double yoffset)
{
    camera_.ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
    homographyDirty_ = true;
}

void Scene::render()
{
    // Begin framebuffer rendering if post-processing is enabled
    if (usePostProcessing_) {
        framebuffer_->bind();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // Render individual components
    renderCube();
    renderQuad();

    // End framebuffer rendering and apply post-processing if enabled
    if (usePostProcessing_) {
        framebuffer_->unbind();
        renderPostProcess();
    }
}

void Scene::renderCube()
{
    shader_->use();

    // Compute model matrix for rotating cube
    glm::mat4 model = glm::rotate(glm::mat4(1.0f),
        glm::radians(cubeRotation_),
        glm::vec3(0.5f, 1.0f, 0.0f));

    // Use camera view matrix
    glm::mat4 view = camera_.GetViewMatrix();

    // Compose MVP matrix
    glm::mat4 mvpCube = projectionMatrix_ * view * model;
    shader_->setMat4("u_MVP", glm::value_ptr(mvpCube));

    // Bind texture and draw cube
    texture_->bind(0);
    cubeVAO_->bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
    cubeVAO_->unbind();
}

void Scene::renderQuad()
{
    shader_->use();

    // Save current depth function state
    GLint previousDepthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunc);

    // Make the quad always pass the depth test (appear in front)
    glDepthFunc(GL_ALWAYS);

    // Compute homography for decal mapping when needed
    if (homographyDirty_) {
        glm::mat4 fixedView = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        glm::mat4 mvpFixed = projectionMatrix_ * fixedView;

        std::array<glm::vec4, 4> faceVertsFixed = {
            mvpFixed * glm::vec4(-0.5f, -0.5f, 0.5f, 1.0f),
            mvpFixed * glm::vec4(0.5f, -0.5f, 0.5f, 1.0f),
            mvpFixed * glm::vec4(0.5f,  0.5f, 0.5f, 1.0f),
            mvpFixed * glm::vec4(-0.5f,  0.5f, 0.5f, 1.0f)
        };

        // Convert clip-space -> NDC -> [0,1] range
        std::array<glm::vec2, 4> dstPoints;
        for (int i = 0; i < 4; i++) {
            glm::vec4 v = faceVertsFixed[i];
            v /= v.w; // perspective divide
            // map from [-1,1] to [0,1]
            dstPoints[i] = glm::vec2((v.x + 1.0f) * 0.5f, (v.y + 1.0f) * 0.5f);
        }

        glm::mat3 T = computeHomography(srcPoints_, dstPoints);
        homographyCache_ = glm::inverse(T);
        homographyDirty_ = false;

        // Update last camera state
        lastCameraPos_ = camera_.Position;
        lastCameraYaw_ = camera_.Yaw;
        lastCameraPitch_ = camera_.Pitch;
    }

    // Pass the cached inverse homography to the shader
    shader_->setMat3("u_homography", glm::value_ptr(homographyCache_));

    // Use cached model matrix for the quad
    glm::mat4 quadView = glm::mat4(1.0f);
    glm::mat4 quadProj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    glm::mat4 mvpQuad = quadProj * quadView * quadModelMatrix_;

    shader_->setMat4("u_MVP", glm::value_ptr(mvpQuad));

    // Draw the quad with decal
    quadVAO_->bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    quadVAO_->unbind();

    // Restore previous depth function
    glDepthFunc(previousDepthFunc);
}

void Scene::renderInstanced()
{
    instancedShader_->use();

    // Update animation for the instance data
    static float time = 0.0f;
    time += deltaTime_ * 0.5f;

    for (int i = 0; i < 100; i++) {
        // Add a subtle wave animation to the instances
        float x = instances_[i].position.x;
        float z = instances_[i].position.z;
        instances_[i].position.y = -1.0f + 0.2f * sin(time + x * 0.5f + z * 0.3f);

        // Pulse the scaling
        float scale = 0.2f + 0.05f * sin(time * 2.0f + i * 0.1f);
        instances_[i].scale = glm::vec3(scale);
    }

    // Update instance buffer with new data
    instanceVBO_->bind();
    instanceVBO_->updateSubData(instances_);

    // Set shader uniforms
    glm::mat4 view = camera_.GetViewMatrix();
    instancedShader_->setMat4("projection", glm::value_ptr(projectionMatrix_));
    instancedShader_->setMat4("view", glm::value_ptr(view));

    // Bind texture and draw instanced cubes
    texture_->bind(0);
    cubeVAO_->bind();
    cubeVAO_->drawArraysInstanced(gl::DrawMode::Triangles, 0, 36, instances_.size());
    cubeVAO_->unbind();
}

void Scene::renderPostProcess()
{
    // Clear the default framebuffer
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Disable depth testing for post-processing pass
    glDisable(GL_DEPTH_TEST);

    // Use post-processing shader
    postProcessShader_->use();
    postProcessShader_->setInt("effect", currentEffect_);

    // Bind framebuffer texture
    framebuffer_->getColorTexture()->bind(0);

    // Render a full-screen quad
    quadVAO_->bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    quadVAO_->unbind();

    // Re-enable depth testing
    glEnable(GL_DEPTH_TEST);
}

void Scene::onWindowResize(int width, int height)
{
    // Mark projection matrix as dirty
    projectionDirty_ = true;

    // Resize framebuffer if it exists
    if (framebuffer_) {
        framebuffer_->resize(width, height);
    }

    // Mark homography as dirty since projection has changed
    homographyDirty_ = true;
}