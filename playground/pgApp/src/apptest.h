#pragma once

#include "BasicMath.h"
#include "SampleBase.h"
#include <vector>

#include "engine/engine.h"

using namespace Diligent;

class AppTest final : public ade::pgApp
{
    enum class RenderingTechnique : int {
        Test,
        Forward,
        Deferred,
        ForwardPlus,
        LightPrepass,
        GDR,
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
    std::vector<ade::pgLight> m_Lights;

    std::shared_ptr<ade::ConstantBuffer> m_PerObjectConstants;
    std::shared_ptr<ade::ConstantBuffer> m_MaterialConstants;
    std::shared_ptr<ade::StructuredBuffer> m_LightsStructuredBuffer;

    std::shared_ptr<ade::pgRenderTarget> m_pRenderTarget;
    std::shared_ptr<ade::pgTexture> m_pBackBuffer;
    std::shared_ptr<ade::pgTexture> m_pDepthStencilBuffer;

    //
    std::shared_ptr<ade::pgTechnique> m_pTechnique;

    std::shared_ptr<ade::pgTechnique> m_pForwardTechnique;
    std::shared_ptr<ade::pgTechnique> m_pDeferredTechnique;
    std::shared_ptr<ade::pgTechnique> m_pForwardPlusTechnique;

    RenderingTechnique m_renderingTechnique = RenderingTechnique::Test;
};
