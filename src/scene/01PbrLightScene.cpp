//
// Created by wlk12 on 2023/8/6.
//

#include "01PbrLightScene.h"
#include "common/Texture.h"
#include "common/Shader.h"
#include "common/FrameBuffer.h"
#include "common/EventSystem.h"
#include "camera/Camera3D.h"
#include "common/Logger.h"
#include "common/RenderModel.h"

PbrLightScene::PbrLightScene(int width, int height)
    : Base3DScene(ID, width, height, true)
{
    this->pbrShader = Shader::createByPath("asset/shader/model.vert", "asset/shader/01pbr_light.frag");
    this->lightShader = Shader::createByPath("asset/shader/model.vert", "asset/shader/model.frag");

    this->camera = Camera3D::create();
    this->camera->setLockUp(true);

    PbrLightScene::reset();
}

SceneRef PbrLightScene::create()
{
    struct enable_make_shared : public PbrLightScene
    {
        enable_make_shared() : PbrLightScene(0, 0) {}
    };
    return std::make_shared<enable_make_shared>();
}

void PbrLightScene::reset()
{
    this->camera->resetView();
    this->camera->forward(200);

    this->lightPositions[0] = {-1000.0f,  1000.0f, 1000.0f};
    this->lightPositions[1] = { 1000.0f,  1000.0f, 1000.0f};
    this->lightPositions[2] = {-1000.0f, -1000.0f, 1000.0f};
    this->lightPositions[3] = { 1000.0f, -1000.0f, 1000.0f};

    this->lightColors[0] = { 30000000.0f, 650000.0f, 650000.0f};
    this->lightColors[1] = { 30000000.0f, 650000.0f, 650000.0f};
    this->lightColors[2] = { 30000000.0f, 650000.0f, 650000.0f};
    this->lightColors[3] = { 30000000.0f, 650000.0f, 650000.0f};


    this->albedo = {0.5f, 0.0f, 0.0f};
    this->roughness = {0.2};
    this->metallic = {0.9};
    this->ao = {1.0};
}

void PbrLightScene::draw()
{
    this->camera->setViewSize((float)this->width, (float)this->height);
    this->camera->update();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    //    glDisable(GL_MULTISAMPLE);

    pbrShader->use();
    pbrShader->setUniform("viewProj", camera->getViewProj());
    pbrShader->setUniform("camPos", camera->getViewPosition());
    pbrShader->setUniform("lightColors", lightColors, lightCount);
    pbrShader->setUniform("lightPositions", lightPositions, lightCount);

    auto mat = math::scale({100, 100, 100});
    auto normalMat = glm::transpose(glm::inverse(math::Mat3{mat}));
    pbrShader->setUniform("model", mat);
    pbrShader->setUniform("normalMatrix", normalMat);

    pbrShader->setUniform("albedo", albedo);
    pbrShader->setUniform("metallic", metallic);
    pbrShader->setUniform("roughness", roughness);
    pbrShader->setUniform("ao", ao);

    renderSphere();



    lightShader->use();
    lightShader->setUniform("viewProj", camera->getViewProj());
    lightShader->setUniform("lightColor", math::Vec3 {1, 1, 1});
    lightShader->setUniform("lightDir", glm::normalize(math::Vec3{1, 1, 0.5}));
    lightShader->setUniform("cameraPos", camera->getViewPosition());
    lightShader->setUniform("albedo", math::Vec3{1, 1, 1});

    mat = math::translate(lightPositions[0]) * math::scale({10, 10, 10});
    normalMat = glm::transpose(glm::inverse(math::Mat3{mat}));
    lightShader->setUniform("model", mat);
    lightShader->setUniform("normalMatrix", normalMat);
    renderSphere();

    mat = math::translate({120, 220, 0}) * math::scale({10, 10, 10});
    normalMat = glm::transpose(glm::inverse(math::Mat3{mat}));
    lightShader->setUniform("model", mat);
    lightShader->setUniform("normalMatrix", normalMat);
    renderSphere();

    mat = math::translate({-120, 220, 0}) * math::scale({10, 10, 10});
    normalMat = glm::transpose(glm::inverse(math::Mat3{mat}));
    lightShader->setUniform("model", mat);
    lightShader->setUniform("normalMatrix", normalMat);
    renderSphere();

}

void PbrLightScene::drawSettings()
{
//    ImGui::ColorEdit3("Light Color", (float*)&lightColor);
//    ImGui::DragFloat3("Light Direction", (float*)&lightDir);
//    ImGui::ColorEdit3("Sphere Color", (float*)&sphereColor);
//    ImGui::ColorEdit3("Cube Color", (float*)&cubeColor);
}