#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <string>

class Entity;

class Component {
public:
    Component() = default;
    virtual ~Component() = default;

    virtual void init() {}
    virtual void update(float deltaTime) {}
    virtual void render() {}

    void setEntity(Entity* entity) { entity_ = entity; }
    Entity* getEntity() const { return entity_; }

    const std::string& getName() const { return name_; }
    void setName(const std::string& name) { name_ = name; }

protected:
    Entity* entity_ = nullptr;
    std::string name_ = "Component";
};

#endif // COMPONENT_HPP