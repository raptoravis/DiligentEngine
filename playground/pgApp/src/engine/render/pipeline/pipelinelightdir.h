#pragma once

#include "engine/engine.h"

class PipelineLightDir : public pgPipeline {
	typedef pgPipeline base;
private:
private:
	void CreatePipelineState();
public:
	PipelineLightDir(std::shared_ptr<pgRenderTarget> rt);
	virtual ~PipelineLightDir();

	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
	virtual void unbind(pgRenderEventArgs& e, pgBindFlag flag);

};