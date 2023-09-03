//
// Created by wlk12 on 2023/8/6.
//
#pragma once

#include "Base3DScene.h"

class IblScene : public Base3DScene
{
public:
    static constexpr const char* ID = "IBL Scene";

    static SceneRef create();

    ~IblScene() override = default;
    IblScene(const IblScene&) = delete;
    IblScene& operator=(IblScene&) = delete;

private:
    enum DrawType
    {
        kCube,
        kSphere,
        kSphereGroup,
    };


    IblScene(int width, int height);

    void draw() override;
    void reset() override;
    void drawSettings() override;
    void drawSkyBox(const TextureRef& cubeMap);
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

    ShaderRef shaderIBL;
    ShaderRef shaderLight;
    ShaderRef shaderHdrToCubeMap;
    ShaderRef shaderIrradiance;
    ShaderRef shaderPrefilter;
    ShaderRef shaderBrdf;
    ShaderRef shaderSkyBox;

    math::Vec3 lightColors[lightCount]{};
    math::Vec3 lightPositions[lightCount]{};
    bool lightEnables[lightCount]{false};

    math::Vec3 albedo{0.5f, 0.0f, 0.0f};
    float roughness{0.5};
    float metallic{0.5};
    float ao{1.0};

    int hdrType = 0;
    int drawType = 0;
    int backgroundType = 0;
};

