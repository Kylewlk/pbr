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

    enum MaterialType : int
    {
        kWall = 0,
        kRusted,
        kGold,
        kGrass,
        kPlastic,

        kMaterialCount
    };
    static constexpr std::string_view materialNames[kMaterialCount]{
        "Wall",
        "Rusted",
        "Gold",
        "Grass",
        "Plastic"
    };

    static constexpr std::string_view materialPaths[kMaterialCount]{
        "asset/material/wall/",
        "asset/material/rusted_iron/",
        "asset/material/gold/",
        "asset/material/grass/",
        "asset/material/plastic/"
    };

    void loadMaterial(MaterialType type)
    {
        if (materials[type].albedo == nullptr)
        {
            this->materials[type].load(materialPaths[type]);
        }
    }

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

    MaterialType materialIndex{kWall};
    PbrMaterial materials[kMaterialCount];
};

