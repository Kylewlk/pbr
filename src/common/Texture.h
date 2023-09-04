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
    static TextureRef createWithMipmap(const std::string& picPath, bool flipY, bool premultiply);
    static TextureRef create(const ByteBuffer& picData, bool flipY, bool genMipmap, bool premultiply);
    static TextureRef createHDR(std::string_view picPath);

    // levels < 0 , 会自动计算level
    static TextureRef create(GLenum target, GLenum format, int32_t width, int32_t height, int32_t levels, int32_t depth = -1);
    static TextureRef create(GLenum format, int32_t width, int32_t height, int levels = 1)
    {
        return create(GL_TEXTURE_2D,format, width, height, levels, -1);
    }
    static TextureRef createCubemap(GLenum format, int32_t width, int32_t height, int levels = 1)
    {
        return create(GL_TEXTURE_CUBE_MAP, format, width, height, levels, -1);
    }
    static int32_t getLevels(int32_t width, int32_t height)
    {
        return int32_t(std::log(float(std::max(width, height))) / std::log(2.0f)) + 1;
    }

    void update(int x, int y, int width, int height, GLenum format, GLenum type, void* data);

    [[nodiscard]] math::Vec2 getSize() const { return {static_cast<float>(width), static_cast<float>(height)}; }
    [[nodiscard]] int getWidth() const { return this->width; }
    [[nodiscard]] int getHeight() const {return this->height; }
    [[nodiscard]] GLenum getFormat() const {return this->format; }

    [[nodiscard]] GLenum getTarget() const { return this->target; }
    [[nodiscard]] GLuint getHandle() const { return this->tex; }

    void setSampler(GLint scale = GL_LINEAR, GLint wrap = GL_CLAMP_TO_EDGE) const;
    void setSampler(GLint minScale, GLint magScale, GLint wrap) const;

    void bind(int unit) const;

    Texture();

protected:
    int width {};
    int height {};
    GLenum target{GL_TEXTURE_2D};
    GLenum format{GL_RGBA8};

    GLuint tex{};
};


