#include "Scene.hpp"
#include "../window/Window.hpp"
#include "../managers/ResourceManager.hpp"
#include "../components/camera/CameraComponent.hpp"
#include "../components/geometry/MeshComponent.hpp"
#include "../components/rendering/MeshRenderer.hpp"
#include "../components/rendering/PostProcessor.hpp"
#include "../components/effects/HomographyEffect.hpp"
#include "../components/input/InputHandler.hpp"
#include "../gl/logger.hpp"

Scene::Scene(Window& window, ResourceManager& resourceManager)
    : window_(window), resourceManager_(resourceManager) {

    // Register this scene with the window
    window_.addScene(this);

    gl::logInfo("Scene created");
}

Scene::~Scene() {
    // Unregister from window
    window_.removeScene(this);

    // Clear entities in reverse order
    while (!entities_.empty()) {
        entities_.pop_back();
    }

    gl::logInfo("Scene destroyed");
}

void Scene::init() {
    setupScene();

    // Initialize all entities
    for (auto& entity : entities_) {
        entity->init();
    }

    gl::logInfo("Scene initialized");
}

void Scene::update(float deltaTime) {
    deltaTime_ = deltaTime;

    // Update all entities
    for (auto& entity : entities_) {
        entity->update(deltaTime);
    }

    // Explicitly update mesh animations
    Entity* cubeEntity = findEntity("Cube");
    if (cubeEntity) {
        auto cubeMesh = cubeEntity->getComponent<MeshComponent>();
        if (cubeMesh) {
            cubeMesh->animate(deltaTime);
        }
    }
}

void Scene::render() {
    // Set a dark background color
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render all entities EXCEPT HomographyEffect entity
    for (auto& entity : entities_) {
        if (entity->getName() != "HomographyEffect") {
            entity->render();
        }
    }

    // Now render HomographyEffect entity separately
    Entity* homographyEntity = findEntity("HomographyEffect");
    if (homographyEntity) {
        auto homographyEffect = homographyEntity->getComponent<HomographyEffect>();
        if (homographyEffect) {
            homographyEffect->render();
        }
    }
}

void Scene::onWindowResize(int width, int height) {
    // Notify components that need to handle resize
    for (auto& entity : entities_) {
        if (auto camera = entity->getComponent<CameraComponent>()) {
            camera->onWindowResize(width, height);
        }
    }
}

Entity* Scene::createEntity(const std::string& name) {
    auto entity = std::make_unique<Entity>(this, name);
    Entity* entityPtr = entity.get();
    entities_.push_back(std::move(entity));
    return entityPtr;
}

Entity* Scene::findEntity(const std::string& name) {
    for (const auto& entity : entities_) {
        if (entity->getName() == name) {
            return entity.get();
        }
    }
    return nullptr;
}

void Scene::setupScene() {
    // Create camera entity
    Entity* cameraEntity = createEntity("MainCamera");
    auto camera = cameraEntity->addComponent<CameraComponent>(glm::vec3(0.0f, 0.0f, 3.0f));

    // Create input handler
    Entity* inputEntity = createEntity("InputHandler");
    auto inputHandler = inputEntity->addComponent<InputHandler>();

    // Load textures
    auto texture = resourceManager_.loadTexture(
        "shrek",
        "resources/textures/shrek.png"
    );

    // Set texture parameters to ensure proper display
    if (texture) {
        texture->setFilterParameters(gl::TextureFilter::Linear, gl::TextureFilter::Linear);
        texture->setWrapParameters(gl::TextureWrap::Repeat, gl::TextureWrap::Repeat);
        gl::logInfo("Texture loaded successfully");
    }
    else {
        gl::logError("Failed to load texture!");
    }

    // Load shader with texture support
    auto cubeShader = resourceManager_.loadShader(
        "cube",
        "resources/shaders/cube/cube.vert",
        "resources/shaders/cube/cube.frag"
    );

    // Create cube entity
    Entity* cubeEntity = createEntity("Cube");
    auto cubeMesh = cubeEntity->addComponent<MeshComponent>();
    cubeMesh->createCube();

    // Set the rotation axis and initial angle
    cubeMesh->setRotation(30.0f, glm::vec3(1.0f, 1.0f, 0.0f));

    // Enable auto-rotation
    cubeMesh->setAutoRotate(true);

    // Set up cube renderer with texture
    auto cubeRenderer = cubeEntity->addComponent<MeshRenderer>();
    cubeRenderer->setShader(cubeShader);
    cubeRenderer->setTexture(texture);

    // Create homography quad
    Entity* homographyEntity = createEntity("HomographyEffect");
    auto quadMesh = homographyEntity->addComponent<MeshComponent>();
    quadMesh->createQuad();

    // Add the required MeshRenderer component for the quad
    auto quadRenderer = homographyEntity->addComponent<MeshRenderer>();
    quadRenderer->setShader(cubeShader);
    quadRenderer->setTexture(texture);

    // Add the homography effect component
    auto homographyEffect = homographyEntity->addComponent<HomographyEffect>();

    gl::logInfo("Scene setup complete");
}