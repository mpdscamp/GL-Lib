#include "InputHandler.hpp"
#include "../camera/CameraComponent.hpp"
#include "../rendering/PostProcessor.hpp"
#include "../geometry/MeshComponent.hpp"
#include "../../window/Window.hpp"
#include "../../core/Entity.hpp"
#include "../../core/Scene.hpp"
#include "../../managers/ResourceManager.hpp"
#include "../../gl/logger.hpp"

InputHandler::InputHandler() {
    name_ = "InputHandler";
}

void InputHandler::init() {
    // Find camera in scene
    if (entity_->getScene()) {
        for (auto entity = entity_->getScene()->findEntity("MainCamera"); entity != nullptr; entity = nullptr) {
            camera_ = entity->getComponent<CameraComponent>();
            if (camera_) {
                break;
            }
        }
    }

    // Find post-processor in scene
    if (entity_->getScene()) {
        for (auto entity = entity_->getScene()->findEntity("PostProcessor"); entity != nullptr; entity = nullptr) {
            postProcessor_ = entity->getComponent<PostProcessor>();
            if (postProcessor_) {
                break;
            }
        }
    }

    // Register with window for mouse movement
    if (entity_->getScene()) {
        entity_->getScene()->getWindow().setMouseCallback([this](double xoffset, double yoffset) {
            this->mouseCallback(xoffset, yoffset);
            });
    }

    gl::logDebug("InputHandler initialized");
}

void InputHandler::update(float deltaTime) {
    processInput(deltaTime);
}

void InputHandler::processInput(float deltaTime) {
    if (!entity_->getScene()) return;

    GLFWwindow* window = entity_->getScene()->getWindow().getGLFWWindow();

    // Camera movement
    if (camera_) {
        if (isKeyPressed(GLFW_KEY_W)) {
            camera_->processKeyboard(CameraComponent::FORWARD, deltaTime);
        }
        if (isKeyPressed(GLFW_KEY_S)) {
            camera_->processKeyboard(CameraComponent::BACKWARD, deltaTime);
        }
        if (isKeyPressed(GLFW_KEY_A)) {
            camera_->processKeyboard(CameraComponent::LEFT, deltaTime);
        }
        if (isKeyPressed(GLFW_KEY_D)) {
            camera_->processKeyboard(CameraComponent::RIGHT, deltaTime);
        }
        if (isKeyPressed(GLFW_KEY_Q)) {
            camera_->processKeyboard(CameraComponent::UP, deltaTime);
        }
        if (isKeyPressed(GLFW_KEY_E)) {
            camera_->processKeyboard(CameraComponent::DOWN, deltaTime);
        }
    }

    // Toggle auto-rotation with R key
    bool rKeyCurrentlyPressed = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
    if (rKeyCurrentlyPressed && !rKeyPressed_) {
        // Find cube entity and toggle auto-rotation
        auto cubeEntity = entity_->getScene()->findEntity("Cube");
        if (cubeEntity) {
            auto cubeMesh = cubeEntity->getComponent<MeshComponent>();
            if (cubeMesh) {
                autoRotate_ = !autoRotate_;
                cubeMesh->setAutoRotate(autoRotate_);
                gl::logInfo("Auto-rotation " + std::string(autoRotate_ ? "enabled" : "disabled"));
            }
        }
    }
    rKeyPressed_ = rKeyCurrentlyPressed;

    // Toggle post-processing with P key
    if (postProcessor_) {
        bool pKeyCurrentlyPressed = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
        if (pKeyCurrentlyPressed && !pKeyPressed_) {
            postProcessor_->setEnabled(!postProcessor_->isEnabled());
            gl::logInfo("Post-processing " + std::string(postProcessor_->isEnabled() ? "enabled" : "disabled"));
        }
        pKeyPressed_ = pKeyCurrentlyPressed;

        // Cycle post-processing effects with [ and ] keys
        bool leftBracketCurrentlyPressed = glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS;
        bool rightBracketCurrentlyPressed = glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS;

        if (leftBracketCurrentlyPressed && !leftBracketPressed_) {
            postProcessor_->previousEffect();
        }
        leftBracketPressed_ = leftBracketCurrentlyPressed;

        if (rightBracketCurrentlyPressed && !rightBracketPressed_) {
            postProcessor_->nextEffect();
        }
        rightBracketPressed_ = rightBracketCurrentlyPressed;
    }

    // Shader hot-reload with F5 key
    bool f5KeyCurrentlyPressed = glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS;
    if (f5KeyCurrentlyPressed && !f5KeyPressed_) {
        entity_->getScene()->getResourceManager().reloadShaders();
        gl::logInfo("Shaders reloaded");
    }
    f5KeyPressed_ = f5KeyCurrentlyPressed;

    // Manual rotation with arrow keys when auto-rotation is off
    if (!autoRotate_) {
        if (entity_->getScene()) {
            auto cubeEntity = entity_->getScene()->findEntity("Cube");
            if (cubeEntity) {
                auto cubeMesh = cubeEntity->getComponent<MeshComponent>();
                if (cubeMesh) {
                    bool rightKeyPressed = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
                    bool leftKeyPressed = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;

                    // Get current rotation angle and axis from cubeMesh
                    float rotationAngle = cubeMesh->getRotationAngle();
                    const glm::vec3& rotationAxis = cubeMesh->getRotationAxis();

                    if (rightKeyPressed) {
                        rotationAngle += deltaTime * 60.0f;
                    }
                    if (leftKeyPressed) {
                        rotationAngle -= deltaTime * 60.0f;
                    }

                    // Update rotation if changed
                    if (rightKeyPressed || leftKeyPressed) {
                        cubeMesh->setRotation(rotationAngle, rotationAxis);
                    }
                }
            }
        }
    }
}

void InputHandler::mouseCallback(double xoffset, double yoffset) {
    if (camera_) {
        camera_->processMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
    }
}

bool InputHandler::isKeyPressed(int key) const {
    auto it = keyState_.find(key);
    if (it != keyState_.end()) {
        return it->second;
    }
    // If not explicitly tracked, query GLFW directly
    if (entity_->getScene()) {
        return glfwGetKey(entity_->getScene()->getWindow().getGLFWWindow(), key) == GLFW_PRESS;
    }
    return false;
}