#ifndef INPUT_HANDLER_HPP
#define INPUT_HANDLER_HPP

#include "../../core/Component.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <string>

class CameraComponent;
class PostProcessor;

class InputHandler : public Component {
public:
    InputHandler();

    void init() override;
    void update(float deltaTime) override;

    void processInput(float deltaTime);
    void mouseCallback(double xoffset, double yoffset);

    void setKeyState(int key, bool pressed) { keyState_[key] = pressed; }
    bool isKeyPressed(int key) const;

private:
    CameraComponent* camera_ = nullptr;
    PostProcessor* postProcessor_ = nullptr;

    std::unordered_map<int, bool> keyState_;

    bool autoRotate_ = true;

    // Key states for toggles
    bool rKeyPressed_ = false;
    bool pKeyPressed_ = false;
    bool leftBracketPressed_ = false;
    bool rightBracketPressed_ = false;
    bool f5KeyPressed_ = false;
};

#endif // INPUT_HANDLER_HPP