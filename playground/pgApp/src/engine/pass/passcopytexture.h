#pragma once

#include "SampleBase.h"

#include "../engine.h"

namespace ade
{

class PassCopyTexture : public Pass
{
    typedef Pass base;

    std::shared_ptr<Texture> m_srcTexture;
    std::shared_ptr<Texture> m_dstTexture;

  public:
    PassCopyTexture(Technique* parentTechnique, std::shared_ptr<Texture> dstTexture,
                    std::shared_ptr<Texture> srcTexture);
    virtual ~PassCopyTexture();

    virtual void Render(Pipeline* pipeline);
};
}    // namespace ade