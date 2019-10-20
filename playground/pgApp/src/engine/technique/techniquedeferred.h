#pragma once

#include "SampleBase.h"

#include "../engine.h"
#include "../pass/passrender.h"
#include "../utils/light.h"

using namespace Diligent;

class TechniqueDeferred : public pgTechnique
{
    typedef pgTechnique base;

    void createGBuffers();
    void createBuffers();

    std::shared_ptr<pgRenderTarget> m_pGBufferRT;
    std::shared_ptr<pgRenderTarget> m_pDepthOnlyRT;
    std::shared_ptr<pgTexture> m_depthStencilTexture;

    std::shared_ptr<ConstantBuffer> m_LightParamsCB;
    std::shared_ptr<ConstantBuffer> m_ScreenToViewParamsCB;

    std::shared_ptr<Shader> g_pVertexShader;
    std::shared_ptr<Shader> g_pPixelShader;
    std::shared_ptr<Shader> g_pGeometryPixelShader;
    std::shared_ptr<Shader> g_pDeferredLightingPixelShader;

    std::shared_ptr<pgPipeline> g_pGeometryPipeline;
    std::shared_ptr<pgPipeline> g_pOpaquePipeline;
    std::shared_ptr<pgPipeline> g_pTransparentPipeline;

  public:
    TechniqueDeferred(std::shared_ptr<pgRenderTarget> rt, std::shared_ptr<pgTexture> backBuffer);
    virtual ~TechniqueDeferred();

    void init(const std::shared_ptr<pgScene> scene, const std::vector<pgLight>& lights);
};