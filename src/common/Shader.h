//
// Created by DELL on 2022/3/4.
//

#pragma once

#include "Define.h"
#include "MathHelp.h"

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

    void setUniform(std::string_view name, bool b) const;
    void setUniform(std::string_view name, int i) const;
    void setUniform(std::string_view name, float f) const;
    void setUniform(std::string_view name, float f1, float f2) const;
    void setUniform(std::string_view name, float f1, float f2, float f3) const;
    void setUniform(std::string_view name, float f1, float f2, float f3, float f4) const;
    void setUniform(std::string_view name, const math::Vec2& vec2) const;
    void setUniform(std::string_view name, const math::Vec3& vec3) const;
    void setUniform(std::string_view name, const math::Vec4& vec4) const;
    void setUniform(std::string_view name, const math::Vec2* vec2, int count) const;
    void setUniform(std::string_view name, const math::Vec3* vec3, int count) const;
    void setUniform(std::string_view name, const math::Vec4* vec4, int count) const;
    void setUniform(std::string_view name, const math::Mat2& mat2) const;
    void setUniform(std::string_view name, const math::Mat3& mat3) const;
    void setUniform(std::string_view name, const math::Mat4& mat4) const;

private:

    Shader();
    static GLuint loadShade(const ShaderInfo *si);
    static GLuint loadShade(GLenum type, const std::string& source);

    GLuint program{};
    short texUnit{0};
};


