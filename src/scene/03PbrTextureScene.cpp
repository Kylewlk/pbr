//
// Created by wlk12 on 2023/8/6.
//

#include "03PbrTextureScene.h"
#include "common/Texture.h"
#include "common/Shader.h"
#include "common/FrameBuffer.h"
#include "common/EventSystem.h"
#include "camera/Camera3D.h"
#include "common/Logger.h"
#include "common/RenderModel.h"

PbrTextureScene::PbrTextureScene(int width, int height)
    : Base3DScene(ID, width, height, true)
{
    this->pbrShader = Shader::createByPath("asset/shader/model.vert", "asset/shader/03pbr_light_texture.frag");
    this->lightShader = Shader::createByPath("asset/shader/model.vert", "asset/shader/model.frag");

    this->materialWall.load("asset/material/wall/");

    PbrTextureScene::reset();
}

SceneRef PbrTextureScene::create()
{
    struct enable_make_shared : public PbrTextureScene
    {
        enable_make_shared() : PbrTextureScene(0, 0) {}
    };
    return std::make_shared<enable_make_shared>();
}

void PbrTextureScene::reset()
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

    this->lightIntensity = 200;
    for (int i = 0; i < lightCount; ++i)
    {
        this->lightColors[i] = { lightIntensity, lightIntensity, lightIntensity};
        this->lightEnables[i] = true;
    }
}

void PbrTextureScene::draw()
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

    pbrShader->setUniform("normalMatrix", normalMat);

    if (materialIndex == 0)
    {
        materialWall.use(pbrShader);
    }
    else if(materialIndex == 1)
    {
        materialRusted.use(pbrShader);
    }
    else
    {
        materialGold.use(pbrShader);
    }

    pbrShader->setUniform("model", math::translate({-80, 0, 0}) * mat);
    renderSphere();

    pbrShader->setUniform("model", math::translate({80, 0, 0}) * mat);
    renderCube();

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

void PbrTextureScene::drawSettings()
{
    if(ImGui::DragFloat("light Intensity", &this->lightIntensity))
    {
        for (auto& light : lightColors)
        {
            light = {lightIntensity, lightIntensity, lightIntensity};
        }
    }

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
    ImGui::RadioButton("Wall", &materialIndex, 0);

    if (ImGui::RadioButton("Rusted", &materialIndex, 1))
    {
        if (this->materialRusted.albedo == nullptr)
        {
            this->materialRusted.load("asset/material/rusted_iron/");
        }
    }

    if (ImGui::RadioButton("Gold", &materialIndex, 2))
    {
        if (this->materialGold.albedo == nullptr)
        {
            this->materialGold.load("asset/material/gold/");
        }
    }
}