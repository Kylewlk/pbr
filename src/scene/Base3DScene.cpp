//
// Created by DELL on 2023/8/30.
//

#include "Base3DScene.h"
#include "common/EventSystem.h"
#include "camera/Camera3D.h"
#include "common/Logger.h"
#include "common/FrameBuffer.h"

Base3DScene::Base3DScene(const char* ID, int width, int height, bool multisample)
    : Scene(ID, width, height, multisample)
{
    this->camera = Camera3D::create();
    this->camera->setLockUp(true);
}

void Base3DScene::drawProperty()
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
            this->reset();
        }
        ImGui::SameLine(0, 20);
        if (ImGui::Button("Save Picture"))
        {
            const auto& pixels = this->fbResolved->readPixel();
            stbi_flip_vertically_on_write(true);
            stbi_write_png(".data/picture-scene.png", width, height, 4, pixels.data(), width * 4);

            auto workingDir = std::filesystem::current_path().u8string();
            LOGI("Save to picture: {}/{}", (const char*)workingDir.data(), ".data/picture-scene.png");
        }
        ImGui::SliderFloat("Exposure", &toneMappingExposure, 0.01f, 4.0f);

        ImGui::Separator();

        this->drawSettings();
    }
    ImGui::End();
}


void Base3DScene::onMouseEvent(const MouseEvent* e)
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
            this->camera->round(delta.x, -delta.y);
        }
        else if(this->holdMidButton)
        {
            this->camera->move({delta.x, -delta.y, 0});
        }
    }
}
