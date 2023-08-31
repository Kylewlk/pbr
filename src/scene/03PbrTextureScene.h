//
// Created by DELL on 2023/8/30.
//
#pragma once

#include "Base3DScene.h"
#include "common/PbrMaterial.h"

class PbrTextureScene : public Base3DScene
{
public:
    static constexpr const char* ID = "Pbr Texture";
    
    static SceneRef create();

    ~PbrTextureScene() override = default;
    PbrTextureScene(const PbrTextureScene&) = delete;
    PbrTextureScene& operator=(PbrTextureScene&) = delete;

private:
    PbrTextureScene(int width, int height);

    void draw() override;
    void reset() override;
    void drawSettings() override;

    static constexpr int lightCount = 8;

    ShaderRef pbrShader;
    ShaderRef lightShader;

    math::Vec3 lightColors[lightCount]{};
    math::Vec3 lightPositions[lightCount]{};
    bool lightEnables[lightCount]{false};
    float lightIntensity{200};

    int materialIndex{0};
    PbrMaterial materialGold;
    PbrMaterial materialRusted;
    PbrMaterial materialWall;
};

