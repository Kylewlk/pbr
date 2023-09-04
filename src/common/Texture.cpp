//
// Created by DELL on 2022/3/4.
//

#include "Texture.h"

Texture::Texture() = default;

Texture::~Texture()
{
    if (this->tex != 0)
    {
        glDeleteTextures(1, &tex);
        this->tex = 0;
    }
}

TextureRef Texture::create(const std::string& picPath, bool premultiply /*= true*/)
{
    return create(readFile(picPath), false, false, premultiply);
}

TextureRef Texture::createWithMipmap(const std::string& picPath, bool flipY, bool premultiply)
{
    return create(readFile(picPath), flipY, true, premultiply);
}

TextureRef Texture::create(const ByteBuffer& picData, bool flipY, bool genMipmap, bool premultiply)
{
    int width;
    int height;
    int channels_in_file;
    if (flipY)
    {
        stbi_set_flip_vertically_on_load(flipY);
    }
    uint8_t* imageData = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(picData.data()), (int) picData.size(),
                                               &width, &height, &channels_in_file, STBI_default);
    if (flipY)
    {
        stbi_set_flip_vertically_on_load(false);
    }
    if (imageData == nullptr)
    {
        return {};
    }

    if (channels_in_file == STBI_rgb_alpha && premultiply)
    {
        premultiplyAlpha(imageData, width, height);
    }

    GLenum inFormat = GL_RGBA8, extFormat = GL_RGBA;
    switch (channels_in_file)
    {
    case STBI_grey:
        inFormat = GL_R8;
        extFormat = GL_RED;
        break;
    case STBI_grey_alpha:
        inFormat = GL_RG8;
        extFormat = GL_RG;
        break;
    case STBI_rgb:
        inFormat = GL_RGB8;
        extFormat = GL_RGB;
        break;
    case STBI_rgb_alpha:
        inFormat = GL_RGBA8;
        extFormat = GL_RGBA;
        break;
    default:
        break;
    }
    int32_t levels = 1;
    if (genMipmap)
    {
        levels = getLevels(width, height);
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, levels, inFormat, width, height);
    if (inFormat != GL_RGBA8)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, extFormat, GL_UNSIGNED_BYTE, imageData);

    if (genMipmap)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    if (GL_R8 == inFormat)
    {
        GLint swizzle[]{GL_RED, GL_RED, GL_RED, GL_ONE};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
    }
    else if (GL_RG8 == inFormat)
    {
        GLint swizzle[]{GL_RED, GL_RED, GL_RED, GL_GREEN};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
    }

    stbi_image_free(imageData);

    auto texture = std::make_shared<Texture>();
    texture->tex = tex;
    texture->width = width;
    texture->height = height;
    texture->format = inFormat;
    if (genMipmap)
    {
        texture->setSampler(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    }
    else
    {
        texture->setSampler(GL_LINEAR, GL_CLAMP_TO_EDGE);
    }

    return texture;
}


TextureRef Texture::createHDR(std::string_view picPath)
{
    int width;
    int height;
    int channels_in_file;
    auto pixels = stbi_loadf(picPath.data(), &width, &height, &channels_in_file, STBI_default);

    if (pixels == nullptr)
        return nullptr;

    GLenum inFormat = GL_RGBA16F, extFormat = GL_RGBA;
    switch (channels_in_file)
    {
    case STBI_grey:
        inFormat = GL_R16F;
        extFormat = GL_RED;
        break;
    case STBI_grey_alpha:
        inFormat = GL_RG16F;
        extFormat = GL_RG;
        break;
    case STBI_rgb:
        inFormat = GL_RGB16F;
        extFormat = GL_RGB;
        break;
    case STBI_rgb_alpha:
        inFormat = GL_RGBA16F;
        extFormat = GL_RGBA;
        break;
    default:
        break;
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, inFormat, width, height);
    if (inFormat == GL_R16F)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, extFormat, GL_FLOAT, pixels);

    if (GL_R16F == inFormat)
    {
        GLint swizzle[]{GL_RED, GL_RED, GL_RED, GL_ONE};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
    }
    else if (GL_RG16F == inFormat)
    {
        GLint swizzle[]{GL_RED, GL_RED, GL_RED, GL_GREEN};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
    }

    stbi_image_free(pixels);

    auto texture = std::make_shared<Texture>();
    texture->tex = tex;
    texture->width = width;
    texture->height = height;
    texture->format = inFormat;
    texture->setSampler(GL_LINEAR, GL_CLAMP_TO_EDGE);

    return texture;
}

void Texture::setSampler(GLint scale /*= GL_LINEAR*/, GLint wrap /*= GL_CLAMP_TO_EDGE*/) const
{
    this->setSampler(scale, scale, wrap);
}

void Texture::setSampler(GLint minScale, GLint magScale, GLint wrap) const
{
    glBindTexture(target, this->tex);

    // GL_LINEAR、GL_NEAREST
    // GL_NEAREST_MIPMAP_NEAREST、 GL_LINEAR_MIPMAP_NEAREST、GL_NEAREST_MIPMAP_LINEAR、GL_LINEAR_MIPMAP_LINEAR
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minScale);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magScale);

    // GL_CLAMP_TO_EDGE、GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT, GL_REPEAT, GL_MIRROR_CLAMP_TO_EDGE
    glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(target, GL_TEXTURE_WRAP_R, wrap);
}

void Texture::bind(int unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(target, tex);
}

TextureRef Texture::create(GLenum target, GLenum format, int32_t width, int32_t height, int32_t levels, int32_t depth /*=-1*/)
{
    if (levels < 1)
    {
        levels = getLevels(width, height);
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(target, tex);
    if (depth <= 0)
    {
        glTexStorage2D(target, levels, format, width, height);
    }
    else
    {
        glTexStorage3D(target, levels, format, width, height, depth);
    }

    if (GL_R8 == format || GL_R16 == format || GL_R16F == format)
    {
        GLint swizzle[]{GL_RED, GL_RED, GL_RED, GL_ONE};
        glTexParameteriv(target, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
    }

    auto texture = std::make_shared<Texture>();
    texture->tex = tex;
    texture->width = width;
    texture->height = height;
    texture->target = target;
    texture->format = format;
    texture->setSampler(GL_LINEAR, GL_CLAMP_TO_EDGE);

    return texture;
}

void Texture::update(int x, int y, int width_, int height_, GLenum format_, GLenum type, void* data)
{
    assert(target == GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, tex);
    if (format != GL_RGBA8)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width_, height_, format_, type, data);
}
