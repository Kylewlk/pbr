//
// Created by DELL on 2022/3/4.
//
#pragma once
#include "Define.h"
#include "MathHelp.h"
#include "Utils.h"

class Texture
{
public:
    friend class RenderTarget;

    virtual ~Texture();
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    static TextureRef create(const std::string& picPath, bool premultiply = true);
    static TextureRef create(const ByteBuffer& picData, bool premultiply = true);
    static TextureRef createHDR(std::string_view picPath);

    static TextureRef create(GLenum format, int32_t width, int32_t height);

    void update(int x, int y, int width, int height, GLenum format, GLenum type, void* data);

    [[nodiscard]] math::Vec2 getSize() const { return {static_cast<float>(width), static_cast<float>(height)}; }
    [[nodiscard]] int getWidth() const { return this->width; }
    [[nodiscard]] int getHeight() const {return this->height; }
    [[nodiscard]] GLenum getFormat() const {return this->format; }

    [[nodiscard]] GLuint getHandle() const { return this->tex; }

    void setSampler(GLint scale = GL_LINEAR, GLint wrap = GL_CLAMP_TO_EDGE) const;
    void setSampler(GLint minScale, GLint magScale, GLint wrap) const;

    void bind(int unit) const;

    Texture();

protected:
    int width {};
    int height {};
    GLenum format{GL_RGBA8};

    GLuint tex{};
};


