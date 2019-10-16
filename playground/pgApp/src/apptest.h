#pragma once 

#include <vector>
#include "SampleBase.h"
#include "BasicMath.h"

#include "engine/app.h"

namespace Diligent
{
	class AppTest final : public pgApp
	{
		enum class RenderingTechnique : int
		{
			Test, 
			Forward,
			Deferred,
			ForwardPlus,
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

		virtual void updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag);

	private:
		void initLightData();
		void initBuffers();
		void createRT(pgCreateInfo ci);
	private:
		std::vector<pgLight>								m_Lights;
		Diligent::RefCntAutoPtr<Diligent::IBuffer>			m_PerObjectConstants;
		Diligent::RefCntAutoPtr<Diligent::IBuffer>          m_MaterialConstants;
		Diligent::RefCntAutoPtr<Diligent::IBuffer>          m_LightsStructuredBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBufferView>		m_LightsBufferSRV;

		std::shared_ptr<pgRenderTarget>						m_pRT;
		std::shared_ptr<pgTexture>							m_pBackBuffer;
		std::shared_ptr<pgTexture>							m_pDepthStencilBuffer;

		// 
		std::shared_ptr<pgTechnique>	m_pTechnique;

		std::shared_ptr<pgTechnique>	m_pForwardTechnique;
		std::shared_ptr<pgTechnique>	m_pDeferredTechnique;
		std::shared_ptr<pgTechnique>	m_pForwardPlusTechnique;

		RenderingTechnique				m_renderingTechnique = RenderingTechnique::Test;
	};
}