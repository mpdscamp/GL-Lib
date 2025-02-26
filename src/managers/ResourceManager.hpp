#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include "../include/gl/shader.hpp"
#include "../include/gl/texture.hpp"
#include <string>
#include <unordered_map>
#include <memory>

class ResourceManager {
public:
    // Access the singleton instance.
    static ResourceManager& getInstance();

    // Loads (or retrieves from cache) and returns a shader.
    std::shared_ptr<gl::Shader> loadShader(const std::string& name, const char* vertexPath, const char* fragmentPath);
    std::shared_ptr<gl::Shader> getShader(const std::string& name);

    // Loads (or retrieves from cache) and returns a texture.
    std::shared_ptr<gl::Texture> loadTexture(const std::string& name, const std::string& filePath);
    std::shared_ptr<gl::Texture> getTexture(const std::string& name);

    // Clears all loaded resources.
    void clear();

    // Delete copy constructor and assignment operator.
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

private:
    ResourceManager() = default;
    ~ResourceManager() = default;

    std::unordered_map<std::string, std::shared_ptr<gl::Shader>> shaders_;
    std::unordered_map<std::string, std::shared_ptr<gl::Texture>> textures_;
};

#endif // RESOURCE_MANAGER_HPP
