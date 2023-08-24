//
// Created by DELL on 2022/3/4.
//

#pragma once

#include "Define.h"


struct ShaderInfo
{
    GLenum type = GL_NONE;
    std::string source;
    bool isPath = false;

    ShaderInfo() = default;

    ShaderInfo(GLenum type, std::string source, bool isPath = false)
        : type(type),  source(std::move(source)), isPath(isPath) {}
};

class Shader
{
public:
    virtual ~Shader();
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    static ShaderRef create(const ShaderInfo *si);
    static ShaderRef create(const std::string& vertSrc, const std::string& fragSrc);
    static ShaderRef createByPath(const std::string& vertPath, const std::string& fragPath);

    [[nodiscard]] GLint location(std::string_view name) const;

    void use();
    void bindTexture(const std::string& uniform, const TextureRef& tex)
    {
        this->bindTexture(uniform, tex.get());
    }
    void bindTexture(GLint uniformLocation, const TextureRef& tex)
    {
        this->bindTexture(uniformLocation, tex.get());
    }
    void bindTexture(const std::string& uniform, const Texture* tex);
    void bindTexture(GLint uniformLocation, const Texture* tex);

    [[nodiscard]] const std::string& getName() const { return this->name; }
    void setName(const std::string& n) { this->name = n; }

private:

    Shader();
    static GLuint loadShade(const ShaderInfo *si);
    static GLuint loadShade(GLenum type, const std::string& source);

    std::string name;
    GLuint program{};
    short texUnit{0};
};


