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
    void createCubMap(const TextureRef& hdr, TextureRef& cubeMap);

    Camera2DRef camera2d;

    TextureRef roomHdr;
    TextureRef roomCubeMap;
    TextureRef skyHdr;
    TextureRef skyCubeMap;
    TextureRef textureHdr;
    TextureRef textureCubeMap;

    ShaderRef shaderTexLinear;
    ShaderRef shaderHdrToCubeMap;
    ShaderRef shaderCubMap;

    ShaderRef shader;

    math::Vec3 lightColor{};
    math::Vec3 lightDir{};
    math::Vec3 sphereColor{};
    math::Vec3 cubeColor{};

    int hdrType = 0;
    int drawType = 0;
};

