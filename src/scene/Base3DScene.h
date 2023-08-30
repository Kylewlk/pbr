//
// Created by DELL on 2023/8/30.
//

#pragma once
#include "Scene.hpp"

class Base3DScene : public Scene
{
public:

    ~Base3DScene() override = default;
    Base3DScene(const Base3DScene&) = delete;
    Base3DScene& operator=(Base3DScene&) = delete;

protected:
    Base3DScene(const char* ID, int width, int height, bool multisample);

    void onMouseEvent(const MouseEvent* e) override;

    void drawProperty() override;
    virtual void reset() = 0;
    virtual void drawSettings() = 0;

    Camera3DRef camera;

    math::Vec2 lastMousePos{};
    bool holdLeftButton{false};
    bool holdMidButton{false};
};

