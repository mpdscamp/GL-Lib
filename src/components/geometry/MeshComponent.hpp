#ifndef MESH_COMPONENT_HPP
#define MESH_COMPONENT_HPP
#include "../../core/Component.hpp"
#include "../../../include/gl/buffer.hpp"
#include "../../../include/gl/vertex_array.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoords;
    glm::vec3 normal;
};

class MeshComponent : public Component {
public:
    MeshComponent();
    ~MeshComponent();
    void init() override;

    // Create predefined shapes
    void createCube();
    void createQuad();

    // Custom geometry
    void setVertices(const std::vector<Vertex>& vertices);
    void setIndices(const std::vector<unsigned int>& indices);

    // Manual geometry with raw data
    void setPositionsAndTexCoords(const std::vector<float>& data, int stride, int posOffset, int texOffset);

    // Transformation methods
    void setPosition(const glm::vec3& position) { position_ = position; transformDirty_ = true; }
    void setRotation(float angle, const glm::vec3& axis) {
        rotationAngle_ = angle;
        rotationAxis_ = axis;
        transformDirty_ = true;
    }
    void setScale(const glm::vec3& scale) { scale_ = scale; transformDirty_ = true; }

    // Getter methods for transform properties
    const glm::vec3& getPosition() const { return position_; }
    float getRotationAngle() const { return rotationAngle_; }
    const glm::vec3& getRotationAxis() const { return rotationAxis_; }
    const glm::vec3& getScale() const { return scale_; }

    // Animation
    void animate(float deltaTime);
    void setAutoRotate(bool autoRotate) { autoRotate_ = autoRotate; }
    void setRotationSpeed(float speed) { rotationSpeed_ = speed; }
    bool isAutoRotating() const { return autoRotate_; }
    float getRotationSpeed() const { return rotationSpeed_; }

    // Access
    gl::VertexArray* getVAO() const { return vao_.get(); }
    int getVertexCount() const { return vertexCount_; }
    int getIndexCount() const { return indexCount_; }
    bool hasIndices() const { return indexCount_ > 0; }
    glm::mat4 getModelMatrix();

private:
    void updateTransform();
    std::unique_ptr<gl::VertexArray> vao_;
    std::unique_ptr<gl::VertexBuffer> vbo_;
    std::unique_ptr<gl::ElementBuffer> ebo_;
    int vertexCount_ = 0;
    int indexCount_ = 0;

    // Transform data
    glm::vec3 position_ = glm::vec3(0.0f);
    float rotationAngle_ = 0.0f;
    glm::vec3 rotationAxis_ = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 scale_ = glm::vec3(1.0f);

    // Animation
    bool autoRotate_ = false;
    float rotationSpeed_ = 30.0f; // Degrees per second

    // Caching
    bool transformDirty_ = true;
    glm::mat4 modelMatrix_ = glm::mat4(1.0f);
};
#endif // MESH_COMPONENT_HPP