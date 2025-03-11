#include "HomographyEffect.hpp"
#include "../geometry/MeshComponent.hpp"
#include "../camera/CameraComponent.hpp"
#include "../rendering/MeshRenderer.hpp"
#include "../../core/Entity.hpp"
#include "../../core/Scene.hpp"
#include "../../../include/gl/homography.hpp"
#include "../../gl/logger.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

HomographyEffect::HomographyEffect() {
    name_ = "HomographyEffect";

    // Define the source points (unit square)
    srcPoints_ = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f)
    };

    // Initialize homography matrix
    homographyCache_ = glm::mat3(1.0f);

    // Initialize quad model matrix
    quadModelMatrix_ = glm::translate(glm::mat4(1.0f), glm::vec3(0.7f, -0.7f, 0.0f));
    quadModelMatrix_ = glm::scale(quadModelMatrix_, glm::vec3(0.5f, 0.5f, 1.0f));
}

void HomographyEffect::init() {
    // Get required components
    quadMesh_ = entity_->getComponent<MeshComponent>();
    if (!quadMesh_) {
        gl::logWarning("HomographyEffect requires a MeshComponent on the same entity");
    }

    renderer_ = entity_->getComponent<MeshRenderer>();
    if (!renderer_) {
        gl::logWarning("HomographyEffect requires a MeshRenderer on the same entity");
    }

    // Find camera in scene
    if (entity_->getScene()) {
        auto cameraEntity = entity_->getScene()->findEntity("MainCamera");
        if (cameraEntity) {
            camera_ = cameraEntity->getComponent<CameraComponent>();
        }
    }

    if (!camera_) {
        gl::logWarning("HomographyEffect couldn't find a CameraComponent in the scene");
    }

    gl::logDebug("HomographyEffect initialized");
}

void HomographyEffect::update(float deltaTime) {
    if (!camera_) return;

    // Update homography when camera moves significantly
    float positionDelta = glm::distance(camera_->getPosition(), lastCameraPos_);
    float rotationDelta = std::abs(camera_->getYaw() - lastCameraYaw_) +
        std::abs(camera_->getPitch() - lastCameraPitch_);

    if (positionDelta > 0.1f || rotationDelta > 0.5f) {
        homographyDirty_ = true;
    }

    if (homographyDirty_) {
        updateHomography();
    }
}

void HomographyEffect::render() {
    if (!quadMesh_ || !renderer_ || !camera_) return;

    // Store current depth function
    glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunc_);

    // Make the quad always pass the depth test (appear in front)
    glDepthFunc(GL_ALWAYS);

    // Pass the cached inverse homography to the shader
    renderer_->getShader()->setMat3("u_homography", glm::value_ptr(homographyCache_));

    // Set up screen-space quad positioning
    glm::mat4 quadView = glm::mat4(1.0f);  // Identity view matrix for screen space
    glm::mat4 quadProj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    // Position in bottom right corner
    glm::mat4 quadModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.7f, -0.7f, 0.0f));
    quadModel = glm::scale(quadModel, glm::vec3(0.5f, 0.5f, 1.0f));

    glm::mat4 mvp = quadProj * quadView * quadModel;

    // Bypass camera transformations
    renderer_->setMVPMatrix(mvp);

    // Manually trigger the renderer
    renderer_->render();

    // Restore previous depth function
    glDepthFunc(previousDepthFunc_);

    // Clear external model matrix after rendering
    renderer_->clearModelMatrix();
}

void HomographyEffect::updateHomography() {
    if (!camera_) return;

    // Compute homography for decal mapping
    glm::mat4 fixedView = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 mvpFixed = camera_->getProjectionMatrix() * fixedView;

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

    glm::mat3 T = gl::computeHomography(srcPoints_, dstPoints);
    homographyCache_ = glm::inverse(T);
    homographyDirty_ = false;

    // Update last camera state
    lastCameraPos_ = camera_->getPosition();
    lastCameraYaw_ = camera_->getYaw();
    lastCameraPitch_ = camera_->getPitch();

    gl::logDebug("HomographyEffect updated homography matrix");
}