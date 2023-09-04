//
// Created by wlk12 on 2023/8/6.
//
#pragma once

#include "common/PbrMaterial.h"
#include "Base3DScene.h"

class IblTextureScene : public Base3DScene
{
public:
    static constexpr const char* ID = "IBL Texture";

    static SceneRef create();

    ~IblTextureScene() override = default;
    IblTextureScene(const IblTextureScene&) = delete;
    IblTextureScene& operator=(IblTextureScene&) = delete;

private:
    enum DrawType
    {
        kCube,
        kSphere,
        kSphereGroup,
    };

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


    IblTextureScene(int width, int height);

    void draw() override;
    void reset() override;
    void drawSettings() override;
    void drawSkyBox(const TextureRef& cubeMap, int lod = -1);
    void createCubMap(const TextureRef& hdr, TextureRef& cubeMap);
    void createIrradiance(const TextureRef& cubeMap, TextureRef& irradiance);
    void createPrefilter(const TextureRef& cubeMap, TextureRef& prefilter);
    void createBrdfLut();

    static constexpr int lightCount = 8;
    static const int prefilterLevels = 5;

    TextureRef roomHdr;
    TextureRef roomCubeMap;
    TextureRef roomIrradiance;
    TextureRef roomPrefilter;

    TextureRef skyHdr;
    TextureRef skyCubeMap;
    TextureRef skyIrradiance;
    TextureRef skyPrefilter;

    TextureRef textureHdr;
    TextureRef textureCubeMap;
    TextureRef textureIrradiance;
    TextureRef texturePrefilter;

    TextureRef brdfLUT;

    bool enableIblDiffuse{true};
    bool enableIblSpecular{true};

    ShaderRef shaderIblTexture;
    ShaderRef shaderLight;
    ShaderRef shaderHdrToCubeMap;
    ShaderRef shaderIrradiance;
    ShaderRef shaderPrefilter;
    ShaderRef shaderBrdf;
    ShaderRef shaderSkyBox;
    ShaderRef shaderSkyBoxLod;

    math::Vec3 lightColors[lightCount]{};
    math::Vec3 lightPositions[lightCount]{};
    bool lightEnables[lightCount]{false};
    float lightIntensity{300};

    MaterialType materialType{kWall};
    PbrMaterial materials[kMaterialCount];

    int hdrType = 0;
    int backgroundType = 0;
    int backgroundPrefilterLod{0};
};

