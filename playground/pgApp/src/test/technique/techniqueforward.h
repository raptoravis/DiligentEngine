#pragma once

#include "SampleBase.h"

#include "engine/engine.h"
#include "test/pass/passrender.h"

using namespace Diligent;

namespace ade
{

class TechniqueForward : public Technique
{
    typedef Technique base;

    std::shared_ptr<Shader> m_pVertexShader;
    std::shared_ptr<Shader> m_pPixelShader;

    std::shared_ptr<SamplerState> m_LinearRepeatSampler;
    std::shared_ptr<SamplerState> m_LinearClampSampler;

    std::shared_ptr<Pipeline> m_pOpaquePipeline;
    std::shared_ptr<Pipeline> m_pTransparentPipeline;

  public:
    TechniqueForward(std::shared_ptr<RenderTarget> rt, std::shared_ptr<Texture> backBuffer);
    virtual ~TechniqueForward();

    void init(std::shared_ptr<Scene> scene, std::vector<Light>* lights);
};

}    // namespace ade