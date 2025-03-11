#ifndef HOMOGRAPHY_EFFECT_HPP
#define HOMOGRAPHY_EFFECT_HPP

#include "../../core/Component.hpp"
#include "../../../include/gl/shader.hpp"
#include <glm/glm.hpp>
#include <array>
#include <memory>

class MeshComponent;
class CameraComponent;
class MeshRenderer;

class HomographyEffect : public Component {
public:
    HomographyEffect();

    void init() override;
    void update(float deltaTime) override;
    void render() override;

    void setQuadTransform(const glm::mat4& transform) {
        quadModelMatrix_ = transform;
        homographyDirty_ = true;
    }

private:
    void updateHomography();

    MeshComponent* quadMesh_ = nullptr;
    MeshRenderer* renderer_ = nullptr;
    CameraComponent* camera_ = nullptr;

    std::array<glm::vec2, 4> srcPoints_;
    glm::mat3 homographyCache_;
    glm::mat4 quadModelMatrix_ = glm::mat4(1.0f);

    bool homographyDirty_ = true;
    glm::vec3 lastCameraPos_ = glm::vec3(0.0f);
    float lastCameraYaw_ = 0.0f;
    float lastCameraPitch_ = 0.0f;

    // For depth state management
    GLint previousDepthFunc_ = GL_LESS;
};

#endif // HOMOGRAPHY_EFFECT_HPP