#include "Entity.hpp"
#include "Scene.hpp"
#include "../gl/logger.hpp"

Entity::Entity(Scene* scene, const std::string& name)
    : scene_(scene), name_(name) {
    gl::logDebug("Entity created: " + name);
}

Entity::~Entity() {
    gl::logDebug("Entity destroyed: " + name_);
}

void Entity::init() {
    for (auto& component : components_) {
        component->init();
    }
}

void Entity::update(float deltaTime) {
    for (auto& component : components_) {
        component->update(deltaTime);
    }
}

void Entity::render() {
    for (auto& component : components_) {
        component->render();
    }
}