#pragma once

#include "SampleBase.h"

#include "../engine.h"

namespace ade
{

class PassSetRT : public Pass
{
    typedef Pass base;

    std::shared_ptr<RenderTarget> m_pRenderTarget;

  public:
    PassSetRT(Technique* parentTechnique, std::shared_ptr<RenderTarget> rt);
    virtual ~PassSetRT();


    virtual void Render(Pipeline* pipeline);
};

}    // namespace ade