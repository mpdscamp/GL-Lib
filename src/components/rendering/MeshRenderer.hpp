#ifndef MESH_RENDERER_HPP
#define MESH_RENDERER_HPP

#include "../../core/Component.hpp"
#include "../../../include/gl/shader.hpp"
#include "../../../include/gl/texture.hpp"
#include <memory>
#include <glm/glm.hpp>

class MeshComponent;
class CameraComponent;

class MeshRenderer : public Component {
public:
    MeshRenderer();

    void init() override;
    void update(float deltaTime) override;
    void render() override;

    void setShader(std::shared_ptr<gl::Shader> shader) { shader_ = shader; }
    void setTexture(std::shared_ptr<gl::Texture> texture) { texture_ = texture; }

    // Standard model matrix - will be transformed by camera view/projection
    void setModelMatrix(const glm::mat4& modelMatrix) {
        useExternalModelMatrix_ = true;
        useMVPDirectly_ = false;
        modelMatrix_ = modelMatrix;
    }

    // Direct MVP matrix - will be used as-is without camera transformations
    void setMVPMatrix(const glm::mat4& mvpMatrix) {
        useExternalModelMatrix_ = true;
        useMVPDirectly_ = true;
        modelMatrix_ = mvpMatrix; // Store the MVP in modelMatrix_ field
    }

    void clearModelMatrix() {
        useExternalModelMatrix_ = false;
        useMVPDirectly_ = false;
    }

    std::shared_ptr<gl::Shader> getShader() const { return shader_; }

private:
    CameraComponent* cameraComponent_ = nullptr;
    MeshComponent* meshComponent_ = nullptr;

    std::shared_ptr<gl::Shader> shader_;
    std::shared_ptr<gl::Texture> texture_;

    bool useExternalModelMatrix_ = false;
    bool useMVPDirectly_ = false;  // New flag for direct MVP usage
    glm::mat4 modelMatrix_ = glm::mat4(1.0f);
};

#endif // MESH_RENDERER_HPP