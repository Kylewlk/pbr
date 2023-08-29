//
// Created by DELL on 2023/8/29.
//

#include "RenderModel.h"
#include "Define.h"
#include "common/MathHelp.h"
#include "common/EventSystem.h"

using namespace math;

struct ModelData
{
    GLuint vao{0}, vbo{0}, ebo{0};
    GLsizei vertexCount{0};
    CustomEventListenerRef cleanListener;

};// namespace DrawTexture


static ModelData sphere;

void renderSphere()
{
    if (sphere.vao == 0)
    {
        glGenVertexArrays(1, &sphere.vao);

        glGenBuffers(1, &sphere.vbo);
        glGenBuffers(1, &sphere.ebo);

        std::vector<Vec3> positions;
        std::vector<Vec2> uv;
        std::vector<Vec3> normals;
        std::vector<uint32_t> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.emplace_back(xPos, yPos, zPos);
                uv.emplace_back(xSegment, ySegment);
                normals.emplace_back(xPos, yPos, zPos);
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y       * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y       * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        sphere.vertexCount = static_cast<GLsizei>(indices.size());

        std::vector<float> data;
        for (unsigned int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (!normals.empty())
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
            if (!uv.empty())
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
        }
        glBindVertexArray(sphere.vao);
        glBindBuffer(GL_ARRAY_BUFFER, sphere.vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        GLsizei stride = (3 + 2 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

        sphere.cleanListener = CustomEventListener::create(CustomEvent::exitSystemEvent);
        sphere.cleanListener->onCustomEvent = [](const CustomEvent* e) {
            glDeleteVertexArrays(1, &sphere.vao);
            glDeleteBuffers(1, &sphere.vbo);
            glDeleteBuffers(1, &sphere.ebo);
        };
        EventSystem::get()->subscribe(sphere.cleanListener);
    }

    glBindVertexArray(sphere.vao);
    glDrawElements(GL_TRIANGLE_STRIP, sphere.vertexCount, GL_UNSIGNED_INT, nullptr);
}
