#pragma once

#include "Define.h"
#include "Logger.h"
#include "MathHelp.h"
#include "TimeSys.h"

#define TO_STRING_CHAR(a) #a
const char* glCodeToString(GLenum error);
#ifdef LOG_SWITCH_OFF
#define CHECK_GL_ERROR()                                                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
    }                                                                                                                  \
    while (false)
#else
#define CHECK_GL_ERROR()                                                                     \
    do                                                                                       \
    {                                                                                        \
        GLenum __error = glGetError();                                                       \
        if (__error)                                                                         \
        {                                                                                    \
            LOGE("OpenGL error 0x{:x}-{}", __error, glCodeToString(__error));                \
        }                                                                                    \
    }                                                                                        \
    while (false)
#endif

#define CLEAR_GL_ERROR()  glGetError()

inline uint32_t rgbPremultiplyAlpha(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return (uint32_t) (((uint32_t) (r * (a + 1)) >> 8) | ((uint32_t) (g * (a + 1) >> 8) << 8) |
                       ((uint32_t) (b * (a + 1) >> 8) << 16) | ((uint32_t) a << 24));
}

inline void premultiplyAlpha(unsigned char* data, uint32_t width, uint32_t height)
{
    auto* fourBytes = (uint32_t*) data;
    for (int i = 0; i < width * height; i++)
    {
        uint8_t* p = data + i * 4;
        fourBytes[i] = rgbPremultiplyAlpha(p[0], p[1], p[2], p[3]);
    }
}

using ByteBuffer = std::vector<char>;

std::string readFileToString(const std::string& filePath);
ByteBuffer readFile(const std::string& filePath);
void writeToFile(const std::string& filePath, const std::string& str);
void writeToFile(const std::string& filePath, const ByteBuffer& data);

void drawQuad();

namespace json
{
    math::Vec2 vec2(Json json);
    math::Vec3 vec3(Json json);
    math::Vec4 vec4(Json json);

    Json to(const math::Vec2& v);
    Json to(const math::Vec3& v);
    Json to(const math::Vec4& v);
}