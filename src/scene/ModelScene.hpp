//
// Created by wlk12 on 2023/8/6.
//
#pragma once

#include "Scene.hpp"

class ModelScene : public Scene
{
public:
    static constexpr const char* ID = "Model Scene";

    static SceneRef create();

    ~ModelScene() override = default;
    ModelScene(const ModelScene&) = delete;
    ModelScene& operator=(ModelScene&) = delete;

private:
    ModelScene(int width, int height);

    void onMouseEvent(const MouseEvent* e) override;

    void draw() override;
    void drawProperty() override;

    ShaderRef shader;
    Camera3DRef camera;

    math::Vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    float rotation = 0.0f;

    math::Vec2 lastMousePos{};
    bool holdLeftButton{false};
    bool holdMidButton{false};
};

