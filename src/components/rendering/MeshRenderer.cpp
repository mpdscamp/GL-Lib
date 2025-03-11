#include "MeshRenderer.hpp"
#include "../camera/CameraComponent.hpp"
#include "../geometry/MeshComponent.hpp"
#include "../../core/Entity.hpp"
#include "../../core/Scene.hpp"
#include "../../gl/logger.hpp"
#include <glm/gtc/type_ptr.hpp>

MeshRenderer::MeshRenderer() {
    name_ = "MeshRenderer";
}

void MeshRenderer::init() {
    // Find required components
    meshComponent_ = entity_->getComponent<MeshComponent>();
    if (!meshComponent_) {
        gl::logWarning("MeshRenderer requires a MeshComponent on the same entity");
    }

    // Find camera in scene
    if (entity_->getScene()) {
        auto cameraEntity = entity_->getScene()->findEntity("MainCamera");
        if (cameraEntity) {
            cameraComponent_ = cameraEntity->getComponent<CameraComponent>();
        }
    }

    if (!cameraComponent_) {
        gl::logWarning("MeshRenderer couldn't find a CameraComponent in the scene");
    }

    gl::logDebug("MeshRenderer initialized");
}

void MeshRenderer::update(float deltaTime) {
    // Nothing to update here
}

void MeshRenderer::render() {
    if (!meshComponent_ || !shader_) {
        return;
    }

    shader_->use();

    // Calculate MVP matrix based on flags
    glm::mat4 mvp;

    if (useExternalModelMatrix_) {
        if (useMVPDirectly_) {
            // Use the provided matrix directly as MVP (for screen-space rendering)
            mvp = modelMatrix_;
        }
        else {
            // Get view and projection matrices from camera
            glm::mat4 view = glm::mat4(1.0f);
            glm::mat4 projection = glm::mat4(1.0f);

            if (cameraComponent_) {
                view = cameraComponent_->getViewMatrix();
                projection = cameraComponent_->getProjectionMatrix();
            }

            // Calculate MVP with camera transformation
            mvp = projection * view * modelMatrix_;
        }
    }
    else {
        // Use model matrix from mesh component
        glm::mat4 model = meshComponent_->getModelMatrix();

        // Get view and projection matrices from camera
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        if (cameraComponent_) {
            view = cameraComponent_->getViewMatrix();
            projection = cameraComponent_->getProjectionMatrix();
        }

        // Calculate MVP
        mvp = projection * view * model;
    }

    // Set uniforms
    shader_->setMat4("u_MVP", glm::value_ptr(mvp));

    // Set texture if available
    if (texture_) {
        texture_->bind(0);
        shader_->setInt("texture1", 0);
    }

    // Draw mesh
    meshComponent_->getVAO()->bind();

    if (meshComponent_->hasIndices()) {
        glDrawElements(GL_TRIANGLES, meshComponent_->getIndexCount(), GL_UNSIGNED_INT, 0);
    }
    else {
        glDrawArrays(GL_TRIANGLES, 0, meshComponent_->getVertexCount());
    }

    meshComponent_->getVAO()->unbind();
}