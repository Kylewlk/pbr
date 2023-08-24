
#include <fstream>
#include <iostream>

#include "EventSystem.h"
#include "Utils.h"

#include "MathHelp.h"

const char* glCodeToString(GLenum error)
{
    switch (error)
    {
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_STACK_OVERFLOW:
        return "GL_STACK_OVERFLOW";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    default:
        return "<<unknow>>";
    }
}

std::string readFileToString(const std::string& filePath)
{
    std::string text;
    std::ifstream stream(filePath, std::ios::binary | std::ios::in);
    if (stream.is_open())
    {
        std::streampos start = stream.tellg();
        stream.seekg(0, std::ios::end);
        std::streampos end = stream.tellg();
        stream.seekg(0, std::ios::beg);
        auto fileSize = end - start;
        text.resize(uint32_t(fileSize));
        stream.read(text.data(), fileSize);
        stream.close();
    }
    else
    {
        std::string msg = "failed to open file!, File Path: " + filePath;
        std::cerr << msg << std::endl;
        throw std::runtime_error(msg);
    }
    return text;
}

ByteBuffer readFile(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        std::string msg = "failed to open file!, File Path: " + filePath;
        std::cerr << msg << std::endl;
        throw std::runtime_error(msg);
    }
    auto fileSize = file.tellg();
    ByteBuffer buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

void writeToFile(const std::string& filePath, const std::string& str)
{
    std::ofstream file(filePath, std::ios::ate|std::ios::binary);
    if (!file.is_open())
    {
        std::string msg = "failed to create file!, File Path: " + filePath;
        std::cerr << msg << std::endl;
        throw std::runtime_error(msg);
    }

    file.write(str.data(), (std::streamsize)str.size());
    file.flush();
    file.close();
}

void writeToFile(const std::string& filePath, const ByteBuffer& data)
{
    std::ofstream file(filePath, std::ios::ate|std::ios::binary);
    if (!file.is_open())
    {
        std::string msg = "failed to create file!, File Path: " + filePath;
        std::cerr << msg << std::endl;
        throw std::runtime_error(msg);
    }

    file.write(data.data(), (std::streamsize)data.size());
    file.flush();
    file.close();
}


namespace DrawQuad
{
    GLuint vao{0}, vbo{0};
    CustomEventListenerRef cleanListener;
}// namespace DrawTexture

void drawQuad()
{
    using namespace DrawQuad;
    if (vao == 0)
    {
        // clang-format off
        const float vertices[] = {
                -1.0f,  1.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f,
                 1.0f,  1.0f, 1.0f, 1.0f,
                 1.0f, -1.0f, 1.0f, 0.0f
        };
        // clang-format on
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*) nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*) (sizeof(float) * 2));
        glEnableVertexAttribArray(1);
        CHECK_GL_ERROR();
        cleanListener = CustomEventListener::create(CustomEvent::exitSystemEvent);
        cleanListener->onCustomEvent = [](const CustomEvent* e) {
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vbo);
        };
        EventSystem::get()->subscribe(cleanListener);
    }

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

namespace json
{
    math::Vec2 vec2(Json json)
    {
        return math::Vec2{json["x"], json["y"]};
    }

    math::Vec3 vec3(Json json)
    {
        return math::Vec3{json["x"], json["y"], json["z"]};
    }

    math::Vec4 vec4(Json json)
    {
        return math::Vec4{json["x"], json["y"], json["z"], json["w"]};
    }

    Json to(const math::Vec2& v)
    {
        Json j;
        j["x"] = v.x;
        j["y"] = v.y;
        return j;
    }

    Json to(const math::Vec3& v)
    {
        Json j;
        j["x"] = v.x;
        j["y"] = v.y;
        j["z"] = v.z;
        return j;
    }

    Json to(const math::Vec4& v)
    {
        Json j;
        j["x"] = v.x;
        j["y"] = v.y;
        j["z"] = v.z;
        j["w"] = v.w;
        return j;
    }
}