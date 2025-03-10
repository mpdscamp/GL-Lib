#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include "../include/gl/shader.hpp"
#include "../include/gl/texture.hpp"
#include <string>
#include <unordered_map>
#include <memory>
#include <filesystem>

class ResourceManager {
public:
    // Constructor/Destructor
    ResourceManager() = default;
    ~ResourceManager() = default;

    // Non-copyable
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    // Loads (or retrieves from cache) and returns a shader.
    std::shared_ptr<gl::Shader> loadShader(const std::string& name, const char* vertexPath, const char* fragmentPath);
    std::shared_ptr<gl::Shader> getShader(const std::string& name);

    // Loads (or retrieves from cache) and returns a texture.
    std::shared_ptr<gl::Texture> loadTexture(const std::string& name, const std::string& filePath);
    std::shared_ptr<gl::Texture> getTexture(const std::string& name);

    // Hot reload shaders
    void reloadShaders();

    // Clears all loaded resources.
    void clear();

    // Convenience method to get base resource directory
    static std::filesystem::path getResourcePath(const std::string& relativePath);

private:
    std::unordered_map<std::string, std::shared_ptr<gl::Shader>> shaders_;
    std::unordered_map<std::string, std::shared_ptr<gl::Texture>> textures_;

    // Store original paths for hot-reloading
    std::unordered_map<std::string, std::string> shaderSourcePaths_;
    std::unordered_map<std::string, std::string> textureSourcePaths_;
};

#endif // RESOURCE_MANAGER_HPP