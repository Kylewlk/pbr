//
// Created by DELL on 2023/8/30.
//
#pragma once

#include "Base3DScene.h"

class PbrLight2Scene : public Base3DScene
{
public:
    static constexpr const char* ID = "Pbr Light2";
    
    static SceneRef create();

    ~PbrLight2Scene() override = default;
    PbrLight2Scene(const PbrLight2Scene&) = delete;
    PbrLight2Scene& operator=(PbrLight2Scene&) = delete;

private:
    PbrLight2Scene(int width, int height);

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
    float ao{1.0};
};

