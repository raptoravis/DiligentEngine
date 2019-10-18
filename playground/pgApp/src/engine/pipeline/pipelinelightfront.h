#pragma once

#include "engine/engine.h"

class PipelineLightFront : public pgPipeline {
	typedef pgPipeline base;
private:
	void CreatePipelineState();
	Diligent::RefCntAutoPtr<Diligent::IBuffer>              m_PerObjectConstants;
public:
	PipelineLightFront(std::shared_ptr<pgRenderTarget> rt, Diligent::IBuffer* PerObjectConstants);
	virtual ~PipelineLightFront();

	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
	virtual void unbind(pgRenderEventArgs& e, pgBindFlag flag);

};