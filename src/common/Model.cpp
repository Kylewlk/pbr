//
// Created by Yun on 2022/6/25.
//

#include "Model.h"

Model::Model()
{
    glGenVertexArrays(1, &vao);
}

Model::~Model()
{
    if (this->vao != 0)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    if (this->vbo != 0)
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if (this->ebo != 0)
    {
        glDeleteBuffers(1, &ebo);
        ebo = 0;
    }
}

ModelRef Model::create()
{
    auto model = new Model;
    return ModelRef(model);
}

ModelRef Model::create(std::string_view filePath)
{
    auto model = new Model;
    model->name = filePath;

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(filePath.data()))
    {
        std::string msg = "Load Obj model failed! path: " + std::string{filePath};
        LOGE("{}", msg);
        if (!reader.Error().empty())
        {
            LOGE("TinyObjReader: {}", reader.Error());
        }
        throw std::runtime_error(msg.c_str());
    }

    if (!reader.Warning().empty())
    {
        LOGW("TinyObjReader: {}", reader.Warning());
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            auto fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3*size_t(idx.vertex_index)+0];
                tinyobj::real_t vy = attrib.vertices[3*size_t(idx.vertex_index)+1];
                tinyobj::real_t vz = attrib.vertices[3*size_t(idx.vertex_index)+2];
                model->vertices.positions.emplace_back(vx, vy, vz);

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3*size_t(idx.normal_index)+0];
                    tinyobj::real_t ny = attrib.normals[3*size_t(idx.normal_index)+1];
                    tinyobj::real_t nz = attrib.normals[3*size_t(idx.normal_index)+2];
                    model->vertices.normals.emplace_back(nx, ny, nz);
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
                    tinyobj::real_t ty = attrib.texcoords[2*size_t(idx.texcoord_index)+1];
                    if (tx > 1.0 || tx < 0.0)
                    {
                        tx = glm::mod(tx, 1.0f);
                    }
                    if (ty > 1.0 || ty < 0.0)
                    {
                        ty = glm::mod(ty, 1.0f);
                    }
                    model->vertices.texCoords.emplace_back(tx, ty);
                }
            }
            index_offset += fv;
        }
    }

    model->initGL();

    return ModelRef(model);
}

void Model::draw()
{
    if(vao == 0 || getVertexCount() <= 0)
    {
        return;
    }

    glBindVertexArray(vao);
    if(this->ebo != 0 && getIndexCount()>0)
    {
        glDrawElements(GL_TRIANGLES, getIndexCount(), GL_UNSIGNED_INT, nullptr );
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, getVertexCount());
    }
    glBindVertexArray(0);

}

void Model::initGL()
{
    int32_t bufferSize{0};
    bufferSize = this->vertices.positions.size() * sizeof(math::Vec3)
                 + this->vertices.normals.size() * sizeof(math::Vec3)
                 + this->vertices.texCoords.size() * sizeof(math::Vec2);
    if(bufferSize == 0)
    {
        return ;
    }
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);
    int32_t offset{0};
    if(!this->vertices.positions.empty())
    {
        auto posBufferSize = int32_t(vertices.positions.size() * sizeof(math::Vec3));
        glBufferSubData(GL_ARRAY_BUFFER, offset, posBufferSize, vertices.positions.data());
        glEnableVertexAttribArray(AttributePointer::kPosition);
        glVertexAttribPointer(AttributePointer::kPosition, 3, GL_FLOAT, GL_FALSE, sizeof(math::Vec3), (void*)((int64_t)offset));
        offset += posBufferSize;
    }
    if(!this->vertices.normals.empty())
    {
        auto normalBufferSize = int32_t(vertices.normals.size() * sizeof(math::Vec3));
        glBufferSubData(GL_ARRAY_BUFFER, offset, normalBufferSize, vertices.normals.data());
        glEnableVertexAttribArray(AttributePointer::kNormal);
        glVertexAttribPointer(AttributePointer::kNormal, 3, GL_FLOAT, GL_FALSE, sizeof(math::Vec3), (void*)((int64_t)offset));
        offset += normalBufferSize;
    }
    if(!this->vertices.texCoords.empty())
    {
        auto texBufferSize = int32_t(vertices.texCoords.size() * sizeof(math::Vec2));
        glBufferSubData(GL_ARRAY_BUFFER, offset, texBufferSize, vertices.texCoords.data());
        glEnableVertexAttribArray(AttributePointer::kTexture);
        glVertexAttribPointer(AttributePointer::kTexture, 2, GL_FLOAT, GL_FALSE, sizeof(math::Vec2), (void*)((int64_t)offset));
        offset += texBufferSize;
    }

    if (!indices.empty())
    {
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize, indices.data(), GL_STATIC_DRAW);
    }

    glBindVertexArray(0);
}
