#pragma once

#include "SampleBase.h"

#include "../engine.h"
#include "../pass/passrender.h"
#include "../utils/light.h"

using namespace Diligent;

class TechniqueForward : public pgTechnique
{
    typedef pgTechnique base;

    std::shared_ptr<Shader> g_pVertexShader;
    std::shared_ptr<Shader> g_pPixelShader;

    std::shared_ptr<pgPipeline> g_pOpaquePipeline;
    std::shared_ptr<pgPipeline> g_pTransparentPipeline;

  public:
    TechniqueForward(std::shared_ptr<pgRenderTarget> rt, std::shared_ptr<pgTexture> backBuffer);
    virtual ~TechniqueForward();

    void init(std::shared_ptr<pgScene> scene, const std::vector<pgLight>& lights);
};