#ifndef ENTITY_HPP
#define ENTITY_HPP

#include "Component.hpp"
#include <memory>
#include <vector>
#include <string>
#include <typeindex>
#include <unordered_map>

class Scene;

class Entity {
public:
    Entity(Scene* scene, const std::string& name = "Entity");
    ~Entity();

    void init();
    void update(float deltaTime);
    void render();

    // Add a component
    template<typename T, typename... Args>
    T* addComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        component->setEntity(this);

        // Store component by type
        std::type_index typeIdx = std::type_index(typeid(T));
        componentsByType_[typeIdx] = component.get();

        T* componentPtr = component.get();
        components_.push_back(std::move(component));

        return componentPtr;
    }

    // Get a component
    template<typename T>
    T* getComponent() const {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        std::type_index typeIdx = std::type_index(typeid(T));
        auto it = componentsByType_.find(typeIdx);
        if (it != componentsByType_.end()) {
            return static_cast<T*>(it->second);
        }
        return nullptr;
    }

    Scene* getScene() const { return scene_; }
    const std::string& getName() const { return name_; }

private:
    Scene* scene_;
    std::string name_;
    std::vector<std::unique_ptr<Component>> components_;
    std::unordered_map<std::type_index, Component*> componentsByType_;
};

#endif // ENTITY_HPP