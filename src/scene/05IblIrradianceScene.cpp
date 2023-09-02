//
// Created by wlk12 on 2023/8/6.
//

#include "05IblIrradianceScene.h"
#include "common/Texture.h"
#include "common/Shader.h"
#include "common/FrameBuffer.h"
#include "common/EventSystem.h"
#include "camera/Camera2D.h"
#include "camera/Camera3D.h"
#include "common/Logger.h"
#include "common/RenderModel.h"

IblIrradianceScene::IblIrradianceScene(int width, int height)
    : Base3DScene(ID, width, height, true)
{
    this->shaderModel = Shader::createByPath("asset/shader/model.vert", "asset/shader/05ibl_irradiance.frag");
    this->shaderLight = Shader::createByPath("asset/shader/model.vert", "asset/shader/model.frag");

    this->shaderHdrToCubeMap = Shader::createByPath("asset/shader/cubemap.vert", "asset/shader/cubemap_from_hdr.frag");
    this->shaderSkyBox = Shader::createByPath("asset/shader/cubemap.vert", "asset/shader/cubemap.frag");
    this->shaderIrradiance = Shader::createByPath("asset/shader/cubemap.vert", "asset/shader/cubemap_irradiance.frag");

    this->roomHdr = Texture::createHDR("asset/room.hdr");
    this->createCubMap(roomHdr, roomCubeMap);
    this->createIrradiance(this->roomCubeMap, this->roomIrradiance);

    this->textureHdr = this->roomHdr;
    this->textureCubeMap = this->roomCubeMap;
    this->textureIrradiance = roomIrradiance;

    IblIrradianceScene::reset();
}

SceneRef IblIrradianceScene::create()
{
    struct enable_make_shared : public IblIrradianceScene
    {
        enable_make_shared() : IblIrradianceScene(0, 0) {}
    };
    return std::make_shared<enable_make_shared>();
}

void IblIrradianceScene::reset()
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

    this->enableIbl = true;

    this->albedo = {0.4f, 0.7f, 0.6f};
    this->roughness = {0.3};
    this->metallic = {0.2};
    this->ao = {1.0};
}

void IblIrradianceScene::draw()
{
    this->camera->setViewSize((float)this->width, (float)this->height);
    this->camera->update();

    glEnable(GL_MULTISAMPLE);

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
    shaderModel->use();
    shaderModel->setUniform("viewProj", camera->getViewProj());
    shaderModel->setUniform("camPos", camera->getViewPosition());
    shaderModel->setUniform("lightColors", lightColors, lightCount);
    shaderModel->setUniform("lightPositions", lightPositions, lightCount);
    shaderModel->setUniform("lightEnables", lightEnables, lightCount);
    shaderModel->bindTexture("irradianceMap", this->textureIrradiance);
    shaderModel->setUniform("enableIbl", this->enableIbl);

    auto mat = math::scale({50, 50, 50});
    auto normalMat = glm::transpose(glm::inverse(math::Mat3{mat}));
    shaderModel->setUniform("model", mat);
    shaderModel->setUniform("normalMatrix", normalMat);

    shaderModel->setUniform("albedo", albedo);
    shaderModel->setUniform("metallic", metallic);
    shaderModel->setUniform("roughness", roughness);
    shaderModel->setUniform("ao", ao);

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
            shaderModel->setUniform("metallic", (float)i / (float)rows);
            for (int j = 0; j < columns; ++j)
            {
                shaderModel->setUniform("roughness", ((float)j / (float)columns) * 0.95f + 0.05f);
                model = math::translate(glm::vec3((float(j) - (columns / 2.0f)) * spacing,
                                                  (float(i) - (rows / 2.0f)) * spacing, 0.0f)) * mat;
                shaderModel->setUniform("model", model);
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

void IblIrradianceScene::drawSkyBox(const TextureRef& cubeMap)
{
    auto viewSize = camera->getViewSize();
    auto proj = math::perspective(camera->getFov(), viewSize.x/viewSize.y, 0.01, 4.0); // 调整near 和 far
    auto view = math::Mat4{math::Mat3{camera->getView()}}; // 去掉位移，将摄像机移动到圆点
    shaderSkyBox->use();
    shaderSkyBox->setUniform("viewProj", proj*view);
    shaderSkyBox->bindTexture("cubeMap", cubeMap);
    renderCube();
}

void IblIrradianceScene::drawSettings()
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
    ImGui::Checkbox("Enable IBL", &this->enableIbl);

    ImGui::Separator();
    ImGui::Text("Background");
    ImGui::RadioButton("Environment", &backgroundType, 0);
    ImGui::RadioButton("Irradiance", &backgroundType, 1);
    ImGui::RadioButton("None", &backgroundType, 2);

    ImGui::Separator();
    bool changeShowType {false};
    changeShowType = ImGui::RadioButton("Ball", &drawType, 0) || changeShowType;
    changeShowType = ImGui::RadioButton("Cube", &drawType, 1) || changeShowType;
    changeShowType = ImGui::RadioButton("Ball Group", &drawType, 2) || changeShowType;
    if (changeShowType)
    {
        this->reset();
    }
}

void IblIrradianceScene::createCubMap(const TextureRef& hdr, TextureRef& cubeMap)
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

void IblIrradianceScene::createIrradiance(const TextureRef& cubeMap, TextureRef& irradiance)
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