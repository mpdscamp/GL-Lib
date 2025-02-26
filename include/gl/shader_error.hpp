#ifndef SHADER_ERROR_HPP
#define SHADER_ERROR_HPP

#include <string>
#include <unordered_map>
#include <stdexcept>
#include <string_view>

namespace gl {

    // Error codes for shader operations
    enum class ShaderErrorCode {
        // File operations
        FILE_NOT_FOUND,
        FILE_READ_ERROR,

        // Compilation errors
        VERTEX_COMPILATION_ERROR,
        FRAGMENT_COMPILATION_ERROR,
        GEOMETRY_COMPILATION_ERROR,
        COMPUTE_COMPILATION_ERROR,

        // Linking errors
        PROGRAM_LINKING_ERROR,

        // Runtime errors
        UNIFORM_NOT_FOUND,
        ATTRIBUTE_NOT_FOUND,
        INVALID_OPERATION,

        // Other
        UNKNOWN_ERROR
    };

    // Error manager for shader operations
    class ShaderErrorManager {
    public:
        static ShaderErrorManager& instance() {
            static ShaderErrorManager instance;
            return instance;
        }

        // Get error message template for a specific error code
        std::string_view getErrorMessage(ShaderErrorCode code) const {
            auto it = errorMessages.find(code);
            if (it != errorMessages.end()) {
                return it->second;
            }
            return errorMessages.at(ShaderErrorCode::UNKNOWN_ERROR);
        }

        // Format error message with additional details
        std::string formatError(ShaderErrorCode code, const std::string& details = "") const {
            std::string message(getErrorMessage(code));
            if (!details.empty()) {
                message += "\n" + details;
            }
            return message;
        }

        // Create exception with formatted error message
        std::runtime_error createException(ShaderErrorCode code, const std::string& details = "") const {
            return std::runtime_error(formatError(code, details));
        }

    private:
        ShaderErrorManager() {
            // Initialize error message templates
            errorMessages = {
                {ShaderErrorCode::FILE_NOT_FOUND, "ERROR::SHADER::FILE_NOT_FOUND"},
                {ShaderErrorCode::FILE_READ_ERROR, "ERROR::SHADER::FILE_READ_ERROR"},
                {ShaderErrorCode::VERTEX_COMPILATION_ERROR, "ERROR::SHADER::VERTEX::COMPILATION_FAILED"},
                {ShaderErrorCode::FRAGMENT_COMPILATION_ERROR, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED"},
                {ShaderErrorCode::GEOMETRY_COMPILATION_ERROR, "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED"},
                {ShaderErrorCode::COMPUTE_COMPILATION_ERROR, "ERROR::SHADER::COMPUTE::COMPILATION_FAILED"},
                {ShaderErrorCode::PROGRAM_LINKING_ERROR, "ERROR::SHADER::PROGRAM::LINKING_FAILED"},
                {ShaderErrorCode::UNIFORM_NOT_FOUND, "ERROR::SHADER::UNIFORM_NOT_FOUND"},
                {ShaderErrorCode::ATTRIBUTE_NOT_FOUND, "ERROR::SHADER::ATTRIBUTE_NOT_FOUND"},
                {ShaderErrorCode::INVALID_OPERATION, "ERROR::SHADER::INVALID_OPERATION"},
                {ShaderErrorCode::UNKNOWN_ERROR, "ERROR::SHADER::UNKNOWN_ERROR"}
            };
        }

        std::unordered_map<ShaderErrorCode, std::string> errorMessages;
    };

    // Helper function to throw shader errors
    inline void throwShaderError(ShaderErrorCode code, const std::string& details = "") {
        throw ShaderErrorManager::instance().createException(code, details);
    }

} // namespace gl

#endif // SHADER_ERROR_HPP