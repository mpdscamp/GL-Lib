#ifndef GL_VERTEX_ARRAY_HPP
#define GL_VERTEX_ARRAY_HPP

#include "common.hpp"
#include "buffer.hpp"
#include <vector>

namespace gl {

    // Vertex array object (VAO)
    class VertexArray {
    public:
        VertexArray() {
            glGenVertexArrays(1, &id_);
            if (id_ == 0) {
                throw GLException("Failed to create vertex array object");
            }
        }

        ~VertexArray() {
            if (id_ != 0) {
                glDeleteVertexArrays(1, &id_);
            }
        }

        // Prevent copying
        VertexArray(const VertexArray&) = delete;
        VertexArray& operator=(const VertexArray&) = delete;

        // Allow moving
        VertexArray(VertexArray&& other) noexcept : id_(other.id_) {
            other.id_ = 0;
        }

        VertexArray& operator=(VertexArray&& other) noexcept {
            if (this != &other) {
                if (id_ != 0) {
                    glDeleteVertexArrays(1, &id_);
                }
                id_ = other.id_;
                other.id_ = 0;
            }
            return *this;
        }

        void bind() const {
            glBindVertexArray(id_);
        }

        void unbind() const {
            glBindVertexArray(0);
        }

        // Attach element buffer to this VAO
        void attachElementBuffer(const ElementBuffer& ebo) {
            bind();
            ebo.bind();
            // Note: Do not unbind the EBO here - it's stored in VAO state
        }

        // Configure a vertex attribute
        void setVertexAttribute(
            GLuint index,
            GLint size,
            DataType type,
            bool normalized,
            GLsizei stride,
            size_t offset
        ) {
            bind();
            glVertexAttribPointer(
                index,
                size,
                static_cast<GLenum>(type),
                normalized ? GL_TRUE : GL_FALSE,
                stride,
                reinterpret_cast<const void*>(offset)
            );
            glEnableVertexAttribArray(index);
        }

        // Configure an integer vertex attribute
        void setVertexAttributeI(
            GLuint index,
            GLint size,
            DataType type,
            GLsizei stride,
            size_t offset
        ) {
            bind();
            glVertexAttribIPointer(
                index,
                size,
                static_cast<GLenum>(type),
                stride,
                reinterpret_cast<const void*>(offset)
            );
            glEnableVertexAttribArray(index);
        }

        // Configure a double-precision vertex attribute
        void setVertexAttributeL(
            GLuint index,
            GLint size,
            DataType type,
            GLsizei stride,
            size_t offset
        ) {
            bind();
            glVertexAttribLPointer(
                index,
                size,
                static_cast<GLenum>(type),
                stride,
                reinterpret_cast<const void*>(offset)
            );
            glEnableVertexAttribArray(index);
        }

        // Set attribute divisor for instanced rendering
        void setAttributeDivisor(GLuint index, GLuint divisor) {
            bind();
            glVertexAttribDivisor(index, divisor);
        }

        // Helper methods for common attribute types

        // Float attributes (positions, normals, colors, etc.)
        void setFloatAttribute(
            GLuint index,
            GLint components,
            GLsizei stride,
            size_t offset,
            bool normalized = false
        ) {
            setVertexAttribute(index, components, DataType::Float, normalized, stride, offset);
        }

        // Integer attributes (indices, IDs, etc.)
        void setIntAttribute(
            GLuint index,
            GLint components,
            GLsizei stride,
            size_t offset
        ) {
            setVertexAttributeI(index, components, DataType::Int, stride, offset);
        }

        // Draw methods

        // Draw arrays (when not using an element buffer)
        void drawArrays(
            DrawMode mode,
            GLint first,
            GLsizei count
        ) const {
            bind();
            glDrawArrays(static_cast<GLenum>(mode), first, count);
        }

        // Draw elements (when using an element buffer)
        void drawElements(
            DrawMode mode,
            GLsizei count,
            DataType type = DataType::UnsignedInt,
            size_t offset = 0
        ) const {
            bind();
            glDrawElements(
                static_cast<GLenum>(mode),
                count,
                static_cast<GLenum>(type),
                reinterpret_cast<const void*>(offset)
            );
        }

        // Draw instanced elements
        void drawElementsInstanced(
            DrawMode mode,
            GLsizei count,
            GLsizei instanceCount,
            DataType type = DataType::UnsignedInt,
            size_t offset = 0
        ) const {
            bind();
            glDrawElementsInstanced(
                static_cast<GLenum>(mode),
                count,
                static_cast<GLenum>(type),
                reinterpret_cast<const void*>(offset),
                instanceCount
            );
        }

        // Draw instanced arrays
        void drawArraysInstanced(
            DrawMode mode,
            GLint first,
            GLsizei count,
            GLsizei instanceCount
        ) const {
            bind();
            glDrawArraysInstanced(
                static_cast<GLenum>(mode),
                first,
                count,
                instanceCount
            );
        }

        GLuint getId() const { return id_; }

    private:
        GLuint id_ = 0;
    };

} // namespace gl

#endif // GL_VERTEX_ARRAY_HPP