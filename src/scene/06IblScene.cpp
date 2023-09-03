//
// Created by wlk12 on 2023/8/6.
//

#include "06IblScene.h"
#include "common/Texture.h"
#include "common/Shader.h"
#include "common/FrameBuffer.h"
#include "common/EventSystem.h"
#include "camera/Camera2D.h"
#include "camera/Camera3D.h"
#include "common/Logger.h"
#include "common/RenderModel.h"

IblScene::IblScene(int width, int height)
    : Base3DScene(ID, width, height, true)
{
    this->shaderIBL = Shader::createByPath("asset/shader/model.vert", "asset/shader/06ibl.frag");
    this->shaderLight = Shader::createByPath("asset/shader/model.vert", "asset/shader/model.frag");

    this->shaderSkyBox = Shader::createByPath("asset/shader/cubemap.vert", "asset/shader/cubemap.frag");
    this->shaderHdrToCubeMap = Shader::createByPath("asset/shader/cubemap.vert", "asset/shader/cubemap_from_hdr.frag");
    this->shaderIrradiance = Shader::createByPath("asset/shader/cubemap.vert", "asset/shader/cubemap_irradiance.frag");
    this->shaderPrefilter = Shader::createByPath("asset/shader/cubemap.vert", "asset/shader/cubemap_prefilter.frag");
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

    IblScene::reset();
}

SceneRef IblScene::create()
{
    struct enable_make_shared : public IblScene
    {
        enable_make_shared() : IblScene(0, 0) {}
    };
    return std::make_shared<enable_make_shared>();
}

void IblScene::reset()
{
    this->camera->resetView();
    if (drawType == 1) // draw cube
    {
        this->camera->round(50, 0);
        this->camera->round(0, -50);
    }
    else if (drawType == 2) // ball group
    {
        this->camera->forward(-700);
    }

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

//    this->enableIbl = true;

    this->albedo = {0.4f, 0.7f, 0.6f};
    this->roughness = {0.3};
    this->metallic = {0.2};
    this->ao = {1.0};


}

void IblScene::draw()
{
    this->camera->setViewSize((float)this->width, (float)this->height);
    this->camera->update();

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // 绘制天空盒子，需要关闭深度缓存写入
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    if (backgroundType == 0){
        drawSkyBox(this->textureCubeMap);
    }
    else if(backgroundType == 1)
    {
        drawSkyBox(this->textureIrradiance);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    shaderIBL->use();
    shaderIBL->setUniform("viewProj", camera->getViewProj());
    shaderIBL->setUniform("camPos", camera->getViewPosition());
    shaderIBL->setUniform("lightColors", lightColors, lightCount);
    shaderIBL->setUniform("lightPositions", lightPositions, lightCount);
    shaderIBL->setUniform("lightEnables", lightEnables, lightCount);
    shaderIBL->bindTexture("irradianceMap", this->textureIrradiance);
    shaderIBL->bindTexture("prefilterMap", this->texturePrefilter);
    shaderIBL->bindTexture("brdfLUT", this->brdfLUT);
//    shaderIBL->setUniform("enableIbl", this->enableIbl);

    auto mat = math::scale({50, 50, 50});
    auto normalMat = glm::transpose(glm::inverse(math::Mat3{mat}));
    shaderIBL->setUniform("model", mat);
    shaderIBL->setUniform("normalMatrix", normalMat);

    shaderIBL->setUniform("albedo", albedo);
    shaderIBL->setUniform("metallic", metallic);
    shaderIBL->setUniform("roughness", roughness);
    shaderIBL->setUniform("ao", ao);

    if (drawType == 0)
    {
        renderSphere();
    }
    else if(drawType == 1)
    {
        renderCube();
    }
    else if (drawType == 2)
    {
        math::Mat4 model{1};
        const int rows    = 7;
        const int columns = 7;
        float spacing = 120;

        for (int i = 0; i < rows; ++i)
        {
            shaderIBL->setUniform("metallic", (float)i / (float)rows);
            for (int j = 0; j < columns; ++j)
            {
                shaderIBL->setUniform("roughness", ((float)j / (float)columns) * 0.95f + 0.05f);
                model = math::translate(glm::vec3((float(j) - (columns / 2.0f)) * spacing,
                                                  (float(i) - (rows / 2.0f)) * spacing, 0.0f)) * mat;
                shaderIBL->setUniform("model", model);
                renderSphere();
            }
        }
    }

    shaderLight->use();
    shaderLight->setUniform("viewProj", camera->getViewProj());
    shaderLight->setUniform("lightColor", math::Vec3 {0.5, 0.5, 0.5});
    shaderLight->setUniform("lightDir", glm::normalize(math::Vec3{1, 1, 0.5}));
    shaderLight->setUniform("cameraPos", camera->getViewPosition());

    for (int i = 0; i < lightCount; ++i)
    {
        if (lightEnables[i])
        {
            shaderLight->setUniform("albedo", math::Vec3{1, 1, 1});
        }
        else
        {
            shaderLight->setUniform("albedo", math::Vec3{0.1, 0.3, 0.2});
        }

        mat = math::translate(lightPositions[i]) * math::scale({10, 10, 10});
        normalMat = glm::transpose(glm::inverse(math::Mat3{mat}));
        shaderLight->setUniform("model", mat);
        shaderLight->setUniform("normalMatrix", normalMat);
        renderSphere();
    }

}

void IblScene::drawSkyBox(const TextureRef& cubeMap)
{
    //    shaderSkyBox->use();
    //    glDisable(GL_DEPTH_TEST);
    //    shaderSkyBox->setUniform("projection", camera->getProj());
    //    shaderSkyBox->setUniform("view", camera->getView());
    //    shaderSkyBox->bindTexture("environmentMap", cubeMap);

    auto viewSize = camera->getViewSize();
    auto proj = math::perspective(camera->getFov(), viewSize.x/viewSize.y, 0.01, 4.0); // 调整near 和 far
    auto view = math::Mat4{math::Mat3{camera->getView()}}; // 去掉位移，将摄像机移动到圆点
    shaderSkyBox->use();
    shaderSkyBox->setUniform("viewProj", proj*view);
    shaderSkyBox->bindTexture("cubeMap", cubeMap);
    renderCube();
}

void IblScene::drawSettings()
{
    if(ImGui::CollapsingHeader("Lights"))
    {
        for (int i = 0; i < lightCount; ++i)
        {
            std::string name = "Enable Light ";
            name += std::to_string(i);

            ImGui::Checkbox(name.c_str(), &lightEnables[i]);
        }
    }

    if (ImGui::Button("All Light"))
    {
        for (auto& lightEnable : lightEnables)
        {
            lightEnable = true;
        }
    }
    ImGui::SameLine(0, 10);
    if (ImGui::Button("Half Light"))
    {
        for (int i = 0; i < lightCount; ++i)
        {
            lightEnables[i] = i < 4;
        }
    }
    ImGui::SameLine(0, 10);
    if (ImGui::Button("1 Light"))
    {
        for (auto& lightEnable : lightEnables)
        {
            lightEnable = false;
        }
        lightEnables[1] = true;
    }
    ImGui::SameLine(0, 10);
    if (ImGui::Button("No light"))
    {
        for (auto& lightEnable : lightEnables)
        {
            lightEnable = false;
        }
    }

    ImGui::Separator();
    ImGui::ColorEdit3("albedo", (float*)&albedo, ImGuiColorEditFlags_Float);
    ImGui::SliderFloat("ao", &ao, 0.0, 1.0);
    if (this->drawType != 2)
    {
        ImGui::SliderFloat("roughness", &roughness, 0.0001, 1.0);
        ImGui::SliderFloat("metallic", &metallic, 0.0001, 1.0);
    }

    ImGui::Separator();
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
//    ImGui::Checkbox("Enable IBL", &this->enableIbl);
//    ImGui::Checkbox("Enable IBL", &this->enableIbl);

    ImGui::Separator();
    ImGui::Text("Background");
    ImGui::RadioButton("Environment", &backgroundType, 0);
    ImGui::RadioButton("Irradiance", &backgroundType, 1);
    ImGui::RadioButton("None", &backgroundType, 2);

    ImGui::Separator();
    bool changeShowType {false};
    changeShowType = ImGui::RadioButton("Sphere", &drawType, 0) || changeShowType;
    changeShowType = ImGui::RadioButton("Cube", &drawType, 1) || changeShowType;
    changeShowType = ImGui::RadioButton("Sphere Group", &drawType, 2) || changeShowType;
    if (changeShowType)
    {
        this->reset();
    }
}


void IblScene::createCubMap(const TextureRef& hdr, TextureRef& cubeMap)
{
    const int mapSize = 512;
    cubeMap = Texture::createCubemap(GL_RGBA16F, mapSize, mapSize, -1);

    FrameBufferRef frameBuffer = FrameBuffer::create(mapSize, mapSize, RenderTarget::kNone, RenderTarget::kNone);

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

void IblScene::createIrradiance(const TextureRef& cubeMap, TextureRef& irradiance)
{
    const int mapSize = 64;
    irradiance = Texture::createCubemap(GL_RGBA16F, mapSize, mapSize);

    FrameBufferRef frameBuffer = FrameBuffer::create(mapSize, mapSize, RenderTarget::kNone, RenderTarget::kNone);

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

void IblScene::createPrefilter(const TextureRef& cubeMap, TextureRef& prefilter)
{
    const int mapSize = 128;
    prefilter = Texture::createCubemap(GL_RGBA16F, mapSize, mapSize, prefilterLevels);
    FrameBufferRef frameBuffer = FrameBuffer::create(mapSize, mapSize, RenderTarget::kNone, RenderTarget::kNone);

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

void IblScene::createBrdfLut()
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

