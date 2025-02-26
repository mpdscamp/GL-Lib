#include "Scene.hpp"
#include "../managers/ResourceManager.hpp"
#include "../window/Window.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <GLFW/glfw3.h>

#include "../include/gl/homography.hpp"

Scene::Scene(Window& window)
    : camera_(glm::vec3(0.0f, 0.0f, 3.0f)),
    window_(window),
    lastTime_(0.0f),
    deltaTime_(0.0f)
{
    setupShaders();
    setupCube();
    setupQuad();
    setupTexture();

    // Define the source points (unit square)
    srcPoints_ = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f)
    };

    lastTime_ = static_cast<float>(glfwGetTime());

    // Set up mouse callback
    window_.setMouseCallback([this](double xoffset, double yoffset) {
        this->mouseCallback(xoffset, yoffset);
        });
}

Scene::~Scene()
{
    // Smart pointers clean up automatically.
}

void Scene::setupShaders()
{
    // Load and cache the shader via ResourceManager
    shader_ = ResourceManager::getInstance().loadShader("cube", "resources/shaders/cube/cube.vert", "resources/shaders/cube/cube.frag");
    shader_->use();
    shader_->setInt("texture1", 0); // set sampler uniform
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
    texture_ = ResourceManager::getInstance().loadTexture("shrek", "resources/textures/shrek.png");
    texture_->setFilterParameters(gl::TextureFilter::Linear, gl::TextureFilter::Linear);
}

void Scene::update(float deltaTime)
{
    deltaTime_ = deltaTime;

    // Process keyboard input for camera movement
    processInput(deltaTime);

    // Update cube rotation if auto-rotate is enabled
    if (autoRotate_) {
        cubeRotation_ += deltaTime * 30.0f; // Rotate 30 degrees per second
        if (cubeRotation_ >= 360.0f) {
            cubeRotation_ -= 360.0f;
        }
    }
}

void Scene::processInput(float deltaTime)
{
    // Get the GLFW window directly for more reliable input checking
    GLFWwindow* window = window_.getGLFWWindow();

    // Process camera movement using direct GLFW calls
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera_.ProcessKeyboard(Camera::Movement::FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera_.ProcessKeyboard(Camera::Movement::BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera_.ProcessKeyboard(Camera::Movement::LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera_.ProcessKeyboard(Camera::Movement::RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        camera_.ProcessKeyboard(Camera::Movement::UP, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        camera_.ProcessKeyboard(Camera::Movement::DOWN, deltaTime);
    }

    // Toggle auto-rotation with R key - use direct GLFW calls
    static bool rKeyPressed = false;
    bool rKeyCurrentlyPressed = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;

    if (rKeyCurrentlyPressed) {
        if (!rKeyPressed) {
            autoRotate_ = !autoRotate_;
            rKeyPressed = true;
            std::cout << "Auto-rotation " << (autoRotate_ ? "enabled" : "disabled") << std::endl;
        }
    }
    else {
        rKeyPressed = false;
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
}

void Scene::render()
{
    shader_->use();

    // ------------------------------------------------
    // 1) Render the rotating cube
    // ------------------------------------------------
    glm::mat4 model = glm::rotate(glm::mat4(1.0f),
        glm::radians(cubeRotation_),
        glm::vec3(0.5f, 1.0f, 0.0f));

    // Use camera view matrix
    glm::mat4 view = camera_.GetViewMatrix();

    // Use camera zoom for perspective projection
    glm::mat4 projection = glm::perspective(
        glm::radians(camera_.Zoom),
        static_cast<float>(window_.getWidth()) / static_cast<float>(window_.getHeight()),
        0.1f, 100.0f
    );

    glm::mat4 mvpCube = projection * view * model;
    shader_->setMat4("u_MVP", glm::value_ptr(mvpCube));

    texture_->bind(0);
    cubeVAO_->bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
    cubeVAO_->unbind();

    // ------------------------------------------------
    // 2) Compute Homography for Decal Mapping
    //    (so the decal doesn't rotate with the cube)
    // ------------------------------------------------
    glm::mat4 fixedView = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 mvpFixed = projection * fixedView;

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
    glm::mat3 T_inv = glm::inverse(T);

    // Pass the inverse homography to the shader
    shader_->setMat3("u_homography", glm::value_ptr(T_inv));

    // ------------------------------------------------
    // 3) Render the Quad with the "decal" projection
    // ------------------------------------------------
    glm::mat4 quadModel = glm::translate(glm::mat4(1.0f),
        glm::vec3(0.7f, -0.7f, 0.0f));
    quadModel = glm::scale(quadModel, glm::vec3(0.5f, 0.5f, 1.0f));

    glm::mat4 quadView = glm::mat4(1.0f);
    glm::mat4 quadProj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    glm::mat4 mvpQuad = quadProj * quadView * quadModel;
    shader_->setMat4("u_MVP", glm::value_ptr(mvpQuad));

    quadVAO_->bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    quadVAO_->unbind();
}