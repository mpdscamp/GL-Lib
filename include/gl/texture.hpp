#ifndef GL_TEXTURE_HPP
#define GL_TEXTURE_HPP

#include "common.hpp"
#include <stb_image.h>
#include <string>

namespace gl {

    enum class TextureType {
        Texture1D = GL_TEXTURE_1D,
        Texture2D = GL_TEXTURE_2D,
        Texture3D = GL_TEXTURE_3D,
        Texture1DArray = GL_TEXTURE_1D_ARRAY,
        Texture2DArray = GL_TEXTURE_2D_ARRAY,
        TextureCubeMap = GL_TEXTURE_CUBE_MAP,
        TextureCubeMapArray = GL_TEXTURE_CUBE_MAP_ARRAY
    };

    enum class TextureWrap {
        Repeat = GL_REPEAT,
        MirroredRepeat = GL_MIRRORED_REPEAT,
        ClampToEdge = GL_CLAMP_TO_EDGE,
        ClampToBorder = GL_CLAMP_TO_BORDER
    };

    enum class TextureFilter {
        Nearest = GL_NEAREST,
        Linear = GL_LINEAR,
        NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
        LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
        NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,
        LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR
    };

    enum class TextureFormat {
        RGB = GL_RGB,
        RGBA = GL_RGBA,
        Red = GL_RED,
        RG = GL_RG,
        BGR = GL_BGR,
        BGRA = GL_BGRA,
        DepthComponent = GL_DEPTH_COMPONENT,
        StencilIndex = GL_STENCIL_INDEX
    };

    class Texture {
    public:
        Texture(TextureType type = TextureType::Texture2D)
            : type_(type) {
            glGenTextures(1, &id_);
            if (id_ == 0) {
                throw GLException("Failed to create texture");
            }
        }

        ~Texture() {
            if (id_ != 0) {
                glDeleteTextures(1, &id_);
            }
        }

        // Prevent copying
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        // Allow moving
        Texture(Texture&& other) noexcept
            : id_(other.id_), type_(other.type_), width_(other.width_), height_(other.height_) {
            other.id_ = 0;
        }

        Texture& operator=(Texture&& other) noexcept {
            if (this != &other) {
                if (id_ != 0) {
                    glDeleteTextures(1, &id_);
                }
                id_ = other.id_;
                type_ = other.type_;
                width_ = other.width_;
                height_ = other.height_;
                other.id_ = 0;
            }
            return *this;
        }

        void bind(GLuint unit = 0) const {
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(static_cast<GLenum>(type_), id_);
        }

        void unbind() const {
            glBindTexture(static_cast<GLenum>(type_), 0);
        }

        // Load a 2D texture from file using stb_image
        bool loadFromFile(const std::string& path, bool generateMipmap = true, bool flipVertically = true) {
            if (type_ != TextureType::Texture2D) {
                throw GLException("Loading from file only supported for Texture2D");
            }

            // stb_image loads from top to bottom by default, but OpenGL expects from bottom to top
            stbi_set_flip_vertically_on_load(flipVertically);

            int width, height, channels;
            unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
            if (!data) {
                return false;
            }

            TextureFormat format;
            switch (channels) {
            case 1: format = TextureFormat::Red; break;
            case 3: format = TextureFormat::RGB; break;
            case 4: format = TextureFormat::RGBA; break;
            default:
                stbi_image_free(data);
                throw GLException("Unsupported number of channels in texture");
            }

            width_ = width;
            height_ = height;

            bind();
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                static_cast<GLint>(format),
                width,
                height,
                0,
                static_cast<GLenum>(format),
                GL_UNSIGNED_BYTE,
                data
            );

            if (generateMipmap) {
                glGenerateMipmap(GL_TEXTURE_2D);
            }

            // Free the image memory
            stbi_image_free(data);

            return true;
        }

        // Set texture wrapping options
        void setWrapParameters(TextureWrap s, TextureWrap t, TextureWrap r = TextureWrap::Repeat) {
            bind();
            glTexParameteri(static_cast<GLenum>(type_), GL_TEXTURE_WRAP_S, static_cast<GLint>(s));
            glTexParameteri(static_cast<GLenum>(type_), GL_TEXTURE_WRAP_T, static_cast<GLint>(t));

            if (type_ == TextureType::Texture3D || type_ == TextureType::TextureCubeMap ||
                type_ == TextureType::Texture2DArray || type_ == TextureType::TextureCubeMapArray) {
                glTexParameteri(static_cast<GLenum>(type_), GL_TEXTURE_WRAP_R, static_cast<GLint>(r));
            }
        }

        // Set texture filtering options
        void setFilterParameters(TextureFilter minFilter, TextureFilter magFilter) {
            bind();
            glTexParameteri(static_cast<GLenum>(type_), GL_TEXTURE_MIN_FILTER, static_cast<GLint>(minFilter));
            glTexParameteri(static_cast<GLenum>(type_), GL_TEXTURE_MAG_FILTER, static_cast<GLint>(magFilter));
        }

        // Set border color (for GL_CLAMP_TO_BORDER)
        void setBorderColor(float r, float g, float b, float a) {
            bind();
            float borderColor[] = { r, g, b, a };
            glTexParameterfv(static_cast<GLenum>(type_), GL_TEXTURE_BORDER_COLOR, borderColor);
        }

        GLuint getId() const { return id_; }
        TextureType getType() const { return type_; }
        int getWidth() const { return width_; }
        int getHeight() const { return height_; }

    private:
        GLuint id_ = 0;
        TextureType type_;
        int width_ = 0;
        int height_ = 0;
    };

} // namespace gl

#endif // GL_TEXTURE_HPP