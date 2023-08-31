//
// Created by DELL on 2023/8/31.
//

#pragma once
#include "Define.h"

class PbrMaterial
{
public:
    void load(std::string_view path, std::string_view extension = ".png");

    void use(const ShaderRef& shader) const;

    TextureRef albedo;
    TextureRef normal;
    TextureRef metallic;
    TextureRef roughness;
    TextureRef ao;
};

