//
// Created by DELL on 2021/11/17.
//

#include "FrameBuffer.h"

#include <memory>
#include <utility>
#include "Utils.h"
#include "Texture.h"

RenderTarget::~RenderTarget()
{
    if (this->handle != 0)
    {
        if (this->format >= kTexColor)
        {
            glDeleteTextures(1, &this->handle);
        }
        else
        {
            glDeleteRenderbuffers(1, &this->handle);
        }
    }
}

RenderTargetRef RenderTarget::create(int width, int height, Format format, GLsizei samples /* = 1*/)
{
    if (format == kNone)
    {
        return nullptr;
    }
    GLenum internalformat;
    switch (format)
    {
    case kRenderColor:
    case kTexColor:
        internalformat = GL_RGBA8;
        break;
    case kTexColorFloat:
        internalformat = GL_RGBA16F;
        break;
    case kRenderDepth:
    case kTexDepth:
        internalformat = GL_DEPTH_COMPONENT32;
        break;
    case kRenderDepthStencil:
    case kTexDepthStencil:
        internalformat = GL_DEPTH24_STENCIL8;
        break;
    default:
        return nullptr;
    }
    return createEx(width, height, format, internalformat, samples);
}

RenderTargetRef RenderTarget::createEx(int width, int height, Format format, GLenum internalformat,
                                       GLsizei samples /* = 1*/)
{
    if (format == kNone)
    {
        return nullptr;
    }
    auto target = new RenderTarget();
    target->width = width;
    target->height = height;
    target->format = format;
    target->samples = samples;
    if (format >= kTexColor)
    {
        if (samples <= 1)
        {
            glGenTextures(1, &target->handle);
            glBindTexture(GL_TEXTURE_2D, target->handle);
            glTexStorage2D(GL_TEXTURE_2D, 1, internalformat, (GLsizei) width, (GLsizei) height);
        }
        else
        {
            glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &target->handle);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, target->handle);
            glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, (GLenum) internalformat, (GLsizei) width, (GLsizei) height, GL_TRUE);
        }

        target->texture = std::make_shared<Texture>();
        target->texture->tex = target->handle;
        target->texture->width = width;
        target->texture->height = height;
        target->texture->format = format;
    }
    else
    {
        glGenRenderbuffers(1, &target->handle);
        glBindRenderbuffer(GL_RENDERBUFFER, target->handle);
        if (samples <= 1)
        {
            glRenderbufferStorage(GL_RENDERBUFFER, internalformat, (GLsizei) width, (GLsizei) height);
        }
        else
        {
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalformat, (GLsizei) width, (GLsizei) height);
        }
    }
    CHECK_GL_ERROR();

    return RenderTargetRef(target);
}


FrameBuffer::~FrameBuffer()
{
    if (this->handle != 0)
    {
        glDeleteFramebuffers(1, &this->handle);
    }
    CHECK_GL_ERROR();
}


FrameBufferRef FrameBuffer::create(int width, int height,
                                   RenderTarget::Format colorFormat /*= RenderTarget::kTexColor*/,
                                   RenderTarget::Format depthFormat /*= RenderTarget::kNone*/)
{
    auto fb = new FrameBuffer();
    if (fb->init(width, height, colorFormat, depthFormat))
    {
        return FrameBufferRef(fb);
    }
    return nullptr;
}

FrameBufferRef FrameBuffer::createMultisample(int width, int height, GLsizei samples /*= 4*/,
                                              RenderTarget::Format colorFormat /*= RenderTarget::kTexColor*/,
                                              bool isNeedDepth /*= true*/)
{
    auto fb = new FrameBuffer();
    if (fb->initMultisample(width, height, samples, colorFormat, isNeedDepth))
    {
        return FrameBufferRef(fb);
    }
    return nullptr;
}

size_t FrameBuffer::addColorAttachment(GLenum internalFormat)
{
    auto format = RenderTarget::kTexColor;
    if (!this->colorAttachments.empty() && this->colorAttachments[0]->format < RenderTarget::kTexColor)
    {
        format = RenderTarget::kRenderColor;
    }

    auto rt = RenderTarget::createEx((int) width, (int) height, format, internalFormat, samples);
    return this->addColorAttachment(rt);
}

size_t FrameBuffer::addColorAttachment(const RenderTargetRef& target)
{
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldFboId);
    glBindFramebuffer(GL_FRAMEBUFFER, this->handle);
    GLenum index = GL_COLOR_ATTACHMENT0 + colorAttachments.size();

    if (target->format == RenderTarget::kTexColor)
    {
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, index, target->handle, 0);
    }
    else if (target->format == RenderTarget::kRenderColor)
    {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, index, GL_RENDERBUFFER, target->handle);
    }
    else
    {
        assert(false && "FrameBuffer::addColorAttachment, format error");
    }

    colorAttachments.push_back(target);

    glBindFramebuffer(GL_FRAMEBUFFER, oldFboId);
    oldFboId = -1;
    CHECK_GL_ERROR();
    return colorAttachments.size() - 1;
}

void FrameBuffer::bind() const
{
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldFboId);
    glGetIntegerv(GL_VIEWPORT, this->oldViewport);

    glBindFramebuffer(GL_FRAMEBUFFER, this->handle);
    glViewport(0, 0, (GLsizei) this->width, (GLsizei) this->height);
    CHECK_GL_ERROR();
}

void FrameBuffer::bind(const TextureRef& texture, int level, int layer /*= -1*/)
{
    assert(this->colorAttachments.empty() && "Color Attachment must be empty!");

    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldFboId);
    glGetIntegerv(GL_VIEWPORT, this->oldViewport);

    glBindFramebuffer(GL_FRAMEBUFFER, this->handle);

    auto target = texture->getTarget();
    if (target == GL_TEXTURE_2D)
    {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture->getHandle(), level);
    }
    else if(target == GL_TEXTURE_CUBE_MAP)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, layer, texture->getHandle(), level);
    }
    else if(target == GL_TEXTURE_3D)
    {
        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, texture->getHandle(), level, layer);
    }
    else
    {
        assert(false && "FrameBuffer::bind, Not support texture type");
    }

    glViewport(0, 0, (GLsizei) this->width, (GLsizei) this->height);
    CHECK_GL_ERROR();

}

void FrameBuffer::unbind() const
{
    if (oldFboId != -1)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, this->oldFboId);
        glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
        CHECK_GL_ERROR();
        oldFboId = -1;
    }
}

bool FrameBuffer::init(int width_, int height_, RenderTarget::Format colorFormat /*= RenderTarget::kTexColor*/,
                       RenderTarget::Format depthFormat /*= RenderTarget::kNone*/)
{
    this->width = width_;
    this->height = height_;

    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldFboId);
    glGenFramebuffers(1, &this->handle);
    glBindFramebuffer(GL_FRAMEBUFFER, this->handle);
    if (colorFormat != RenderTarget::kNone)
    {
        auto color = RenderTarget::create(width_, height_, colorFormat);
        if (colorFormat == RenderTarget::kTexColor || colorFormat == RenderTarget::kTexColorFloat)
        {
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color->handle, 0);
        }
        else if (colorFormat == RenderTarget::kRenderColor)
        {
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color->handle);
        }
        this->colorAttachments.push_back(std::move(color));
    }

    if (depthFormat != RenderTarget::kNone)
    {
        depth = RenderTarget::create(width_, height_, depthFormat);
        switch (depthFormat)
        {
        case RenderTarget::kRenderDepthStencil:
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth->handle);
        case RenderTarget::kRenderDepth:
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth->handle);
            break;
        case RenderTarget::kTexDepthStencil:
            glFramebufferTexture(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, depth->handle, 0);
        case RenderTarget::kTexDepth:
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth->handle, 0);
            break;
        default:
            break;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, oldFboId);
    oldFboId = -1;
    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
    {
        std::cerr << "FrameBuffer Status Error: " << (int) glCheckFramebufferStatus(GL_FRAMEBUFFER) << '\n';
        return false;
    }
    CHECK_GL_ERROR();
    return true;
}

bool FrameBuffer::initMultisample(int width_, int height_, GLsizei samples_, RenderTarget::Format colorFormat, bool isNeedDepth)
{
    assert(colorFormat != RenderTarget::kNone && "Must add a color attachment!");

    this->width = width_;
    this->height = height_;
    this->samples = samples_;

    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldFboId);
    glGenFramebuffers(1, &this->handle);
    glBindFramebuffer(GL_FRAMEBUFFER, this->handle);

    auto color= RenderTarget::create(width_, height_, colorFormat, samples_);
    if (colorFormat == RenderTarget::kTexColor || colorFormat == RenderTarget::kTexColorFloat)
    {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color->handle, 0);
        if (isNeedDepth)
        {
            depth= RenderTarget::create(width_, height_, RenderTarget::kTexDepth, samples_);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth->handle, 0);
        }
        this->colorAttachments.push_back(std::move(color));
    }
    else
    {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color->handle);
        if (isNeedDepth)
        {
            depth= RenderTarget::create(width_, height_, RenderTarget::kRenderDepth, samples_);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth->handle);
        }
        this->colorAttachments.push_back(std::move(color));
    }

    glBindFramebuffer(GL_FRAMEBUFFER, oldFboId);
    oldFboId = -1;
    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
    {
        std::cerr << "FrameBuffer Status Error: " << (int) glCheckFramebufferStatus(GL_FRAMEBUFFER) << '\n';
        return false;
    }
    CHECK_GL_ERROR();
    return true;
}


void FrameBuffer::blitFramebuffer(int dstX0, int dstY0, int dstX1, int dstY1, GLuint frameBuffer, GLsizei sampleCount) const
{
    if (this->colorAttachments[0]->samples > 1 || sampleCount > 1)
    {
        if (abs(dstX0 - dstX1) != this->width || abs(dstY0 - dstY1) != this->height)
        {
            std::cerr << "source and destination rectangles should be identical when Enable multisampling\n";
        }
    }
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &oldFboId);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, this->handle);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
    glBlitFramebuffer(0, 0, (GLint) width, (GLint) height, dstX0, dstY0, dstX1, dstY1, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, oldFboId);
    oldFboId = -1;
    CHECK_GL_ERROR();
}

std::vector<uint8_t> FrameBuffer::readPixel() const
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, this->handle);
    std::vector<uint8_t> pixels(this->width * this->height * 4);
    glReadPixels(0, 0, (GLsizei)width, (GLsizei)height, GL_RGBA, GL_UNSIGNED_BYTE, (void*)pixels.data());
    return pixels;
}
