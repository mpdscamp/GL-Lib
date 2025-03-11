#ifndef CAMERA_COMPONENT_HPP
#define CAMERA_COMPONENT_HPP

#include "../../core/Component.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class CameraComponent : public Component {
public:
    CameraComponent(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 3.0f));

    void init() override;
    void update(float deltaTime) override;

    // Camera controls
    void processKeyboard(int direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void processMouseScroll(float yoffset);

    // Get view and projection matrices
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix();

    // Window resize handler
    void onWindowResize(int width, int height);

    // Get camera properties
    glm::vec3 getPosition() const { return position_; }
    glm::vec3 getFront() const { return front_; }
    glm::vec3 getUp() const { return up_; }
    glm::vec3 getRight() const { return right_; }

    float getYaw() const { return yaw_; }
    float getPitch() const { return pitch_; }
    float getZoom() const { return zoom_; }

    // Camera movement directions
    enum Direction {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

private:
    void updateCameraVectors();

    // Camera attributes
    glm::vec3 position_;
    glm::vec3 front_;
    glm::vec3 up_;
    glm::vec3 right_;
    glm::vec3 worldUp_;

    // Euler angles
    float yaw_;
    float pitch_;

    // Camera options
    float movementSpeed_;
    float mouseSensitivity_;
    float zoom_;

    // Projection matrix cache
    glm::mat4 projectionMatrix_;
    bool projectionDirty_;

    // Screen dimensions for projection
    int screenWidth_;
    int screenHeight_;
};

#endif // CAMERA_COMPONENT_HPP