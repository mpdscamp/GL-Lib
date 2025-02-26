#include "ResourceManager.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <stdexcept>

// Singleton instance access
ResourceManager& ResourceManager::getInstance() {
    static ResourceManager instance;
    return instance;
}

// Shader loading and caching
std::shared_ptr<gl::Shader> ResourceManager::loadShader(
    const std::string& name,
    const char* vertexPath,
    const char* fragmentPath)
{
    // Check if shader is already loaded
    auto it = shaders_.find(name);
    if (it != shaders_.end()) {
        return it->second;
    }

    // Create a new shader
    try {
        auto shader = std::make_shared<gl::Shader>(vertexPath, fragmentPath);
        shaders_[name] = shader;
        return shader;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to load shader '" << name << "': " << e.what() << std::endl;
        throw;
    }
}

std::shared_ptr<gl::Shader> ResourceManager::getShader(const std::string& name) {
    auto it = shaders_.find(name);
    if (it != shaders_.end()) {
        return it->second;
    }

    throw std::runtime_error("Shader '" + name + "' not found");
}

// Texture loading and caching
std::shared_ptr<gl::Texture> ResourceManager::loadTexture(
    const std::string& name,
    const std::string& filePath)
{
    // Check if texture is already loaded
    auto it = textures_.find(name);
    if (it != textures_.end()) {
        return it->second;
    }

    // Create a new texture
    auto texture = std::make_shared<gl::Texture>();
    if (!texture->loadFromFile(filePath)) {
        throw std::runtime_error("Failed to load texture from " + filePath);
    }

    textures_[name] = texture;
    return texture;
}

std::shared_ptr<gl::Texture> ResourceManager::getTexture(const std::string& name) {
    auto it = textures_.find(name);
    if (it != textures_.end()) {
        return it->second;
    }

    throw std::runtime_error("Texture '" + name + "' not found");
}

// Clear all resources
void ResourceManager::clear() {
    shaders_.clear();
    textures_.clear();
}