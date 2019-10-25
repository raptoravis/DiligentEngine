#pragma once

#include "BasicMath.h"
#include "SampleBase.h"
#include <vector>

#include "engine/engine.h"

using namespace Diligent;

class AppTest final : public ade::App
{
    enum class RenderingTechnique : int {
        Test,
        Forward,
        Deferred,
        ForwardPlus,
        Gdr,
        LightPrepass,
        NumTechniques
    };

  public:
    virtual ~AppTest();

    virtual void Initialize(IEngineFactory* pEngineFactory, IRenderDevice* pDevice,
                            IDeviceContext** ppContexts, Uint32 NumDeferredCtx,
                            ISwapChain* pSwapChain) override final;

    virtual void Render() override final;
    virtual void Update(double CurrTime, double ElapsedTime) override final;
    virtual const Char* GetSampleName() const override final { return "AppTest"; }

  private:
    void initLightData();
    void initBuffers();
    void createRT();

  private:
    std::vector<ade::Light> m_Lights;

    std::shared_ptr<ade::ConstantBuffer> m_PerObjectConstants;
    std::shared_ptr<ade::ConstantBuffer> m_MaterialConstants;
    std::shared_ptr<ade::StructuredBuffer> m_LightsStructuredBuffer;

    std::shared_ptr<ade::RenderTarget> m_pRenderTarget;
    std::shared_ptr<ade::Texture> m_pBackBuffer;
    std::shared_ptr<ade::Texture> m_pDepthStencilBuffer;

    //
    std::shared_ptr<ade::Technique> m_pTechnique;

    std::shared_ptr<ade::Technique> m_pForwardTechnique;
    std::shared_ptr<ade::Technique> m_pDeferredTechnique;
    std::shared_ptr<ade::Technique> m_pForwardPlusTechnique;
    std::shared_ptr<ade::Technique> m_pGdrTechnique;

    RenderingTechnique m_renderingTechnique = RenderingTechnique::Test;
};
