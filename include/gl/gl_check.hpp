#ifndef GL_CHECK_HPP
#define GL_CHECK_HPP

#include "common.hpp"
#include "logger.hpp"
#include <string>
#include <source_location>

namespace gl {

    // Enhanced error checking with source location information
    inline void checkError(const char* operation, const char* file, int line) {
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

            std::string message = std::string(operation) + " failed with " +
                errorString + " at " + file + ":" + std::to_string(line);

            // Log the error before throwing the exception
            logError(message);

            throw GLException(message);
        }
    }

    // C++20 version with source_location
    inline void checkError(const char* operation,
        const std::source_location& location = std::source_location::current()) {
        checkError(operation, location.file_name(), location.line());
    }

    // Macro for automatic error checking
#define GL_CHECK(x) do { \
    (x); \
    gl::checkError(#x, __FILE__, __LINE__); \
} while(0)

} // namespace gl

#endif // GL_CHECK_HPP