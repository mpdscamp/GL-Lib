#include "PostProcessor.hpp"
#include "../geometry/MeshComponent.hpp"
#include "../../core/Entity.hpp"
#include "../../core/Scene.hpp"
#include "../../window/Window.hpp"
#include "../../gl/logger.hpp"
#include <glad/glad.h>

PostProcessor::PostProcessor() {
    name_ = "PostProcessor";
}

void PostProcessor::init() {
    // Create quad mesh if needed
    if (!entity_->getComponent<MeshComponent>()) {
        quadMesh_ = entity_->addComponent<MeshComponent>();
        quadMesh_->createQuad();
    }
    else {
        quadMesh_ = entity_->getComponent<MeshComponent>();
    }

    // Create framebuffer
    if (entity_->getScene()) {
        Window& window = entity_->getScene()->getWindow();
        framebuffer_ = std::make_unique<gl::FrameBuffer>(window.getWidth(), window.getHeight());
    }
    else {
        framebuffer_ = std::make_unique<gl::FrameBuffer>(800, 600);
    }

    gl::logDebug("PostProcessor initialized");
}

void PostProcessor::update(float deltaTime) {
    // Nothing to update
}

void PostProcessor::render() {
    // This is called during regular scene rendering
    // The actual post-processing is done in endRender()
}

void PostProcessor::beginRender() {
    if (!enabled_ || !framebuffer_) {
        return;
    }

    framebuffer_->bind();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcessor::endRender() {
    if (!enabled_ || !framebuffer_ || !shader_ || !quadMesh_) {
        return;
    }

    framebuffer_->unbind();

    // Clear the default framebuffer
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Disable depth testing for post-processing pass
    glDisable(GL_DEPTH_TEST);

    // Use post-processing shader
    shader_->use();
    shader_->setInt("effect", currentEffect_);

    // Bind framebuffer texture
    framebuffer_->getColorTexture()->bind(0);
    shader_->setInt("screenTexture", 0);

    // Render a full-screen quad
    quadMesh_->getVAO()->bind();
    glDrawArrays(GL_TRIANGLES, 0, quadMesh_->getVertexCount());
    quadMesh_->getVAO()->unbind();

    // Re-enable depth testing
    glEnable(GL_DEPTH_TEST);
}

void PostProcessor::onWindowResize(int width, int height) {
    if (framebuffer_) {
        framebuffer_->resize(width, height);
        gl::logDebug("PostProcessor framebuffer resized to " + std::to_string(width) + "x" + std::to_string(height));
    }
}

void PostProcessor::nextEffect() {
    currentEffect_ = (currentEffect_ + 1) % numEffects_;
    gl::logInfo("Post-processing effect: " + std::to_string(currentEffect_));
}

void PostProcessor::previousEffect() {
    currentEffect_ = (currentEffect_ - 1 + numEffects_) % numEffects_;
    gl::logInfo("Post-processing effect: " + std::to_string(currentEffect_));
}