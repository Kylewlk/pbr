//
// Created by wlk12 on 2023/8/6.
//
#pragma once

#include "Base3DScene.h"

class HdrTextureScene : public Base3DScene
{
public:
    static constexpr const char* ID = "HDR Texture";

    static SceneRef create();

    ~HdrTextureScene() override = default;
    HdrTextureScene(const HdrTextureScene&) = delete;
    HdrTextureScene& operator=(HdrTextureScene&) = delete;

private:
    enum DrawType
    {
        kTexture,
        kTextureCubeMap,

        kCubeEnvironment,
        kSphereEnvironment,
        kSkyBoxEnvironment,
        kUnfoldEnvironment,

        kCubeIrradiance,
        kSphereIrradiance,
        kSkyBoxIrradiance,
        kUnfoldIrradiance,

        kCubePrefilter,
        kSpherePrefilter,
        kSkyBoxPrefilter,
        kUnfoldPrefilter,

        kBrdfLUT
    };


    HdrTextureScene(int width, int height);

    void onMouseEvent(const MouseEvent* e) override;

    void draw() override;
    void reset() override;
    void drawSettings() override;
    void drawCubMap(const TextureRef& cubeMap, float scale, bool isCube, float lod = -1.0f);
    void drawSkyBox(const TextureRef& cubeMap);
    void drawUnfold(const TextureRef& cubeMap);
    void createCubMap(const TextureRef& hdr, TextureRef& cubeMap);
    void createIrradiance(const TextureRef& cubeMap, TextureRef& irradiance);
    void createPrefilter(const TextureRef& cubeMap, TextureRef& prefilter);
    void createBrdfLut();

    Camera2DRef camera2d;

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

    ShaderRef shaderTexLinear;
    ShaderRef shaderHdrToCubeMap;
    ShaderRef shaderCubMap;
    ShaderRef shaderCubMapLod;
    ShaderRef shaderIrradiance;
    ShaderRef shaderPrefilter;
    ShaderRef shaderBrdf;
    ShaderRef shaderSkyBox;
    ShaderRef shaderUnfold;
    ShaderRef shaderUnfoldLod;

    static const int prefilterLevels = 5;
    int hdrType = 0;
    int drawType = 0;
    int drawPrefilterLevel{0};
    bool enableCubeMapSampless{true};
};

