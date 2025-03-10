#ifndef GL_FRAMEBUFFER_HPP
#define GL_FRAMEBUFFER_HPP

#include "common.hpp"
#include "texture.hpp"
#include <memory>

namespace gl {

    class FrameBuffer {
    public:
        FrameBuffer(int width, int height)
            : width_(width), height_(height) {
            glGenFramebuffers(1, &id_);
            if (id_ == 0) {
                throw GLException("Failed to create FrameBuffer");
            }

            // Create a color attachment texture
            colorTexture_ = std::make_unique<Texture>(TextureType::Texture2D);
            colorTexture_->bind();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                GL_UNSIGNED_BYTE, nullptr);
            colorTexture_->setFilterParameters(TextureFilter::Linear, TextureFilter::Linear);

            // Create a renderbuffer for depth and stencil
            glGenRenderbuffers(1, &rbo_);
            glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

            // Attach to FrameBuffer
            bind();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D, colorTexture_->getId(), 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                GL_RENDERBUFFER, rbo_);

            // Check if FrameBuffer is complete
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                throw GLException("FrameBuffer is not complete");
            }
            unbind();
        }

        ~FrameBuffer() {
            cleanup();
        }

        // Prevent copying
        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer& operator=(const FrameBuffer&) = delete;

        // Allow moving
        FrameBuffer(FrameBuffer&& other) noexcept
            : id_(other.id_), rbo_(other.rbo_),
            width_(other.width_), height_(other.height_),
            colorTexture_(std::move(other.colorTexture_)) {
            other.id_ = 0;
            other.rbo_ = 0;
        }

        FrameBuffer& operator=(FrameBuffer&& other) noexcept {
            if (this != &other) {
                cleanup();

                id_ = other.id_;
                rbo_ = other.rbo_;
                width_ = other.width_;
                height_ = other.height_;
                colorTexture_ = std::move(other.colorTexture_);

                other.id_ = 0;
                other.rbo_ = 0;
            }
            return *this;
        }

        void bind() const {
            glBindFramebuffer(GL_FRAMEBUFFER, id_);
            glViewport(0, 0, width_, height_);
        }

        void unbind() const {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void resize(int width, int height) {
            if (width_ == width && height_ == height) {
                return;
            }

            width_ = width;
            height_ = height;

            // Recreate color attachment
            colorTexture_->bind();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                GL_UNSIGNED_BYTE, nullptr);

            // Recreate depth-stencil renderbuffer
            glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

            // Check completeness
            bind();
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                throw GLException("FrameBuffer is not complete after resize");
            }
            unbind();
        }

        GLuint getId() const { return id_; }
        Texture* getColorTexture() const { return colorTexture_.get(); }
        int getWidth() const { return width_; }
        int getHeight() const { return height_; }

    private:
        GLuint id_ = 0;
        GLuint rbo_ = 0;
        int width_, height_;
        std::unique_ptr<Texture> colorTexture_;

        void cleanup() {
            if (rbo_ != 0) {
                glDeleteRenderbuffers(1, &rbo_);
                rbo_ = 0;
            }
            if (id_ != 0) {
                glDeleteFramebuffers(1, &id_);
                id_ = 0;
            }
            // colorTexture_ is cleaned up by its own destructor
        }
    };

} // namespace gl

#endif // GL_FRAMEBUFFER_HPP