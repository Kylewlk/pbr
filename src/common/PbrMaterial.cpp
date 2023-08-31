//
// Created by DELL on 2023/8/31.
//

#include "PbrMaterial.h"
#include "Texture.h"
#include "Shader.h"

void PbrMaterial::load(std::string_view path, std::string_view extension)
{
    std::string file;

    file = path;
    file += "albedo";
    file += extension;
    this->albedo = Texture::create(file, false);

    file = path;
    file += "normal";
    file += extension;
    this->normal = Texture::create(file, false);

    file = path;
    file += "metallic";
    file += extension;
    this->metallic = Texture::create(file, false);

    file = path;
    file += "roughness";
    file += extension;
    this->roughness = Texture::create(file, false);

    file = path;
    file += "ao";
    file += extension;
    this->ao = Texture::create(file, false);
}

void PbrMaterial::use(const ShaderRef& shader) const
{
    shader->bindTexture("albedoMap", this->albedo);
    shader->bindTexture("normalMap", this->normal);
    shader->bindTexture("metallicMap", this->metallic);
    shader->bindTexture("roughnessMap", this->roughness);
    shader->bindTexture("aoMap", this->ao);
}
