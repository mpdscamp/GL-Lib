#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/glad.h>
#include "shader_error.hpp"
#include "logger.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <filesystem>

namespace gl {

    class Shader {
    public:
        Shader(const char* vertexPath, const char* fragmentPath) {
            try {
                // Read shader source files
                std::string vertexCode = readFile(vertexPath);
                std::string fragmentCode = readFile(fragmentPath);

                // Compile shaders
                GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexCode);
                GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentCode);

                // Link shaders to the program
                ID = createAndLinkProgram(vertex, fragment);

                // Delete shaders as they're linked into the program and are no longer needed
                glDeleteShader(vertex);
                glDeleteShader(fragment);
            }
            catch (const std::exception&) {
                if (ID != 0) {
                    glDeleteProgram(ID);
                    ID = 0;
                }
                throw;
            }
        }

        ~Shader() {
            if (ID != 0) {
                glDeleteProgram(ID);
            }
        }

        // Prevent copying
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        // Allow moving
        Shader(Shader&& other) noexcept : ID(other.ID), uniformLocationCache(std::move(other.uniformLocationCache)) {
            other.ID = 0;
        }

        Shader& operator=(Shader&& other) noexcept {
            if (this != &other) {
                if (ID != 0) {
                    glDeleteProgram(ID);
                }
                ID = other.ID;
                uniformLocationCache = std::move(other.uniformLocationCache);
                other.ID = 0;
            }
            return *this;
        }

        void use() const {
            glUseProgram(ID);
        }

        // Utility uniform functions
        void setBool(const std::string& name, bool value) const {
            glUniform1i(getUniformLocation(name), static_cast<int>(value));
        }

        void setInt(const std::string& name, int value) const {
            glUniform1i(getUniformLocation(name), value);
        }

        void setFloat(const std::string& name, float value) const {
            glUniform1f(getUniformLocation(name), value);
        }

        void setVec2(const std::string& name, float x, float y) const {
            glUniform2f(getUniformLocation(name), x, y);
        }

        void setVec3(const std::string& name, float x, float y, float z) const {
            glUniform3f(getUniformLocation(name), x, y, z);
        }

        void setVec4(const std::string& name, float x, float y, float z, float w) const {
            glUniform4f(getUniformLocation(name), x, y, z, w);
        }

        void setMat4(const std::string& name, const float* value) const {
            glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, value);
        }

        // Helper function to set a 3x3 matrix uniform (for homography)
        void setMat3(const std::string& name, const float* value) const {
            glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, value);
        }

        GLuint getProgramID() const {
            return ID;
        }

    private:
        GLuint ID = 0;
        mutable std::unordered_map<std::string, GLint> uniformLocationCache;

        std::string readFile(const char* filePath) {
            namespace fs = std::filesystem;
            if (!fs::exists(filePath)) {
                gl::throwShaderError(gl::ShaderErrorCode::FILE_NOT_FOUND, std::string(filePath) + " does not exist.");
            }
            std::ifstream file(filePath);
            if (!file) {
                gl::throwShaderError(gl::ShaderErrorCode::FILE_READ_ERROR, "Failed to open " + std::string(filePath));
            }
            std::stringstream stream;
            stream << file.rdbuf();
            file.close();
            return stream.str();
        }

        GLuint compileShader(GLenum type, const std::string& source) {
            const char* shaderCode = source.c_str();
            GLuint shader = glCreateShader(type);

            glShaderSource(shader, 1, &shaderCode, nullptr);
            glCompileShader(shader);

            // Check for compilation errors
            GLint success;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

            if (!success) {
                GLchar infoLog[1024];
                glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);

                gl::ShaderErrorCode errorCode;
                switch (type) {
                case GL_VERTEX_SHADER:
                    errorCode = gl::ShaderErrorCode::VERTEX_COMPILATION_ERROR;
                    break;
                case GL_FRAGMENT_SHADER:
                    errorCode = gl::ShaderErrorCode::FRAGMENT_COMPILATION_ERROR;
                    break;
                case GL_GEOMETRY_SHADER:
                    errorCode = gl::ShaderErrorCode::GEOMETRY_COMPILATION_ERROR;
                    break;
                case GL_COMPUTE_SHADER:
                    errorCode = gl::ShaderErrorCode::COMPUTE_COMPILATION_ERROR;
                    break;
                default:
                    errorCode = gl::ShaderErrorCode::UNKNOWN_ERROR;
                }

                glDeleteShader(shader);
                gl::throwShaderError(errorCode, infoLog);
                return 0; // Unreachable
            }

            return shader;
        }

        GLuint createAndLinkProgram(GLuint vertexShader, GLuint fragmentShader) {
            GLuint program = glCreateProgram();
            glAttachShader(program, vertexShader);
            glAttachShader(program, fragmentShader);
            glLinkProgram(program);

            GLint success;
            glGetProgramiv(program, GL_LINK_STATUS, &success);

            if (!success) {
                GLchar infoLog[1024];
                glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);

                glDeleteProgram(program);
                gl::throwShaderError(gl::ShaderErrorCode::PROGRAM_LINKING_ERROR, infoLog);
                return 0; // Unreachable
            }

            return program;
        }

        GLint getUniformLocation(const std::string& name) const {
            auto it = uniformLocationCache.find(name);
            if (it != uniformLocationCache.end()) {
                return it->second;
            }
            GLint location = glGetUniformLocation(ID, name.c_str());
            uniformLocationCache[name] = location;

            if (location == -1) {
                gl::logWarning(gl::ShaderErrorManager::instance().formatError(
                    gl::ShaderErrorCode::UNIFORM_NOT_FOUND,
                    "'" + name + "' doesn't exist or is not used"
                ));
            }
            return location;
        }
    };
}

#endif // SHADER_HPP
