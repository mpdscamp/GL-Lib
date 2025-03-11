#ifndef SCENE_HPP
#define SCENE_HPP

#include "Entity.hpp"
#include <memory>
#include <vector>
#include <string>

class Window;
class ResourceManager;

class Scene {
public:
    Scene(Window& window, ResourceManager& resourceManager);
    virtual ~Scene();

    virtual void init();
    virtual void update(float deltaTime);
    virtual void render();

    void onWindowResize(int width, int height);

    // Create a new entity
    Entity* createEntity(const std::string& name = "Entity");

    // Find an entity by name
    Entity* findEntity(const std::string& name);

    // Getters
    Window& getWindow() const { return window_; }
    ResourceManager& getResourceManager() const { return resourceManager_; }

    float getDeltaTime() const { return deltaTime_; }

protected:
    Window& window_;
    ResourceManager& resourceManager_;
    std::vector<std::unique_ptr<Entity>> entities_;
    float deltaTime_ = 0.0f;

    virtual void setupScene();
};

#endif // SCENE_HPP