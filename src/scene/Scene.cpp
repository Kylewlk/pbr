//
// Created by wlk12 on 2023/8/6.
//

#include "Scene.hpp"
#include "common/FrameBuffer.h"
#include "common/Texture.h"
#include "common/Shader.h"
#include "common/Utils.h"

Scene::Scene(const char* name, int width, int height, bool multisample)
    : name(name), width(width), height(height), multisample(multisample)
{
    if (width > 0 && height > 0)
    {
        Scene::resize(width, height);
    }

    if (multisample)
    {
        this->toneMappingShader = Shader::createByPath("asset/shader/tone_mapping.vert", "asset/shader/tone_mapping_multisample.frag");
    }
    else
    {
        this->toneMappingShader = Shader::createByPath("asset/shader/tone_mapping.vert", "asset/shader/tone_mapping.frag");
    }

    this->mouseListener = MouseListener::create();
    this->mouseListener->onMouseEvent = [this](auto e){ this->onMouseEvent(e); };
    EventSystem::get()->subscribe(this->mouseListener);

    this->keyListener = KeyListener::create();
    this->keyListener->onKey = [this](auto e){ this->onKeyEvent(e); };
    EventSystem::get()->subscribe(this->keyListener);
}

TextureRef Scene::getColorTexture()
{
    return this->fbResolved->getColor();
}

void Scene::resize(int width_, int height_)
{
    if (this->width == width_ && this->height == height_)
    {
        return;
    }

    this->width = width_;
    this->height = height_;
    this->fbResolved = FrameBuffer::create(width, height, RenderTarget::kTexColor, RenderTarget::kNone);
    if (this->multisample)
    {
        this->fbDraw = FrameBuffer::createMultisample(width, height, multisampleCount, RenderTarget::kTexColorFloat, true);
    }
    else
    {
        this->fbDraw = FrameBuffer::create(width, height, RenderTarget::kTexColorFloat, RenderTarget::kTexDepth);
    }
}

void Scene::draw()
{

}

void Scene::drawProperty()
{

}

void Scene::render()
{
    fbDraw->bind();
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    this->draw();

    fbDraw->unbind();

//    fbDraw->blitFramebuffer(0, 0, width, height, this->fbResolved);

    this->fbResolved->bind();
    this->toneMappingShader->use();

    if (this->multisample)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbDraw->getColor()->getHandle());
        glUniform1i(0, 0);

        glUniform1i(1, (GLint)multisampleCount);
    }
    else
    {
        this->toneMappingShader->bindTexture(0, this->fbDraw->getColor());
    }
    drawQuad();
    this->fbResolved->unbind();


    this->drawProperty();
}

void Scene::onMouseEvent(const MouseEvent* e)
{

}

void Scene::onKeyEvent(const KeyEvent* e)
{

}
