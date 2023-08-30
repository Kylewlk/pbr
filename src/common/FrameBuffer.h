//
// Created by DELL on 2021/11/17.
//
#pragma once
#include "Define.h"
#include "GLWindow.h"

class RenderTarget
{
public:
    virtual ~RenderTarget();
    RenderTarget(const RenderTarget&) = delete;
    RenderTarget& operator=(const RenderTarget&) = delete;

    friend class FrameBuffer;
    enum Format
    {
        kNone = 0,
        kRenderColor,
        kRenderDepth,
        kRenderDepthStencil,
        kTexColor,
        kTexColorFloat,
        kTexDepth,
        kTexDepthStencil,
    };

    static RenderTargetRef create(int width, int height, Format format, GLsizei samples  = 1);

    /// samples for Multisample
    static RenderTargetRef createEx(int width, int height, Format format, GLenum internalformat, GLsizei samples = 1);

    [[nodiscard]] Format getFormat() const { return this->format; }
    [[nodiscard]] GLuint getHandle() const { return this->handle; }
    [[nodiscard]] uint32_t getWidth() const { return this->width; }
    [[nodiscard]] uint32_t getHeight() const { return this->height; }
    [[nodiscard]] uint32_t getSampleCount() const { return this->samples; }

private:
    RenderTarget() = default;

    Format format{kNone};

    uint32_t width{};
    uint32_t height{};
    GLuint handle{};
    TextureRef texture{};

    GLsizei samples{1};
};

class FrameBuffer
{
public:
    virtual ~FrameBuffer();
    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;

    static FrameBufferRef create(int width, int height, RenderTarget::Format colorFormat = RenderTarget::kTexColor,
                                 RenderTarget::Format depthFormat = RenderTarget::kNone);
    static FrameBufferRef createMultisample(int width, int height, GLsizei samples = 4,
                                            RenderTarget::Format colorFormat = RenderTarget::kTexColor,
                                            bool isNeedDepth = true);

    size_t addColorAttachment(GLenum internalFormat);
    size_t addColorAttachment(const RenderTargetRef& target);
    void bind() const;
    void unbind() const;
    /// blit to default frame buffer
    void blitFramebuffer(int dstX0, int dstY0, int dstX1, int dstY1) const
    {
        blitFramebuffer(dstX0, dstY0, dstX1, dstY1, 0, GLWindow::get()->multisamplingCount);
    }
    void blitFramebuffer(int dstX0, int dstY0, int dstX1, int dstY1, const std::shared_ptr<FrameBuffer>& dst) const
    {
        blitFramebuffer(dstX0, dstY0, dstX1, dstY1, dst->getHandle(), samples);
    }
    void blitFramebuffer(int dstX0, int dstY0, int dstX1, int dstY1, GLuint dstFrameBuffer, GLsizei sampleCount ) const;

    [[nodiscard]]std::vector<uint8_t> readPixel() const;

    GLuint getHandle() const { return this->handle; }
    uint32_t getWidth() const { return this->width; }
    uint32_t getHeight() const { return this->height; }
    GLsizei getSamples() const { return this->samples; }

    const RenderTargetRef& getColorRef() const { return this->colorAttachments[0]; }
    const RenderTargetRef& getDepthRef() const { return this->depth; }
    const RenderTargetRef& getColorAttachmentEx(size_t id) const { return colorAttachments[id]; }

    const TextureRef& getColor() const { return colorAttachments[0]->texture; }
    const TextureRef& getDepth() const { return depth->texture; }

private:
    FrameBuffer() = default;

    bool init(int width, int height, RenderTarget::Format colorFormat = RenderTarget::kTexColor,
              RenderTarget::Format depthFormat = RenderTarget::kNone);
    bool initMultisample(int width, int height, GLsizei samples,  RenderTarget::Format colorFormat, bool isNeedDepth);

    uint32_t width{};
    uint32_t height{};
    GLuint handle{};
    GLsizei samples{1};

    std::vector<RenderTargetRef> colorAttachments;
    RenderTargetRef depth{};

    mutable GLint oldFboId{-1};
    mutable GLint oldViewport[4]{0};
};
