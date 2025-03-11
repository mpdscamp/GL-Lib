#include "CameraComponent.hpp"
#include "../../core/Entity.hpp"
#include "../../core/Scene.hpp"
#include "../../window/Window.hpp"
#include "../../gl/logger.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

CameraComponent::CameraComponent(const glm::vec3& position)
    : position_(position),
    front_(glm::vec3(0.0f, 0.0f, -1.0f)),
    worldUp_(glm::vec3(0.0f, 1.0f, 0.0f)),
    yaw_(-90.0f),
    pitch_(0.0f),
    movementSpeed_(2.5f),
    mouseSensitivity_(0.1f),
    zoom_(45.0f),
    projectionDirty_(true),
    screenWidth_(800),
    screenHeight_(600)
{
    name_ = "CameraComponent";
    updateCameraVectors();
}

void CameraComponent::init() {
    if (entity_ && entity_->getScene()) {
        Window& window = entity_->getScene()->getWindow();
        screenWidth_ = window.getWidth();
        screenHeight_ = window.getHeight();
        projectionDirty_ = true;
    }
}

void CameraComponent::update(float deltaTime) {
    // Nothing to update automatically
}

void CameraComponent::processKeyboard(int direction, float deltaTime) {
    float velocity = movementSpeed_ * deltaTime;

    if (direction == FORWARD)
        position_ += front_ * velocity;
    if (direction == BACKWARD)
        position_ -= front_ * velocity;
    if (direction == LEFT)
        position_ -= right_ * velocity;
    if (direction == RIGHT)
        position_ += right_ * velocity;
    if (direction == UP)
        position_ += up_ * velocity;
    if (direction == DOWN)
        position_ -= up_ * velocity;
}

void CameraComponent::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= mouseSensitivity_;
    yoffset *= mouseSensitivity_;

    yaw_ += xoffset;
    pitch_ += yoffset;

    // Constrain pitch
    if (constrainPitch) {
        pitch_ = std::clamp(pitch_, -89.0f, 89.0f);
    }

    // Update front, right and up vectors
    updateCameraVectors();
}

void CameraComponent::processMouseScroll(float yoffset) {
    zoom_ -= yoffset;
    zoom_ = std::clamp(zoom_, 1.0f, 45.0f);
    projectionDirty_ = true;
}

glm::mat4 CameraComponent::getViewMatrix() const {
    return glm::lookAt(position_, position_ + front_, up_);
}

glm::mat4 CameraComponent::getProjectionMatrix() {
    // If projection is dirty, recompute it (expensive)
    if (projectionDirty_) {
        projectionMatrix_ = glm::perspective(
            glm::radians(zoom_),
            static_cast<float>(screenWidth_) / static_cast<float>(screenHeight_),
            0.1f, 100.0f
        );
        projectionDirty_ = false;
    }

    return projectionMatrix_;
}

void CameraComponent::onWindowResize(int width, int height) {
    screenWidth_ = width;
    screenHeight_ = height;
    projectionDirty_ = true;

    gl::logDebug("Camera projection updated for " + std::to_string(width) + "x" + std::to_string(height));
}

void CameraComponent::updateCameraVectors() {
    // Calculate the new front vector
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    newFront.y = sin(glm::radians(pitch_));
    newFront.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front_ = glm::normalize(newFront);

    // Recalculate the right and up vector
    right_ = glm::normalize(glm::cross(front_, worldUp_));
    up_ = glm::normalize(glm::cross(right_, front_));
}