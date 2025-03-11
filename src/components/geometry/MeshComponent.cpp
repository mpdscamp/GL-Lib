#include "MeshComponent.hpp"
#include "../../gl/logger.hpp"
#include <glm/gtc/matrix_transform.hpp>

MeshComponent::MeshComponent()
    : vao_(std::make_unique<gl::VertexArray>()),
    vbo_(std::make_unique<gl::VertexBuffer>()),
    ebo_(std::make_unique<gl::ElementBuffer>())
{
    name_ = "MeshComponent";
}

MeshComponent::~MeshComponent() {
    // Smart pointers handle cleanup
}

void MeshComponent::init() {
    // Already initialized in constructor
}

void MeshComponent::createCube() {
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

    setPositionsAndTexCoords(std::vector<float>(cubeVertices, cubeVertices + sizeof(cubeVertices) / sizeof(float)),
        5 * sizeof(float), 0, 3 * sizeof(float));

    vertexCount_ = 36;
    indexCount_ = 0;

    gl::logDebug("Cube mesh created");
}

void MeshComponent::createQuad() {
    float quadVertices[] = {
        // positions         // texture coords
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f
    };

    setPositionsAndTexCoords(std::vector<float>(quadVertices, quadVertices + sizeof(quadVertices) / sizeof(float)),
        5 * sizeof(float), 0, 3 * sizeof(float));

    vertexCount_ = 6;
    indexCount_ = 0;

    gl::logDebug("Quad mesh created");
}

void MeshComponent::setVertices(const std::vector<Vertex>& vertices) {
    vao_->bind();
    vbo_->bind();
    vbo_->setData(vertices, gl::BufferUsage::StaticDraw);

    // Position attribute
    vao_->setVertexAttribute(0, 3, gl::DataType::Float, false, sizeof(Vertex), offsetof(Vertex, position));
    // Texture coord attribute
    vao_->setVertexAttribute(1, 2, gl::DataType::Float, false, sizeof(Vertex), offsetof(Vertex, texCoords));
    // Normal attribute
    vao_->setVertexAttribute(2, 3, gl::DataType::Float, false, sizeof(Vertex), offsetof(Vertex, normal));

    vbo_->unbind();
    vao_->unbind();

    vertexCount_ = static_cast<int>(vertices.size());
    indexCount_ = 0;
}

void MeshComponent::setIndices(const std::vector<unsigned int>& indices) {
    vao_->bind();
    ebo_->bind();
    ebo_->setIndices(indices, gl::BufferUsage::StaticDraw);
    vao_->unbind();

    indexCount_ = static_cast<int>(indices.size());
}

void MeshComponent::setPositionsAndTexCoords(const std::vector<float>& data, int stride, int posOffset, int texOffset) {
    vao_->bind();
    vbo_->bind();
    vbo_->setData(data, gl::BufferUsage::StaticDraw);

    // Position attribute
    vao_->setVertexAttribute(0, 3, gl::DataType::Float, false, stride, posOffset);
    // Texture coordinate attribute
    vao_->setVertexAttribute(1, 2, gl::DataType::Float, false, stride, texOffset);

    vbo_->unbind();
    vao_->unbind();

    vertexCount_ = static_cast<int>(data.size() * sizeof(float) / stride);
    indexCount_ = 0;
}

void MeshComponent::animate(float deltaTime) {
    if (autoRotate_) {
        rotationAngle_ += deltaTime * rotationSpeed_;
        if (rotationAngle_ >= 360.0f) {
            rotationAngle_ -= 360.0f;
        }
        transformDirty_ = true;
    }
}

glm::mat4 MeshComponent::getModelMatrix() {
    if (transformDirty_) {
        updateTransform();
    }
    return modelMatrix_;
}

void MeshComponent::updateTransform() {
    modelMatrix_ = glm::mat4(1.0f);

    // Apply transformations in order: scale, rotate, translate
    modelMatrix_ = glm::translate(modelMatrix_, position_);
    modelMatrix_ = glm::rotate(modelMatrix_, glm::radians(rotationAngle_), rotationAxis_);
    modelMatrix_ = glm::scale(modelMatrix_, scale_);

    transformDirty_ = false;
}