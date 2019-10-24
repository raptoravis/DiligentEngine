#pragma once

#include "SampleBase.h"

#include "../engine.h"
#include "../pass/passpostprocess.h"
#include "../pass/passrender.h"
#include "../utils/light.h"

using namespace Diligent;

class TechniqueDeferred : public pgTechnique
{
    typedef pgTechnique base;

    void createGBuffers();
    void createBuffers();
    void initDebug();

    std::shared_ptr<pgRenderTarget> m_pGBufferRT;
    std::shared_ptr<pgRenderTarget> m_pDepthOnlyRenderTarget;
    std::shared_ptr<pgRenderTarget> m_pColorOnlyRenderTarget;
    std::shared_ptr<pgTexture> m_depthStencilTexture;

    std::shared_ptr<ConstantBuffer> m_LightParamsCB;
    std::shared_ptr<ConstantBuffer> m_ScreenToViewParamsCB;

	std::shared_ptr<Shader> m_pDebugTexturePixelShader;
    std::shared_ptr<Shader> m_pDebugDepthTexturePixelShader;

    std::shared_ptr<Shader> m_pVertexShader;
    std::shared_ptr<Shader> m_pPixelShader;
    std::shared_ptr<Shader> m_pGeometryPixelShader;
    std::shared_ptr<Shader> m_pDeferredLightingPixelShader;

    std::shared_ptr<pgPipeline> m_pDebugTexturePipeline;
    std::shared_ptr<pgPipeline> m_pDebugDepthTexturePipeline;

    std::shared_ptr<pgPipeline> m_pGeometryPipeline;
    std::shared_ptr<pgPipeline> m_pOpaquePipeline;
    std::shared_ptr<pgPipeline> m_pTransparentPipeline;

    std::shared_ptr<SamplerState> m_LinearRepeatSampler;
    std::shared_ptr<SamplerState> m_LinearClampSampler;

    std::shared_ptr<PassPostprocess> m_DebugTexture0Pass;
    std::shared_ptr<PassPostprocess> m_DebugTexture1Pass;
    std::shared_ptr<PassPostprocess> m_DebugTexture2Pass;
    std::shared_ptr<PassPostprocess> m_DebugTexture3Pass;

    bool m_bDepth;
    bool m_bDiffuse;
    bool m_bSpecular;
    bool m_bNormal;

  public:
    TechniqueDeferred(std::shared_ptr<pgRenderTarget> rt, std::shared_ptr<pgTexture> backBuffer);
    virtual ~TechniqueDeferred();

    void init(const std::shared_ptr<pgScene> scene, std::vector<pgLight>* lights);

    virtual void Update();
};