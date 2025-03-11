#ifndef POST_PROCESSOR_HPP
#define POST_PROCESSOR_HPP

#include "../../core/Component.hpp"
#include "../../../include/gl/shader.hpp"
#include "../../../include/gl/framebuffer.hpp"
#include <memory>

class MeshComponent;

class PostProcessor : public Component {
public:
    PostProcessor();

    void init() override;
    void update(float deltaTime) override;
    void render() override;

    void setShader(std::shared_ptr<gl::Shader> shader) { shader_ = shader; }

    void beginRender();
    void endRender();

    void onWindowResize(int width, int height);

    bool isEnabled() const { return enabled_; }
    void setEnabled(bool enabled) { enabled_ = enabled; }

    int getEffectIndex() const { return currentEffect_; }
    void setEffectIndex(int index) { currentEffect_ = index; }
    void nextEffect();
    void previousEffect();

private:
    bool enabled_ = true;
    int currentEffect_ = 0;
    int numEffects_ = 5;

    std::unique_ptr<gl::FrameBuffer> framebuffer_;
    std::shared_ptr<gl::Shader> shader_;
    MeshComponent* quadMesh_ = nullptr;
};

#endif // POST_PROCESSOR_HPP