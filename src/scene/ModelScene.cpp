//
// Created by wlk12 on 2023/8/6.
//

#include "ModelScene.hpp"
#include "common/Texture.h"
#include "common/Shader.h"
#include "common/FrameBuffer.h"
#include "common/EventSystem.h"
#include "camera/Camera3D.h"
#include "common/Logger.h"
#include "common/RenderModel.h"

ModelScene::ModelScene(int width, int height)
    : Base3DScene(ID, width, height, true)
{
    this->shader = Shader::createByPath("asset/shader/model.vert", "asset/shader/model.frag");
    this->camera = Camera3D::create();
    this->camera->setLockUp(true);

    ModelScene::reset();
}

SceneRef ModelScene::create()
{
    struct enable_make_shared : public ModelScene
    {
        enable_make_shared() : ModelScene(0, 0) {}
    };
    return std::make_shared<enable_make_shared>();
}

void ModelScene::reset()
{
    this->camera->resetView();
    this->camera->forward(-200);

    this->lightColor = {0.9, 0.9, 0.9};
    this->lightDir = {1, 1, 0.3};

    this->sphereColor = math::Vec3{0.3, 0.7, 0.5};
    this->cubeColor = math::Vec3{0.2, 0.6, 0.8};
}

void ModelScene::draw()
{
    this->camera->setViewSize((float)this->width, (float)this->height);
    this->camera->update();

    shader->use();
    shader->setUniform("viewProj", camera->getViewProj());

    shader->setUniform("lightColor", lightColor);
    shader->setUniform("lightDir", glm::normalize(lightDir));
    shader->setUniform("cameraPos", camera->getViewPosition());

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
//    glDisable(GL_MULTISAMPLE);

    shader->setUniform("albedo", sphereColor);

    auto mat = math::translate({120, 0, 0}) * math::scale({100, 100, 100});
    auto normalMat = glm::transpose(glm::inverse(math::Mat3{mat}));
    shader->setUniform("model", mat);
    shader->setUniform("normalMatrix", normalMat);
    renderSphere();

    mat = math::translate({-120, 0, 0}) * math::scale({100, 100, 100});
    normalMat = glm::transpose(glm::inverse(math::Mat3{mat}));
    shader->setUniform("model", mat);
    shader->setUniform("normalMatrix", normalMat);
    renderSphere();

    mat = math::translate({120, 220, 0}) * math::scale({100, 100, 100});
    normalMat = glm::transpose(glm::inverse(math::Mat3{mat}));
    shader->setUniform("model", mat);
    shader->setUniform("normalMatrix", normalMat);
    renderSphere();

    mat = math::translate({-120, 220, 0}) * math::scale({100, 100, 100});
    normalMat = glm::transpose(glm::inverse(math::Mat3{mat}));
    shader->setUniform("model", mat);
    shader->setUniform("normalMatrix", normalMat);
    renderSphere();

    shader->setUniform("albedo", cubeColor);

    mat = math::translate({120, -220, 0}) * math::scale({80, 80, 80});
    normalMat = glm::transpose(glm::inverse(math::Mat3{mat}));
    shader->setUniform("model", mat);
    shader->setUniform("normalMatrix", normalMat);
    renderCube();

    mat = math::translate({-120, -220, 0}) * math::scale({80, 80, 80});
    normalMat = glm::transpose(glm::inverse(math::Mat3{mat}));
    shader->setUniform("model", mat);
    shader->setUniform("normalMatrix", normalMat);
    renderCube();
}

void ModelScene::drawSettings()
{
    ImGui::ColorEdit3("Light Color", (float*)&lightColor, ImGuiColorEditFlags_Float);
    ImGui::DragFloat3("Light Direction", (float*)&lightDir);
    ImGui::ColorEdit3("Sphere Color", (float*)&sphereColor, ImGuiColorEditFlags_Float);
    ImGui::ColorEdit3("Cube Color", (float*)&cubeColor, ImGuiColorEditFlags_Float);
}