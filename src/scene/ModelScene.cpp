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
    : Scene(ID, width, height)
{
    this->shader = Shader::createByPath("asset/shader/model.vert", "asset/shader/model.frag");
    this->camera = Camera3D::create();
    this->camera->setLockUp(true);
}

SceneRef ModelScene::create()
{
    struct enable_make_shared : public ModelScene
    {
        enable_make_shared() : ModelScene(0, 0) {}
    };
    return std::make_shared<enable_make_shared>();
}

void ModelScene::draw()
{
    this->camera->setViewSize((float)this->width, (float)this->height);
    this->camera->update();

    shader->use();
    shader->setUniform("viewProj", camera->getViewProj());

    shader->setUniform("lightColor", math::Vec3{0.9, 0.9, 0.9});
    shader->setUniform("lightDir", glm::normalize(math::Vec3{0.9, 0.9, 0.0}));
    shader->setUniform("cameraPos", camera->getViewPosition());

    shader->setUniform("albedo", math::Vec3{0.6, 1.0, 0.9});

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

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
}


void ModelScene::drawProperty()
{
    if (!showPropertyWindow)
    {
        return;
    }

    if(ImGui::Begin(Scene::PropertyWindow, &showPropertyWindow, 0))
    {
        ImGui::SetWindowSize({300, 400}, ImGuiCond_FirstUseEver);
        if (ImGui::Button("Reset", {100.0f, 0}))
        {
            this->color = { 1.0f, 1.0f, 1.0f, 1.0f};
            this->rotation = 0.0f;
            this->camera->resetView();
        }

        ImGui::ColorEdit4("Color", (float*)&color);
        ImGui::DragFloat("Rotate", &rotation);

        ImGui::Separator();

        if (ImGui::Button("Save", {100.0f, 0}))
        {
            const auto& pixels = this->fbResolved->readPixel();
            stbi_flip_vertically_on_write(true);
            stbi_write_png(".data/picture-scene.png", width, height, 4, pixels.data(), width * 4);

            auto workingDir = std::filesystem::current_path().u8string();
            LOGI("Save to picture: {}/{}", (const char*)workingDir.data(), ".data/picture-scene.png");
        }

    }
    ImGui::End();
}

void ModelScene::onMouseEvent(const MouseEvent* e)
{
    if (e->mouseButton == MouseEvent::kButtonLeft)
    {
        if (e->mouseEventType == MouseEvent::kMousePress)
        {
            this->holdLeftButton = true;
        }
        else if (e->mouseEventType == MouseEvent::kMouseRelease)
        {
            this->holdLeftButton = false;
        }
    }

    if (e->mouseButton == MouseEvent::kButtonMiddle)
    {
        if (e->mouseEventType == MouseEvent::kMousePress)
        {
            this->holdMidButton = true;
        }
        else if (e->mouseEventType == MouseEvent::kMouseRelease)
        {
            this->holdMidButton = false;
        }
    }

    if (e->mouseEventType == MouseEvent::kMouseScroll)
    {
        this->camera->forward((float)e->scrollY*20.0f);
    }
    else if (e->mouseEventType == MouseEvent::kMouseMove)
    {
        auto delta = e->posDelta;
        if (this->holdLeftButton)
        {
            this->camera->round(delta.x, delta.y);
        }
        else if(this->holdMidButton)
        {
            this->camera->move({delta.x, -delta.y, 0});
        }
    }
}