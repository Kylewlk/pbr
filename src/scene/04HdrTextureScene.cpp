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
    this->shaderTexLinear = Shader::createByPath("asset/shader/picture.vert", "asset/shader/picture_linear.frag");
    this->shaderHdrToCubeMap = Shader::createByPath("asset/shader/cubemap.vert", "asset/shader/cubemap_from_hdr.frag");
    this->shaderCubMap = Shader::createByPath("asset/shader/cubemap.vert", "asset/shader/cubemap.frag");
    this->shaderIrradiance = Shader::createByPath("asset/shader/cubemap.vert", "asset/shader/cubemap_irradiance.frag");
    this->shaderSkyBox = Shader::createByPath("asset/shader/sky_box.vert", "asset/shader/sky_box.frag");

    this->roomHdr = Texture::createHDR("asset/room.hdr");
    this->createCubMap(roomHdr, roomCubeMap);
    this->createIrradiance(this->roomCubeMap, this->roomIrradiance);
    this->textureHdr = this->roomHdr;
    this->textureCubeMap = this->roomCubeMap;
    this->textureIrradiance = roomIrradiance;

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
    if (drawType == 1 || drawType == 2 || drawType == 4) // draw cube
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
    glEnable(GL_DEPTH_TEST);
    float modelScale = 100;

    if (drawType == 0)
    {
        shaderTexLinear->use();
        auto mat = camera2d->getViewProj() * math::scale({(float)textureHdr->getWidth() * 0.3f, -(float)textureHdr->getHeight() * 0.3f, 1.0f});
        glUniformMatrix4fv(1, 1, false, (float*)&mat);
        glUniform4f(2, 1, 1, 1, 1);
        shaderTexLinear->bindTexture(3, this->textureHdr);
        glEnable(GL_MULTISAMPLE);
        drawQuad();
    }
    else if (drawType == 1)
    {
        shaderHdrToCubeMap->use();
        auto mat = camera->getViewProj() * math::scale({modelScale, modelScale, modelScale});
        shaderHdrToCubeMap->setUniform("viewProj", mat);
        shaderHdrToCubeMap->bindTexture("sphericalMap", this->textureHdr);
        renderCube();
    }
    else if (drawType == 2 || drawType == 3)
    {
        drawCubMap(this->textureCubeMap, modelScale, drawType == 2);
    }
    else if(drawType == 4 || drawType == 5)
    {
        drawCubMap(this->textureIrradiance, modelScale, drawType == 4);
    }
    else if(drawType == 6)
    {
        drawSkyBox(this->textureCubeMap);
    }
    else if(drawType == 7)
    {
        drawSkyBox(this->textureIrradiance);
    }
}

void HdrTextureScene::drawCubMap(const TextureRef& cubeMap, float scale, bool isCube)
{
    shaderCubMap->use();
    auto mat = camera->getViewProj() * math::scale({scale, scale, scale});
    shaderCubMap->setUniform("viewProj", mat);
    shaderCubMap->bindTexture("cubeMap", cubeMap);
    if (isCube)
    {
        renderCube();
    }
    else
    {
        renderSphere();
    }
}

void HdrTextureScene::drawSkyBox(const TextureRef& cubeMap)
{
    shaderSkyBox->use();
    glDisable(GL_DEPTH_TEST);

    auto mat = camera->getViewProj();
    shaderSkyBox->setUniform("projection", camera->getProj());
    shaderSkyBox->setUniform("view", camera->getView());
    shaderSkyBox->bindTexture("environmentMap", cubeMap);
    renderCube();
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
            this->createCubMap(roomHdr, roomCubeMap);
            this->createIrradiance(roomCubeMap, roomIrradiance);
        }
        this->textureHdr = roomHdr;
        this->textureCubeMap = roomCubeMap;
        this->textureIrradiance = roomIrradiance;
    }
    if(ImGui::RadioButton("HDR Sky", &hdrType, 1))
    {
        if (skyHdr == nullptr)
        {
            this->skyHdr = Texture::createHDR("asset/sky.hdr");
            this->createCubMap(skyHdr, skyCubeMap);
            this->createIrradiance(skyCubeMap, skyIrradiance);
        }
        this->textureHdr = skyHdr;
        this->textureCubeMap = skyCubeMap;
        this->textureIrradiance = skyIrradiance;
    }

    ImGui::Separator();
    bool changeShowType {false};
    changeShowType = ImGui::RadioButton("HDR Texture", &drawType, 0) || changeShowType;
    changeShowType = ImGui::RadioButton("Cube Map", &drawType, 1) || changeShowType;
    changeShowType = ImGui::RadioButton("Cube Environment", &drawType, 2) || changeShowType;
    changeShowType = ImGui::RadioButton("Sphere Environment", &drawType, 3) || changeShowType;
    changeShowType = ImGui::RadioButton("Cube Irradiance", &drawType, 4) || changeShowType;
    changeShowType = ImGui::RadioButton("Sphere Irradiance", &drawType, 5) || changeShowType;
    changeShowType = ImGui::RadioButton("SkyBox Environment", &drawType, 6) || changeShowType;
    changeShowType = ImGui::RadioButton("SkyBox Irradiance", &drawType, 7) || changeShowType;
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

void HdrTextureScene::createCubMap(const TextureRef& hdr, TextureRef& cubeMap)
{
    cubeMap = Texture::createCubemap(GL_RGBA16F, 512, 512);

    FrameBufferRef frameBuffer = FrameBuffer::create(512, 512, RenderTarget::kNone, RenderTarget::kRenderDepth);

    using namespace math;

    // 参考 https://blog.csdn.net/wlk1229/article/details/85077819
    Mat4 project = math::perspective(glm::radians(90.0f), 1, 0.1, 10.0);
    Mat4 views[]{
        lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 1.0f,  0.0f,  0.0f), Vec3(0.0f, -1.0f,  0.0f)),
        lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3(-1.0f,  0.0f,  0.0f), Vec3(0.0f, -1.0f,  0.0f)),
        lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f,  1.0f,  0.0f), Vec3(0.0f,  0.0f,  1.0f)),
        lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f, -1.0f,  0.0f), Vec3(0.0f,  0.0f, -1.0f)),
        lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f,  0.0f,  1.0f), Vec3(0.0f, -1.0f,  0.0f)),
        lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f,  0.0f, -1.0f), Vec3(0.0f, -1.0f,  0.0f))
    };

    shaderHdrToCubeMap->use();
    glEnable(GL_DEPTH_TEST);

    for (int i = 0; i < 6; ++i)
    {
        frameBuffer->bind(cubeMap, 0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderHdrToCubeMap->setUniform("viewProj", project*views[i]);
        shaderHdrToCubeMap->bindTexture("sphericalMap", hdr);
        renderCube();
        frameBuffer->unbind();
    }
}

void HdrTextureScene::createIrradiance(const TextureRef& cubeMap, TextureRef& irradiance)
{
    irradiance = Texture::createCubemap(GL_RGBA16F, 64, 64);

    FrameBufferRef frameBuffer = FrameBuffer::create(64, 64, RenderTarget::kNone, RenderTarget::kRenderDepth);

    using namespace math;

    // 参考 https://blog.csdn.net/wlk1229/article/details/85077819
    Mat4 project = math::perspective(glm::radians(90.0f), 1, 0.1, 10.0);
    Mat4 views[]{
        lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 1.0f,  0.0f,  0.0f), Vec3(0.0f, -1.0f,  0.0f)),
        lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3(-1.0f,  0.0f,  0.0f), Vec3(0.0f, -1.0f,  0.0f)),
        lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f,  1.0f,  0.0f), Vec3(0.0f,  0.0f,  1.0f)),
        lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f, -1.0f,  0.0f), Vec3(0.0f,  0.0f, -1.0f)),
        lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f,  0.0f,  1.0f), Vec3(0.0f, -1.0f,  0.0f)),
        lookAt(Vec3(0.0f, 0.0f, 0.0f), Vec3( 0.0f,  0.0f, -1.0f), Vec3(0.0f, -1.0f,  0.0f))
    };

    shaderIrradiance->use();
    glEnable(GL_DEPTH_TEST);

    for (int i = 0; i < 6; ++i)
    {
        frameBuffer->bind(irradiance, 0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderIrradiance->setUniform("viewProj", project*views[i]);
        shaderIrradiance->bindTexture("environmentMap", cubeMap);
        renderCube();
        frameBuffer->unbind();
    }
}