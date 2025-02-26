#ifndef GL_COMMON_HPP
#define GL_COMMON_HPP

#include <glad/glad.h>
#include <string>
#include <stdexcept>

namespace gl {

    // Common buffer types
    enum class BufferType {
        Vertex = GL_ARRAY_BUFFER,
        Element = GL_ELEMENT_ARRAY_BUFFER,
        Uniform = GL_UNIFORM_BUFFER,
        ShaderStorage = GL_SHADER_STORAGE_BUFFER,
        PixelPack = GL_PIXEL_PACK_BUFFER,
        PixelUnpack = GL_PIXEL_UNPACK_BUFFER,
        TransformFeedback = GL_TRANSFORM_FEEDBACK_BUFFER,
        AtomicCounter = GL_ATOMIC_COUNTER_BUFFER,
        DispatchIndirect = GL_DISPATCH_INDIRECT_BUFFER,
        DrawIndirect = GL_DRAW_INDIRECT_BUFFER,
        Query = GL_QUERY_BUFFER,
        Texture = GL_TEXTURE_BUFFER
    };

    // Buffer usage hints
    enum class BufferUsage {
        StaticDraw = GL_STATIC_DRAW,
        DynamicDraw = GL_DYNAMIC_DRAW,
        StreamDraw = GL_STREAM_DRAW,
        StaticRead = GL_STATIC_READ,
        DynamicRead = GL_DYNAMIC_READ,
        StreamRead = GL_STREAM_READ,
        StaticCopy = GL_STATIC_COPY,
        DynamicCopy = GL_DYNAMIC_COPY,
        StreamCopy = GL_STREAM_COPY
    };

    // Data types for vertex attributes
    enum class DataType {
        Byte = GL_BYTE,
        UnsignedByte = GL_UNSIGNED_BYTE,
        Short = GL_SHORT,
        UnsignedShort = GL_UNSIGNED_SHORT,
        Int = GL_INT,
        UnsignedInt = GL_UNSIGNED_INT,
        HalfFloat = GL_HALF_FLOAT,
        Float = GL_FLOAT,
        Double = GL_DOUBLE,
        Fixed = GL_FIXED
    };

    // Draw modes
    enum class DrawMode {
        Points = GL_POINTS,
        LineStrip = GL_LINE_STRIP,
        LineLoop = GL_LINE_LOOP,
        Lines = GL_LINES,
        LineStripAdjacency = GL_LINE_STRIP_ADJACENCY,
        LinesAdjacency = GL_LINES_ADJACENCY,
        TriangleStrip = GL_TRIANGLE_STRIP,
        TriangleFan = GL_TRIANGLE_FAN,
        Triangles = GL_TRIANGLES,
        TriangleStripAdjacency = GL_TRIANGLE_STRIP_ADJACENCY,
        TrianglesAdjacency = GL_TRIANGLES_ADJACENCY,
        Patches = GL_PATCHES
    };

    // OpenGL capability flags
    enum class Capability {
        Blend = GL_BLEND,
        CullFace = GL_CULL_FACE,
        DepthTest = GL_DEPTH_TEST,
        Dither = GL_DITHER,
        PolygonOffsetFill = GL_POLYGON_OFFSET_FILL,
        SampleAlphaToCoverage = GL_SAMPLE_ALPHA_TO_COVERAGE,
        SampleCoverage = GL_SAMPLE_COVERAGE,
        ScissorTest = GL_SCISSOR_TEST,
        StencilTest = GL_STENCIL_TEST,
        Multisample = GL_MULTISAMPLE
    };

    // Exception class for general OpenGL errors
    class GLException : public std::runtime_error {
    public:
        explicit GLException(const std::string& message)
            : std::runtime_error("GL Error: " + message) {}
    };

    // Helper utilities

    // Check for OpenGL errors and throw if any are found
    inline void checkError(const std::string& operation) {
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::string errorString;
            switch (error) {
            case GL_INVALID_ENUM: errorString = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE: errorString = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: errorString = "GL_INVALID_OPERATION"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: errorString = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
            case GL_OUT_OF_MEMORY: errorString = "GL_OUT_OF_MEMORY"; break;
            case GL_STACK_UNDERFLOW: errorString = "GL_STACK_UNDERFLOW"; break;
            case GL_STACK_OVERFLOW: errorString = "GL_STACK_OVERFLOW"; break;
            default: errorString = "Unknown error (" + std::to_string(error) + ")";
            }
            throw GLException(operation + " failed: " + errorString);
        }
    }

    // Enable/disable OpenGL capabilities
    inline void enable(Capability cap) {
        glEnable(static_cast<GLenum>(cap));
    }

    inline void disable(Capability cap) {
        glDisable(static_cast<GLenum>(cap));
    }

} // namespace gl

#endif // GL_COMMON_HPP