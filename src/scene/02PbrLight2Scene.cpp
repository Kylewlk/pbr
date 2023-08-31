//
// Created by wlk12 on 2023/8/6.
//

#include "02PbrLight2Scene.h"
#include "common/Texture.h"
#include "common/Shader.h"
#include "common/FrameBuffer.h"
#include "common/EventSystem.h"
#include "camera/Camera3D.h"
#include "common/Logger.h"
#include "common/RenderModel.h"

PbrLight2Scene::PbrLight2Scene(int width, int height)
    : Base3DScene(ID, width, height, true)
{
    this->pbrShader = Shader::createByPath("asset/shader/model.vert", "asset/shader/01pbr_light.frag");
    this->lightShader = Shader::createByPath("asset/shader/model.vert", "asset/shader/model.frag");

    this->camera = Camera3D::create();
    this->camera->setLockUp(true);

    PbrLight2Scene::reset();
}

SceneRef PbrLight2Scene::create()
{
    struct enable_make_shared : public PbrLight2Scene
    {
        enable_make_shared() : PbrLight2Scene(0, 0) {}
    };
    return std::make_shared<enable_make_shared>();
}

void PbrLight2Scene::reset()
{
    this->camera->resetView();
    this->camera->forward(-800);

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
        this->lightEnables[i] = i < 4;
    }

    this->albedo = {0.8f, 0.0f, 0.0f};
    this->ao = {1.0};
}

void PbrLight2Scene::draw()
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

    pbrShader->setUniform("albedo", albedo);
    pbrShader->setUniform("ao", ao);

    auto mat = math::scale({50, 50, 50});
    auto normalMat = glm::transpose(glm::inverse(math::Mat3{mat}));
    pbrShader->setUniform("normalMatrix", normalMat);

    math::Mat4 model{1};
    const int rows    = 7;
    const int columns = 7;
    float spacing = 120;

    for (int i = 0; i < rows; ++i)
    {
        pbrShader->setUniform("metallic", (float)i / (float)rows);
        for (int j = 0; j < columns; ++j)
        {
            // we clamp the roughness to 0.05 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
            // on direct lighting.
            pbrShader->setUniform("roughness", ((float)j / (float)columns) * 0.95f + 0.05f);

            model = math::translate(glm::vec3((float(j) - (columns / 2.0f)) * spacing,
                                              (float(i) - (rows / 2.0f)) * spacing, 0.0f)) * mat;
            pbrShader->setUniform("model", model);

            renderSphere();
        }
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

void PbrLight2Scene::drawSettings()
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
    ImGui::SliderFloat("ao", &ao, 0.0, 1.0);

}