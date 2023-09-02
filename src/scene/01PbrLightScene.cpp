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
//    this->camera->forward(200);

    this->lightPositions[0] = {-500.0f,  500.0f, 500.0f};
    this->lightPositions[1] = { 500.0f,  500.0f, 500.0f};
    this->lightPositions[2] = {-500.0f, -500.0f, 500.0f};
    this->lightPositions[3] = { 500.0f, -500.0f, 500.0f};
    this->lightPositions[4] = {-500.0f,  500.0f, -500.0f};
    this->lightPositions[5] = { 500.0f,  500.0f, -500.0f};
    this->lightPositions[6] = {-500.0f, -500.0f, -500.0f};
    this->lightPositions[7] = { 500.0f, -500.0f, -500.0f};

    for (int i = 0; i < lightCount; ++i)
    {
        this->lightColors[i] = { 300.0f, 300.0f, 300.0f};
        this->lightEnables[i] = false;
    }
    lightEnables[1] = true;

    this->albedo = {0.8f, 0.0f, 0.0f};
    this->roughness = {0.3};
    this->metallic = {0.2};
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
    pbrShader->setUniform("lightEnables", lightEnables, lightCount);

    auto mat = math::scale({50, 50, 50});
    auto normalMat = glm::transpose(glm::inverse(math::Mat3{mat}));
    pbrShader->setUniform("model", mat);
    pbrShader->setUniform("normalMatrix", normalMat);

    pbrShader->setUniform("albedo", albedo);
    pbrShader->setUniform("metallic", metallic);
    pbrShader->setUniform("roughness", roughness);
    pbrShader->setUniform("ao", ao);

    if (this->drawType == 0)
    {
        renderSphere();
    }
    else
    {
        renderCube();
    }

    lightShader->use();
    lightShader->setUniform("viewProj", camera->getViewProj());
    lightShader->setUniform("lightColor", math::Vec3 {0.5, 0.5, 0.5});
    lightShader->setUniform("lightDir", glm::normalize(math::Vec3{1, 1, 0.5}));
    lightShader->setUniform("cameraPos", camera->getViewPosition());

    for (int i = 0; i < lightCount; ++i)
    {
        if (lightEnables[i])
        {
            lightShader->setUniform("albedo", math::Vec3{1, 1, 1});
        }
        else
        {
            lightShader->setUniform("albedo", math::Vec3{0.1, 0.3, 0.2});
        }

        mat = math::translate(lightPositions[i]) * math::scale({10, 10, 10});
        normalMat = glm::transpose(glm::inverse(math::Mat3{mat}));
        lightShader->setUniform("model", mat);
        lightShader->setUniform("normalMatrix", normalMat);
        renderSphere();
    }
}

void PbrLightScene::drawSettings()
{

    for (int i = 0; i < lightCount; ++i)
    {
        std::string name = "Enable Light ";
        name += std::to_string(i);

        ImGui::Checkbox(name.c_str(), &lightEnables[i]);
    }
    if (ImGui::Button("All Light"))
    {
        for (auto& lightEnable : lightEnables)
        {
            lightEnable = true;
        }
    }
    ImGui::SameLine(0, 30);
    if (ImGui::Button("Half Light"))
    {
        for (int i = 0; i < lightCount; ++i)
        {
            lightEnables[i] = i < 4;
        }
    }
    ImGui::SameLine(0, 30);
    if (ImGui::Button("1 Light"))
    {
        for (auto& lightEnable : lightEnables)
        {
            lightEnable = false;
        }
        lightEnables[1] = true;
    }
    ImGui::Separator();

    ImGui::ColorEdit3("albedo", (float*)&albedo, ImGuiColorEditFlags_Float);
    ImGui::SliderFloat("roughness", &roughness, 0.0001, 1.0);
    ImGui::SliderFloat("metallic", &metallic, 0.0001, 1.0);
    ImGui::SliderFloat("ao", &ao, 0.0, 1.0);

    ImGui::Separator();
    ImGui::RadioButton("Ball", &drawType, 0);
    ImGui::RadioButton("Cube", &drawType, 1);
}