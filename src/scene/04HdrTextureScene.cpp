//
// Created by wlk12 on 2023/8/6.
//

#include "04HdrTextureScene.h"
#include "common/Texture.h"
#include "common/Shader.h"
#include "common/FrameBuffer.h"
#include "common/EventSystem.h"
#include "camera/Camera2D.h"
#include "camera/Camera3D.h"
#include "common/Logger.h"
#include "common/RenderModel.h"

HdrTextureScene::HdrTextureScene(int width, int height)
    : Base3DScene(ID, width, height, true)
{
    this->shader = Shader::createByPath("asset/shader/model.vert", "asset/shader/model.frag");

    this->roomHdr = Texture::createHDR("asset/room.hdr");
    this->textureHdr = this->roomHdr;
    this->shaderTexLinear = Shader::createByPath("asset/shader/picture.vert", "asset/shader/picture_linear.frag");
    this->camera2d = Camera2D::create();

    HdrTextureScene::reset();
}

SceneRef HdrTextureScene::create()
{
    struct enable_make_shared : public HdrTextureScene
    {
        enable_make_shared() : HdrTextureScene(0, 0) {}
    };
    return std::make_shared<enable_make_shared>();
}

void HdrTextureScene::reset()
{
    this->camera->resetView();
    if (drawType == 1) // draw cube
    {
        this->camera->round(50, 0);
        this->camera->round(0, -50);
    }
    this->camera2d->resetView();

    this->lightColor = {0.9, 0.9, 0.9};
    this->lightDir = {1, 1, 0.3};

    this->sphereColor = math::Vec3{0.3, 0.7, 0.5};
    this->cubeColor = math::Vec3{0.2, 0.6, 0.8};
}

void HdrTextureScene::draw()
{
    this->camera->setViewSize((float)this->width, (float)this->height);
    this->camera->update();
    this->camera2d->setViewSize((float)this->width, (float)this->height);
    this->camera2d->update();

    glEnable(GL_MULTISAMPLE);

    if (drawType == 0)
    {
        shaderTexLinear->use();
        auto mat = camera2d->getViewProj() * math::scale({(float)textureHdr->getWidth() * 0.3f, -(float)textureHdr->getHeight() * 0.3f, 1.0f});
        glUniformMatrix4fv(1, 1, false, (float*)&mat);
        glUniform4f(2, 1, 1, 1, 1);
        shaderTexLinear->bindTexture(3, this->textureHdr);
        glEnable(GL_MULTISAMPLE);
        drawQuad();
        return;
    }

    glEnable(GL_DEPTH_TEST);

    shader->use();
    shader->setUniform("viewProj", camera->getViewProj());
    shader->setUniform("lightColor", lightColor);
    shader->setUniform("lightDir", glm::normalize(lightDir));
    shader->setUniform("cameraPos", camera->getViewPosition());
    shader->setUniform("albedo", sphereColor);

    auto mat = math::scale({100, 100, 100});
    auto normalMat = glm::transpose(glm::inverse(math::Mat3{mat}));
    shader->setUniform("model", mat);
    shader->setUniform("normalMatrix", normalMat);

    if (drawType == 1)
    {
        renderCube();
    }
    else if (drawType == 2)
    {
        renderSphere();
    }
}

void HdrTextureScene::drawSettings()
{
    ImGui::ColorEdit3("Light Color", (float*)&lightColor, ImGuiColorEditFlags_Float);
    ImGui::DragFloat3("Light Direction", (float*)&lightDir);
    ImGui::ColorEdit3("Sphere Color", (float*)&sphereColor, ImGuiColorEditFlags_Float);
    ImGui::ColorEdit3("Cube Color", (float*)&cubeColor, ImGuiColorEditFlags_Float);

    ImGui::Separator();
    if(ImGui::RadioButton("HDR Room", &hdrType, 0))
    {
        if (roomHdr == nullptr)
        {
            this->roomHdr = Texture::createHDR("asset/room.hdr");
        }
        this->textureHdr = roomHdr;
    }
    if(ImGui::RadioButton("HDR Sky", &hdrType, 1))
    {
        if (skyHdr == nullptr)
        {
            this->skyHdr = Texture::createHDR("asset/sky.hdr");
        }
        this->textureHdr = skyHdr;
    }

    ImGui::Separator();
    bool changeShowType {false};
    changeShowType = ImGui::RadioButton("HDR Texture", &drawType, 0) || changeShowType;
    changeShowType = ImGui::RadioButton("Cube", &drawType, 1) || changeShowType;
    changeShowType = ImGui::RadioButton("Sphere", &drawType, 2) || changeShowType;
    if (changeShowType)
    {
        this->reset();
    }
}

void HdrTextureScene::onMouseEvent(const MouseEvent* e)
{
    Base3DScene::onMouseEvent(e);

    if (e->mouseEventType == MouseEvent::kMouseScroll)
    {
        float scale = this->camera2d->getViewScale();
        scale = (e->scrollY > 0) ? scale * 0.8f : scale * 1.25f;
        this->camera2d->setViewScale(scale);
    }
    else if (e->mouseEventType == MouseEvent::kMouseMove)
    {
        if (this->holdLeftButton)
        {
            auto delta = e->posDelta;
            delta *= this->camera2d->getViewScale();
            this->camera2d->move({delta.x, -delta.y, 0});
        }
    }
}