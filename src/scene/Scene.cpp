//
// Created by wlk12 on 2023/8/6.
//

#include "Scene.hpp"
#include "common/FrameBuffer.h"
#include "common/Shader.h"
#include "common/Utils.h"

Scene::Scene(const char* name, int width, int height) :name(name), width(width), height(height)
{
    if (width > 0 && height > 0)
    {
        Scene::resize(width, height);
    }

    this->toneMappingShader = Shader::createByPath("asset/shader/tone_mapping.vert", "asset/shader/tone_mapping.frag");

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
    this->fbDraw = FrameBuffer::create(width, height, RenderTarget::kTexColorFloat, RenderTarget::kTexDepth);
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
    this->toneMappingShader->bindTexture(0, this->fbDraw->getColor());
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
