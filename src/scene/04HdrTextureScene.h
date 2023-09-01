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
    HdrTextureScene(int width, int height);

    void onMouseEvent(const MouseEvent* e) override;

    void draw() override;
    void reset() override;
    void drawSettings() override;
    void drawCubMap(const TextureRef& cubeMap, float scale, bool isCube);
    void drawSkyBox(const TextureRef& cubeMap);
    void drawUnfold(const TextureRef& cubeMap);
    void createCubMap(const TextureRef& hdr, TextureRef& cubeMap);
    void createIrradiance(const TextureRef& cubeMap, TextureRef& irradiance);

    Camera2DRef camera2d;

    TextureRef roomHdr;
    TextureRef roomCubeMap;
    TextureRef roomIrradiance;
    TextureRef skyHdr;
    TextureRef skyCubeMap;
    TextureRef skyIrradiance;
    TextureRef textureHdr;
    TextureRef textureCubeMap;
    TextureRef textureIrradiance;

    ShaderRef shaderTexLinear;
    ShaderRef shaderHdrToCubeMap;
    ShaderRef shaderCubMap;
    ShaderRef shaderIrradiance;
    ShaderRef shaderSkyBox;
    ShaderRef shaderUnfold;

    int hdrType = 0;
    int drawType = 0;
};

