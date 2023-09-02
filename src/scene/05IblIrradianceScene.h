//
// Created by wlk12 on 2023/8/6.
//
#pragma once

#include "Base3DScene.h"

class IblIrradianceScene : public Base3DScene
{
public:
    static constexpr const char* ID = "IBL Irradiance";

    static SceneRef create();

    ~IblIrradianceScene() override = default;
    IblIrradianceScene(const IblIrradianceScene&) = delete;
    IblIrradianceScene& operator=(IblIrradianceScene&) = delete;

private:
    IblIrradianceScene(int width, int height);

    void draw() override;
    void reset() override;
    void drawSettings() override;
    void drawSkyBox(const TextureRef& cubeMap);
    void createCubMap(const TextureRef& hdr, TextureRef& cubeMap);
    void createIrradiance(const TextureRef& cubeMap, TextureRef& irradiance);

    static constexpr int lightCount = 8;

    TextureRef roomHdr;
    TextureRef roomCubeMap;
    TextureRef roomIrradiance;
    TextureRef skyHdr;
    TextureRef skyCubeMap;
    TextureRef skyIrradiance;
    TextureRef textureHdr;
    TextureRef textureCubeMap;
    TextureRef textureIrradiance;

    ShaderRef shaderModel;
    ShaderRef shaderLight;
    ShaderRef shaderHdrToCubeMap;
    ShaderRef shaderIrradiance;
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

