//
// Created by DELL on 2023/8/30.
//
#pragma once

#include "Base3DScene.h"

class PbrLightScene : public Base3DScene
{
public:
    static constexpr const char* ID = "Pbr Light";
    
    static SceneRef create();

    ~PbrLightScene() override = default;
    PbrLightScene(const PbrLightScene&) = delete;
    PbrLightScene& operator=(PbrLightScene&) = delete;

private:
    PbrLightScene(int width, int height);

    void draw() override;
    void reset() override;
    void drawSettings() override;

    static constexpr int lightCount = 8;

    ShaderRef pbrShader;
    ShaderRef lightShader;

    math::Vec3 lightColors[lightCount]{};
    math::Vec3 lightPositions[lightCount]{};
    bool lightEnables[lightCount]{false};

    math::Vec3 albedo{0.5f, 0.0f, 0.0f};
    float roughness{0.5};
    float metallic{0.5};
    float ao{1.0};

    int drawType = 0;
};

