#pragma once

#include "engine/engine.h"

class PipelineColorVertex : public pgPipeline {
	typedef pgPipeline base;
private:
	Diligent::RefCntAutoPtr<Diligent::IBuffer>               m_VSConstants;

private:
	void CreatePipelineState();

public:
	PipelineColorVertex(std::shared_ptr<pgRenderTarget> rt);
	virtual ~PipelineColorVertex();

	//virtual void update(pgRenderEventArgs& e);
	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
	//virtual void render(pgRenderEventArgs& e);

};