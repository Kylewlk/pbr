//
// Created by wlk12 on 2023/8/6.
//
#pragma once

#include "common/PbrMaterial.h"
#include "Base3DScene.h"

class PbrModelScene : public Base3DScene
{
public:
    static constexpr const char* ID = "PBR Model";

    static SceneRef create();

    ~PbrModelScene() override = default;
    PbrModelScene(const PbrModelScene&) = delete;
    PbrModelScene& operator=(PbrModelScene&) = delete;

private:

    PbrModelScene(int width, int height);

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

    ModelRef model;
    PbrMaterial material;

    int hdrType = 0;
    int backgroundType = 0;
    int backgroundPrefilterLod{0};
};

