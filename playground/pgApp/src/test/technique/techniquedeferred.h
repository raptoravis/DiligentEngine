#pragma once

#include "SampleBase.h"

#include "engine/engine.h"
#include "test/pass/passpostprocess.h"
#include "test/pass/passrender.h"

using namespace Diligent;

namespace ade
{

class TechniqueDeferred : public Technique
{
    typedef Technique base;

    void createGBuffers();
    void createBuffers();
    void initDebug();

    std::shared_ptr<RenderTarget> m_pGBufferRT;
    std::shared_ptr<RenderTarget> m_pDepthOnlyRenderTarget;
    std::shared_ptr<RenderTarget> m_pColorOnlyRenderTarget;
    std::shared_ptr<Texture> m_depthStencilTexture;

    std::shared_ptr<ConstantBuffer> m_LightParamsCB;
    std::shared_ptr<ConstantBuffer> m_ScreenToViewParamsCB;

    std::shared_ptr<Shader> m_pDebugDepthTexturePixelShader;

    std::shared_ptr<Shader> m_pVertexShader;
    std::shared_ptr<Shader> m_pPixelShader;
    std::shared_ptr<Shader> m_pGeometryPixelShader;
    std::shared_ptr<Shader> m_pDeferredLightingPixelShader;

    std::shared_ptr<Pipeline> m_pDebugDepthTexturePipeline;

    std::shared_ptr<Pipeline> m_pGeometryPipeline;
    std::shared_ptr<Pipeline> m_pOpaquePipeline;
    std::shared_ptr<Pipeline> m_pTransparentPipeline;

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
    TechniqueDeferred(std::shared_ptr<RenderTarget> rt, std::shared_ptr<Texture> backBuffer);
    virtual ~TechniqueDeferred();

    void init(const std::shared_ptr<Scene> scene, std::vector<Light>* lights);

    virtual void Update();
};

}    // namespace ade