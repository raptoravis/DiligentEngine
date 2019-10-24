#pragma once

#include "SampleBase.h"

#include "../engine.h"

namespace ade
{

class PassClearRT : public Pass
{
    typedef Pass base;

    std::shared_ptr<RenderTarget> m_RenderTarget;
    std::shared_ptr<Texture> m_Texture;
    ClearFlags m_ClearFlags;
    Diligent::float4 m_ClearColor;
    float m_ClearDepth;
    uint8_t m_ClearStencil;

  public:
    PassClearRT(Technique* parentTechnique, std::shared_ptr<RenderTarget> rt,
                ClearFlags clearFlags = ClearFlags::All,
                Diligent::float4 clearColor = Diligent::float4(0.39f, 0.58f, 0.93f, 1.0f),
                float clearDepth = 1.0f, uint8_t clearStencil = 0);

    virtual ~PassClearRT();

    virtual void Render(Pipeline* pipeline);
};
}    // namespace ade