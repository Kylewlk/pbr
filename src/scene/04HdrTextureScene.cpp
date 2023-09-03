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
    this->shaderCubMapLod = Shader::createByPath("asset/shader/cubemap.vert", "asset/shader/cubemap_lod.frag");
    this->shaderIrradiance = Shader::createByPath("asset/shader/cubemap.vert", "asset/shader/cubemap_irradiance.frag");
    this->shaderPrefilter = Shader::createByPath("asset/shader/cubemap.vert", "asset/shader/cubemap_prefilter.frag");
    this->shaderSkyBox = Shader::createByPath("asset/shader/sky_box.vert", "asset/shader/sky_box.frag");
    this->shaderUnfold = Shader::createByPath("asset/shader/cubemap_unfold.vert", "asset/shader/cubemap_unfold.frag");
    this->shaderUnfoldLod = Shader::createByPath("asset/shader/cubemap_unfold.vert", "asset/shader/cubemap_unfold_lod.frag");
    this->shaderBrdf = Shader::createByPath("asset/shader/brdf.vert", "asset/shader/brdf.frag");

    this->roomHdr = Texture::createHDR("asset/room.hdr");
    this->createCubMap(roomHdr, roomCubeMap);
    this->createIrradiance(this->roomCubeMap, this->roomIrradiance);
    this->createPrefilter(this->roomCubeMap, this->roomPrefilter);
    this->textureHdr = this->roomHdr;
    this->textureCubeMap = this->roomCubeMap;
    this->textureIrradiance = this->roomIrradiance;
    this->texturePrefilter = this->roomPrefilter;

    this->createBrdfLut();

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
    if (drawType == 1 || drawType == 2 || drawType == 4 || drawType == kCubePrefilter) // draw cube
    {
        this->camera->round(50, 0);
        this->camera->round(0, -50);
    }
    this->camera2d->resetView();
}

void HdrTextureScene::draw()
{
    this->camera->setViewSize((float)this->width, (float)this->height);
    this->camera->update();
    this->camera2d->setViewSize((float)this->width, (float)this->height);
    this->camera2d->update();

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    if (this->enableCubeMapSampless)
    {
        // cube map 两个面连接处，可以使用两个面的纹理数据插值采样
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    }
    else
    {
        glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    }

    float modelScale = 100;

    if (drawType == 0)
    {
        shaderTexLinear->use();
        auto mat = camera2d->getViewProj() * math::scale({(float)textureHdr->getWidth() * 0.3f, -(float)textureHdr->getHeight() * 0.3f, 1.0f});
        glUniformMatrix4fv(1, 1, false, (float*)&mat);
        glUniform4f(2, 1, 1, 1, 1);
        shaderTexLinear->bindTexture(3, this->textureHdr);
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
    else if(drawType == 8)
    {
        drawUnfold(this->textureCubeMap);
    }
    else if(drawType == 9)
    {
        drawUnfold(this->textureIrradiance);
    }
    else if(drawType == kCubePrefilter || drawType == kSpherePrefilter)
    {
        drawCubMap(this->texturePrefilter, modelScale, drawType == kCubePrefilter, (float)this->drawPrefilterLevel);
    }
    else if(drawType == kSkyBoxPrefilter)
    {
        auto viewSize = camera->getViewSize();
        auto proj = math::perspective(camera->getFov(), viewSize.x/viewSize.y, 0.01, 4.0); // 调整near 和 far
        auto view = math::Mat4{math::Mat3{camera->getView()}}; // 去掉位移，将摄像机移动到圆点
        shaderCubMapLod->use();
        shaderCubMapLod->setUniform("viewProj", proj*view);
        shaderCubMapLod->bindTexture("cubeMap", texturePrefilter);
        shaderCubMapLod->setUniform("lod", (float)this->drawPrefilterLevel);
        renderCube();
    }
    else if(drawType == kUnfoldPrefilter)
    {
        shaderUnfoldLod->use();
        auto mat = camera2d->getViewProj() * math::scale({200, 200, 1.0}) * math::translate({-2, -1.5, 0});
        shaderUnfoldLod->setUniform("mvp", mat);
        shaderUnfoldLod->bindTexture("cubeMap", texturePrefilter);
        shaderUnfoldLod->setUniform("lod", (float)this->drawPrefilterLevel);
        renderUnfoldCube();
    }
    else if (drawType == kBrdfLUT)
    {
        shaderTexLinear->use();
        auto mat = camera2d->getViewProj()
                   * math::scale({(float)brdfLUT->getWidth() * 0.5f , (float)brdfLUT->getHeight() * 0.5f, 1.0f});
        glUniformMatrix4fv(1, 1, false, (float*)&mat);
        glUniform4f(2, 1, 1, 1, 1);
        shaderTexLinear->bindTexture(3, this->brdfLUT);
        drawQuad();
    }

}

void HdrTextureScene::drawCubMap(const TextureRef& cubeMap, float scale, bool isCube, float lod /*= -1.0f*/)
{
    bool useLod = lod >= 0;
    auto shader = useLod ? shaderCubMapLod : shaderCubMap;

    shader->use();
    auto mat = camera->getViewProj() * math::scale({scale, scale, scale});
    shader->setUniform("viewProj", mat);
    shader->bindTexture("cubeMap", cubeMap);
    if (useLod)
    {
        shader->setUniform("lod", lod);
    }
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
//    shaderSkyBox->use();
//    glDisable(GL_DEPTH_TEST);
//    shaderSkyBox->setUniform("projection", camera->getProj());
//    shaderSkyBox->setUniform("view", camera->getView());
//    shaderSkyBox->bindTexture("environmentMap", cubeMap);

    auto viewSize = camera->getViewSize();
    auto proj = math::perspective(camera->getFov(), viewSize.x/viewSize.y, 0.01, 4.0); // 调整near 和 far
    auto view = math::Mat4{math::Mat3{camera->getView()}}; // 去掉位移，将摄像机移动到圆点
    shaderCubMap->use();
    shaderCubMap->setUniform("viewProj", proj*view);
    shaderCubMap->bindTexture("cubeMap", cubeMap);
    renderCube();
}

void HdrTextureScene::drawUnfold(const TextureRef& cubeMap)
{
    shaderUnfold->use();
    auto mat = camera2d->getViewProj() * math::scale({200, 200, 1.0}) * math::translate({-2, -1.5, 0});
    shaderUnfold->setUniform("mvp", mat);
    shaderUnfold->bindTexture("cubeMap", cubeMap);
    renderUnfoldCube();
}

void HdrTextureScene::drawSettings()
{
    if(ImGui::RadioButton("HDR Room", &hdrType, 0))
    {
        if (roomHdr == nullptr)
        {
            this->roomHdr = Texture::createHDR("asset/room.hdr");
            this->createCubMap(roomHdr, roomCubeMap);
            this->createIrradiance(roomCubeMap, roomIrradiance);
            this->createPrefilter(roomCubeMap, roomPrefilter);
        }
        this->textureHdr = roomHdr;
        this->textureCubeMap = roomCubeMap;
        this->textureIrradiance = roomIrradiance;
        this->texturePrefilter = this->roomPrefilter;
    }
    if(ImGui::RadioButton("HDR Sky", &hdrType, 1))
    {
        if (skyHdr == nullptr)
        {
            this->skyHdr = Texture::createHDR("asset/sky.hdr");
            this->createCubMap(skyHdr, skyCubeMap);
            this->createIrradiance(skyCubeMap, skyIrradiance);
            this->createPrefilter(skyCubeMap, skyPrefilter);
        }
        this->textureHdr = skyHdr;
        this->textureCubeMap = skyCubeMap;
        this->textureIrradiance = skyIrradiance;
        this->texturePrefilter = this->skyPrefilter;
    }
    ImGui::Separator();
    ImGui::Checkbox("Enable Cube Map Sampless", &this->enableCubeMapSampless);
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
    changeShowType = ImGui::RadioButton("Unfold Environment", &drawType, 8) || changeShowType;
    changeShowType = ImGui::RadioButton("Unfold Irradiance", &drawType, 9) || changeShowType;
    ImGui::Separator();
    changeShowType = ImGui::RadioButton("Cube Prefilter", &drawType, kCubePrefilter) || changeShowType;
    changeShowType = ImGui::RadioButton("Sphere Prefilter", &drawType, kSpherePrefilter) || changeShowType;
    changeShowType = ImGui::RadioButton("SkyBox Prefilter", &drawType, kSkyBoxPrefilter) || changeShowType;
    changeShowType = ImGui::RadioButton("Unfold Prefilter", &drawType, kUnfoldPrefilter) || changeShowType;
    ImGui::RadioButton("0", &drawPrefilterLevel, 0);
    for (int i = 1; i < prefilterLevels; ++i)
    {
        ImGui::SameLine(0, 5);
        ImGui::RadioButton(std::to_string(i).c_str(), &drawPrefilterLevel, i);
    }

    ImGui::Separator();
    changeShowType = ImGui::RadioButton("BRDF LUT", &drawType, kBrdfLUT) || changeShowType;

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
    const int mapSize = 512;
    cubeMap = Texture::createCubemap(GL_RGBA16F, mapSize, mapSize, -1);

    FrameBufferRef frameBuffer = FrameBuffer::create(mapSize, mapSize, RenderTarget::kNone, RenderTarget::kRenderDepth);

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

    glEnable(GL_DEPTH_TEST);
    shaderHdrToCubeMap->use();
    shaderHdrToCubeMap->bindTexture("sphericalMap", hdr);

    for (int i = 0; i < 6; ++i)
    {
        frameBuffer->bind(cubeMap, 0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderHdrToCubeMap->setUniform("viewProj", project*views[i]);
        renderCube();
        frameBuffer->unbind();
    }

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    cubeMap->setSampler(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
}

void HdrTextureScene::createIrradiance(const TextureRef& cubeMap, TextureRef& irradiance)
{
    const int mapSize = 64;
    irradiance = Texture::createCubemap(GL_RGBA16F, mapSize, mapSize);

    FrameBufferRef frameBuffer = FrameBuffer::create(mapSize, mapSize, RenderTarget::kNone, RenderTarget::kRenderDepth);

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

    glEnable(GL_DEPTH_TEST);
    shaderIrradiance->use();

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

void HdrTextureScene::createPrefilter(const TextureRef& cubeMap, TextureRef& prefilter)
{
    const int mapSize = 128;
    prefilter = Texture::createCubemap(GL_RGBA16F, mapSize, mapSize, prefilterLevels);
    FrameBufferRef frameBuffer = FrameBuffer::create(mapSize, mapSize, RenderTarget::kNone, RenderTarget::kRenderDepth);

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

    // cube map 两个面连接处，可以使用两个面的纹理数据插值采样
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glEnable(GL_DEPTH_TEST);
    shaderPrefilter->use();
    shaderPrefilter->bindTexture("environmentMap", cubeMap);

    for (int mip = 0; mip < prefilterLevels; ++mip)
    {
        float roughness = (float)mip / (float)(prefilterLevels - 1);
        shaderPrefilter->setUniform("roughness", roughness);
        for (int i = 0; i < 6; ++i)
        {
            frameBuffer->bind(prefilter, mip, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            shaderPrefilter->setUniform("viewProj", project*views[i]);
            renderCube();
            frameBuffer->unbind();
        }
    }
    prefilter->setSampler(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
}

void HdrTextureScene::createBrdfLut()
{
    constexpr int lutSize = 512;
    this->brdfLUT = Texture::create(GL_RG16F, lutSize, lutSize);
    FrameBufferRef frameBuffer = FrameBuffer::create(lutSize, lutSize, RenderTarget::kNone, RenderTarget::kNone);

    frameBuffer->bind(brdfLUT, 0);
    shaderBrdf->use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawQuad();
    frameBuffer->unbind();

}