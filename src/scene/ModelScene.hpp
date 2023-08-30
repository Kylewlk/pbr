//
// Created by wlk12 on 2023/8/6.
//
#pragma once

#include "Base3DScene.h"

class ModelScene : public Base3DScene
{
public:
    static constexpr const char* ID = "Model Scene";

    static SceneRef create();

    ~ModelScene() override = default;
    ModelScene(const ModelScene&) = delete;
    ModelScene& operator=(ModelScene&) = delete;

private:
    ModelScene(int width, int height);

    void draw() override;
    void reset() override;
    void drawSettings() override;

    ShaderRef shader;

    math::Vec3 lightColor{};
    math::Vec3 lightDir{};
    math::Vec3 sphereColor{};
    math::Vec3 cubeColor{};
};

