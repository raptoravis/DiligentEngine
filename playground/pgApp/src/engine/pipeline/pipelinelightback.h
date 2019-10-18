#pragma once

#include "engine/engine.h"

class PipelineLightBack : public pgPipeline {
	typedef pgPipeline base;
private:
	void CreatePipelineState();
	std::shared_ptr<pgRenderTarget>							m_pGBufferRT;
	Diligent::RefCntAutoPtr<Diligent::IBuffer>              m_PerObjectConstants;
	Diligent::RefCntAutoPtr<Diligent::IBufferView>			m_LightsBufferSRV;
	Diligent::RefCntAutoPtr<Diligent::IBuffer>              m_LightParamsCB;
	Diligent::RefCntAutoPtr<Diligent::IBuffer>              m_ScreenToViewParamsCB;
public:
	PipelineLightBack(std::shared_ptr<pgRenderTarget> rt,
		std::shared_ptr<pgRenderTarget> GBufferRT,
		Diligent::IBuffer* PerObjectConstants,
		Diligent::IBufferView* LightsBufferSRV,
		Diligent::IBuffer* LightParamsCB,
		Diligent::IBuffer* ScreenToViewParamsCB);

	virtual ~PipelineLightBack();

	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
	virtual void unbind(pgRenderEventArgs& e, pgBindFlag flag);

};