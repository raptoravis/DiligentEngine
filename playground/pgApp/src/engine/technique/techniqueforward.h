#pragma once

#include "SampleBase.h"

#include "../engine.h"
#include "../pass/passrender.h"
#include "../utils/light.h"

using namespace Diligent;

class TechniqueForward : public pgTechnique
{
    typedef pgTechnique base;

    std::shared_ptr<Shader> m_pVertexShader;
    std::shared_ptr<Shader> m_pPixelShader;

    std::shared_ptr<SamplerState> m_LinearRepeatSampler;
    std::shared_ptr<SamplerState> m_LinearClampSampler;

    std::shared_ptr<pgPipeline> m_pOpaquePipeline;
    std::shared_ptr<pgPipeline> m_pTransparentPipeline;

  public:
    TechniqueForward(std::shared_ptr<pgRenderTarget> rt, std::shared_ptr<pgTexture> backBuffer);
    virtual ~TechniqueForward();

    void init(std::shared_ptr<pgScene> scene, std::vector<pgLight>* lights);
};