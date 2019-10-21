#pragma once 

#include <vector>
#include "SampleBase.h"
#include "BasicMath.h"

#include "engine/engine.h"
#include "engine/utils/light.h"

using namespace Diligent;


class AppTest final : public pgApp
{
	enum class RenderingTechnique : int
	{
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

	virtual void Initialize(IEngineFactory*   pEngineFactory,
		IRenderDevice*    pDevice,
		IDeviceContext**  ppContexts,
		Uint32            NumDeferredCtx,
		ISwapChain*       pSwapChain) override final;

	virtual void Render()override final;
	virtual void Update(double CurrTime, double ElapsedTime)override final;
	virtual const Char* GetSampleName()const override final { return "AppTest"; }

private:
	void initLightData();
	void initBuffers();
	void createRT();
private:
	std::vector<pgLight>						m_Lights;

	std::shared_ptr<ConstantBuffer>				m_PerObjectConstants;
    std::shared_ptr<ConstantBuffer>				m_MaterialConstants;
    std::shared_ptr<StructuredBuffer>			m_LightsStructuredBuffer;

	std::shared_ptr<pgRenderTarget>				m_pRenderTarget;
	std::shared_ptr<pgTexture>					m_pBackBuffer;
	std::shared_ptr<pgTexture>					m_pDepthStencilBuffer;

	// 
	std::shared_ptr<pgTechnique>				m_pTechnique;

	std::shared_ptr<pgTechnique>				m_pForwardTechnique;
	std::shared_ptr<pgTechnique>				m_pDeferredTechnique;
	std::shared_ptr<pgTechnique>				m_pForwardPlusTechnique;

	RenderingTechnique							m_renderingTechnique = RenderingTechnique::Test;
};
