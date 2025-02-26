#ifndef GL_BUFFER_HPP
#define GL_BUFFER_HPP

#include "common.hpp"
#include <vector>
#include <memory>

namespace gl {

    // Forward declaration for VertexArray
    class VertexArray;

    // Base Buffer class
    class Buffer {
    public:
        Buffer(BufferType type)
            : type_(type) {
            glGenBuffers(1, &id_);
            if (id_ == 0) {
                throw GLException("Failed to create OpenGL buffer");
            }
        }

        virtual ~Buffer() {
            if (id_ != 0) {
                glDeleteBuffers(1, &id_);
            }
        }

        // Prevent copying
        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        // Allow moving
        Buffer(Buffer&& other) noexcept
            : id_(other.id_), type_(other.type_) {
            other.id_ = 0;
        }

        Buffer& operator=(Buffer&& other) noexcept {
            if (this != &other) {
                if (id_ != 0) {
                    glDeleteBuffers(1, &id_);
                }
                id_ = other.id_;
                type_ = other.type_;
                other.id_ = 0;
            }
            return *this;
        }

        void bind() const {
            glBindBuffer(static_cast<GLenum>(type_), id_);
        }

        void unbind() const {
            glBindBuffer(static_cast<GLenum>(type_), 0);
        }

        // Set buffer data from a vector
        template<typename T>
        void setData(const std::vector<T>& data, BufferUsage usage = BufferUsage::StaticDraw) {
            bind();
            glBufferData(
                static_cast<GLenum>(type_),
                data.size() * sizeof(T),
                data.data(),
                static_cast<GLenum>(usage)
            );
        }

        // Set buffer data from a raw array
        template<typename T>
        void setData(const T* data, size_t count, BufferUsage usage = BufferUsage::StaticDraw) {
            bind();
            glBufferData(
                static_cast<GLenum>(type_),
                count * sizeof(T),
                data,
                static_cast<GLenum>(usage)
            );
        }

        // Update a subset of buffer data
        template<typename T>
        void updateSubData(const std::vector<T>& data, size_t offset = 0) {
            bind();
            glBufferSubData(
                static_cast<GLenum>(type_),
                offset,
                data.size() * sizeof(T),
                data.data()
            );
        }

        // Get buffer size
        GLint getSize() const {
            GLint size = 0;
            bind();
            glGetBufferParameteriv(static_cast<GLenum>(type_), GL_BUFFER_SIZE, &size);
            return size;
        }

        // Map buffer to client memory (advanced usage)
        template<typename T>
        T* mapRange(GLintptr offset, GLsizeiptr length, GLbitfield access) {
            bind();
            return static_cast<T*>(glMapBufferRange(
                static_cast<GLenum>(type_),
                offset,
                length,
                access
            ));
        }

        // Unmap buffer
        bool unmap() {
            bind();
            return glUnmapBuffer(static_cast<GLenum>(type_));
        }

        // Getters
        GLuint getId() const { return id_; }
        BufferType getType() const { return type_; }

    protected:
        GLuint id_ = 0;
        BufferType type_;

        friend class VertexArray;
    };

    // Specialized buffer types

    // Vertex buffer (positions, colors, UVs, etc.)
    class VertexBuffer : public Buffer {
    public:
        VertexBuffer() : Buffer(BufferType::Vertex) {}
    };

    // Element buffer (indices)
    class ElementBuffer : public Buffer {
    public:
        ElementBuffer() : Buffer(BufferType::Element) {}

        // Helper methods specific to index buffers
        template<typename T>
        void setIndices(const std::vector<T>& indices, BufferUsage usage = BufferUsage::StaticDraw) {
            setData(indices, usage);
        }

        // Get number of indices based on index type
        GLsizei getCount(DataType indexType = DataType::UnsignedInt) const {
            GLint typeSize;
            switch (indexType) {
            case DataType::UnsignedByte: typeSize = 1; break;
            case DataType::UnsignedShort: typeSize = 2; break;
            case DataType::UnsignedInt: typeSize = 4; break;
            default: throw GLException("Invalid index type");
            }
            return getSize() / typeSize;
        }
    };

    // Uniform buffer (for sharing uniforms between shaders)
    class UniformBuffer : public Buffer {
    public:
        UniformBuffer() : Buffer(BufferType::Uniform) {}

        // Bind to a specific binding point
        void bindBase(GLuint bindingPoint) const {
            glBindBufferBase(static_cast<GLenum>(type_), bindingPoint, id_);
        }

        // Bind a range to a specific binding point
        void bindRange(GLuint bindingPoint, GLintptr offset, GLsizeiptr size) const {
            glBindBufferRange(static_cast<GLenum>(type_), bindingPoint, id_, offset, size);
        }
    };

    // Storage buffer (for compute shaders)
    class ShaderStorageBuffer : public Buffer {
    public:
        ShaderStorageBuffer() : Buffer(BufferType::ShaderStorage) {}

        // Bind to a specific binding point
        void bindBase(GLuint bindingPoint) const {
            glBindBufferBase(static_cast<GLenum>(type_), bindingPoint, id_);
        }

        // Bind a range to a specific binding point
        void bindRange(GLuint bindingPoint, GLintptr offset, GLsizeiptr size) const {
            glBindBufferRange(static_cast<GLenum>(type_), bindingPoint, id_, offset, size);
        }
    };

} // namespace gl

#endif // GL_BUFFER_HPP