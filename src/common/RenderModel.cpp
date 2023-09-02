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


void renderSphere()
{
    static ModelData sphere;

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

void renderCube()
{
    static ModelData cube;

    // initialize (if necessary)
    if (cube.vao == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
            1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                                                                // right face
            1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
            1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
            1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
            1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
            1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
            1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
            1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        cube.vertexCount = 36;

        glGenVertexArrays(1, &cube.vao);
        glGenBuffers(1, &cube.vbo);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cube.vao);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        cube.cleanListener = CustomEventListener::create(CustomEvent::exitSystemEvent);
        cube.cleanListener->onCustomEvent = [](const CustomEvent* e) {
            glDeleteVertexArrays(1, &cube.vao);
            glDeleteBuffers(1, &cube.vbo);
        };
        EventSystem::get()->subscribe(cube.cleanListener);
    }
    // render Cube
    glBindVertexArray(cube.vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void renderUnfoldCube()
{
    struct Vertex{
        float x{0}, y{0};
        float cx{0}, cy{0}, cz{0}; // cube map 对应的位置
    };

    static ModelData unfoldCube;

    if (unfoldCube.vao == 0)
    {
        glGenVertexArrays(1, &unfoldCube.vao);

        glGenBuffers(1, &unfoldCube.vbo);
        glGenBuffers(1, &unfoldCube.ebo);

        // 参考 /asset/unfold_cube.jpg https://github.com/Kylewlk/pbr/blob/main/asset/unfold_cube.jpg
//        constexpr Vertex vertices[]{
//            {-1, 0, -1, 1, -1},
//            {0, 0, -1, -1, -1}, // 原点
//            {0, 1, -1, 1, -1},
//            {1, 1, 1, 1, -1},
//            {1, 0, 1, -1, -1},
//            {2, 0, 1, 1, -1},
//            {3, 0, -1, 1, -1},
//            {3, -1, -1, 1, 1},
//            {2, -1, 1, 1, 1},
//            {1, -1, 1, -1, 1},
//            {1, -2, 1, 1, 1},
//            {0, -2, -1, 1, 1},
//            {0, -1, -1, -1, 1},
//            {-1, -1, -1, 1, 1},
//        };
//        constexpr uint32_t indices[]{
//            0, 13, 12, // 左
//            0, 12, 1,
//            2, 1, 4, // 后
//            2, 4, 3,
//            1, 12, 9, // 下
//            1, 9, 4,
//            12, 11, 10, // 前
//            12, 10, 9,
//            4, 9, 8, // 右
//            4, 8, 5,
//            5, 8, 7, // 上
//            5, 7, 6
//        };

        // 参考 /asset/unfold_cube2.jpg https://github.com/Kylewlk/pbr/blob/main/asset/unfold_cube2.jpg
        constexpr Vertex vertices[]{
            {1, 1, -1, -1, -1},
            {1, 0, -1, -1, 1},
            {2, 0, 1, -1, 1},
            {2, 1, 1, -1, -1},
            {3, 1, 1, -1, 1},
            {4, 1, -1, -1, 1},
            {4, 2, -1, 1, 1},
            {3, 2, 1, 1, 1},
            {2, 2, 1, 1, -1},
            {2, 3, 1, 1, 1},
            {1, 3, -1, 1, 1},
            {1, 2, -1, 1, -1},
            {0, 2, -1, 1, 1},
            {0, 1, -1, -1, 1},
        };
        constexpr uint32_t indices[]{
            12, 13, 0, // 左
            12, 0, 11,
            10, 11, 8, // 上
            10, 8, 9,
            11, 0, 3, // 后
            11, 3, 8,
            0, 1, 2,  // 下
            0, 2, 3,
            8,3, 4, // 右
            8, 4, 7,
            7,4, 5,  // 前
            7, 5, 6
        };

        unfoldCube.vertexCount = sizeof(indices)/sizeof(uint32_t);

        glBindVertexArray(unfoldCube.vao);
        glBindBuffer(GL_ARRAY_BUFFER, unfoldCube.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, unfoldCube.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        GLsizei stride = sizeof(vertices[0]);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*) offsetof(Vertex, x));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, cx));

        unfoldCube.cleanListener = CustomEventListener::create(CustomEvent::exitSystemEvent);
        unfoldCube.cleanListener->onCustomEvent = [](const CustomEvent* e) {
            glDeleteVertexArrays(1, &unfoldCube.vao);
            glDeleteBuffers(1, &unfoldCube.vbo);
            glDeleteBuffers(1, &unfoldCube.ebo);
        };
        EventSystem::get()->subscribe(unfoldCube.cleanListener);
    }

    glBindVertexArray(unfoldCube.vao);
    glDrawElements(GL_TRIANGLES, unfoldCube.vertexCount, GL_UNSIGNED_INT, nullptr);
}