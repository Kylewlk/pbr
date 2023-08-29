//
// Created by wlk12 on 2023/8/6.
//

#pragma once

#include "Define.h"
#include "common/MathHelp.h"

class Scene
{
public:
    static constexpr const char* PropertyWindow = "Scene-Property";

    virtual ~Scene() = default;
    Scene(const Scene&) = delete;
    Scene& operator=(Scene&) = delete;

    [[nodiscard]] const std::string& getName() const { return this->name; }
    [[nodiscard]] TextureRef getColorTexture();

    [[nodiscard]] int getWidth() const { return this->width; };
    [[nodiscard]] int getHeight() const { return this->height; }

    [[nodiscard]] const math::Vec4& getClearColor() const { return this->clearColor; }
    void setClearColor(const math::Vec4& color) { this->clearColor = color; }

    [[nodiscard]] bool getShowPropertyWindow() const { return this->showPropertyWindow; }
    void setShowPropertyWindow(bool show) { this->showPropertyWindow = show; }

    virtual void resize(int width, int height);
    virtual void render();

protected:
    Scene(const char* name, int width, int height);

    virtual void draw();
    virtual void drawProperty();

    virtual void onMouseEvent(const MouseEvent* e);
    virtual void onKeyEvent(const KeyEvent* e);

    std::string name;
    int width{0};
    int height{0};
    math::Vec4 clearColor{0.0f, 0.0f, 0.0f, 1.0f};
    bool showPropertyWindow{true};

    FrameBufferRef fbDraw; // multi-sample
    FrameBufferRef fbResolved;
    ShaderRef toneMappingShader;

    MouseListenerRef mouseListener;
    KeyListenerRef keyListener;
};


