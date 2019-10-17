#pragma once

#include "engine/engine.h"

class PipelineLightFront : public pgPipeline {
	typedef pgPipeline base;
private:
private:
	void CreatePipelineState();
public:
	PipelineLightFront(std::shared_ptr<pgRenderTarget> rt);
	virtual ~PipelineLightFront();

	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
	virtual void unbind(pgRenderEventArgs& e, pgBindFlag flag);

};