//
// Created by Yun on 2022/6/25.
//

#pragma once
#include "Define.h"
#include "MathHelp.h"
#include "Utils.h"

class Model
{
public:
    enum AttributePointer
    {
        kPosition = 0, // vec3
        kNormal = 1,  // vec3
        kTexture = 2, // vec2
        kTangent = 3, // vec3
        kBitangent = 4, // vec3
    };

    struct Vertices
    {
        std::vector<math::Vec3> positions;
        std::vector<math::Vec3> normals;
        std::vector<math::Vec2> texCoords;
//        std::vector<math::Vec3> tangents;
//        std::vector<math::Vec3> bitangents;
    };

    virtual ~Model();
    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    static ModelRef create(std::string_view filePath);
    static ModelRef create();

    [[nodiscard]] GLenum getPrimitiveType() const { return this->primitiveType; }
    GLenum setPrimitiveType(GLenum primitive) { this->primitiveType = primitive; }
    [[nodiscard]] const Vertices& getVertices() const { return this->vertices; }
    [[nodiscard]] const std::vector<uint32_t>& getIndices() const { return this->indices; }
    [[nodiscard]] int32_t getVertexCount() const { return (int32_t)this->vertices.positions.size(); }
    [[nodiscard]] int32_t getIndexCount() const { return (int32_t)this->indices.size(); }

    void setVertices(Vertices vs)
    {
        this->vertices = std::move(vs);
        this->initGL();
    }
    virtual void draw();

    Model();

    std::string name{};

protected:

    void initGL();

    GLenum primitiveType{GL_TRIANGLES};
    Vertices vertices;
    std::vector<uint32_t> indices;

    GLuint vao{};
    GLuint vbo{};
    GLuint ebo{};
};

