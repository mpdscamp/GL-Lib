#include "ResourceManager.hpp"
#include "gl/logger.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>
#include <filesystem>

std::filesystem::path ResourceManager::getResourcePath(const std::string& relativePath) {
    // By default, resources are relative to the executable location
    static std::filesystem::path basePath = std::filesystem::current_path() / "resources";
    return basePath / relativePath;
}

// Shader loading and caching
std::shared_ptr<gl::Shader> ResourceManager::loadShader(
    const std::string& name,
    const char* vertexPath,
    const char* fragmentPath)
{
    // Store paths for hot-reloading
    shaderSourcePaths_[name + ".vert"] = vertexPath;
    shaderSourcePaths_[name + ".frag"] = fragmentPath;

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
        gl::logError("Failed to load shader '" + name + "': " + e.what());
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
    // Store path for potential hot-reloading
    textureSourcePaths_[name] = filePath;

    // Check if texture is already loaded
    auto it = textures_.find(name);
    if (it != textures_.end()) {
        return it->second;
    }

    // Create a new texture
    auto texture = std::make_shared<gl::Texture>();
    if (!texture->loadFromFile(filePath)) {
        gl::logError("Failed to load texture from " + filePath);
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

// Hot reload shaders
void ResourceManager::reloadShaders() {
    gl::logInfo("Reloading shaders...");

    // We'll track which shaders have been processed
    std::unordered_map<std::string, bool> processed;

    // First, identify all unique shader names
    for (const auto& [path, shaderName] : shaderSourcePaths_) {
        // Extract base name from path.vert or path.frag
        std::string baseName;
        size_t dotPos = path.find('.');
        if (dotPos != std::string::npos) {
            baseName = path.substr(0, dotPos);
        }
        else {
            baseName = path; // Fallback
        }

        if (!processed[baseName]) {
            processed[baseName] = true;

            // Find vertex and fragment paths
            std::string vertPath = shaderSourcePaths_[baseName + ".vert"];
            std::string fragPath = shaderSourcePaths_[baseName + ".frag"];

            if (!vertPath.empty() && !fragPath.empty()) {
                try {
                    // Create a new shader
                    auto newShader = std::make_shared<gl::Shader>(
                        vertPath.c_str(), fragPath.c_str());

                    // Replace the old shader
                    shaders_[baseName] = std::move(newShader);
                    gl::logInfo("Reloaded shader: " + baseName);
                }
                catch (const std::exception& e) {
                    gl::logError("Failed to reload shader " + baseName + ": " + e.what());
                }
            }
        }
    }
}

// Clear all resources
void ResourceManager::clear() {
    shaders_.clear();
    textures_.clear();
    shaderSourcePaths_.clear();
    textureSourcePaths_.clear();
}