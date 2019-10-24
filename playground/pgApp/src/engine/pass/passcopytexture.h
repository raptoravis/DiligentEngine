#pragma once

#include "SampleBase.h"

#include "../engine.h"

namespace ade
{

class PassCopyTexture : public pgPass
{
    typedef pgPass base;

    std::shared_ptr<pgTexture> m_srcTexture;
    std::shared_ptr<pgTexture> m_dstTexture;

  public:
    PassCopyTexture(pgTechnique* parentTechnique, std::shared_ptr<pgTexture> dstTexture,
                    std::shared_ptr<pgTexture> srcTexture);
    virtual ~PassCopyTexture();

    virtual void Render(pgPipeline* pipeline);
};
}    // namespace ade