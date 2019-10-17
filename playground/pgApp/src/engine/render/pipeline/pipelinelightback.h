#pragma once

#include "engine/engine.h"

class PipelineLightBack : public pgPipeline {
	typedef pgPipeline base;
private:
private:
	void CreatePipelineState(const pgPipelineCreateInfo& ci);
public:
	PipelineLightBack(const pgPipelineCreateInfo& ci);
	virtual ~PipelineLightBack();

	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
	virtual void unbind(pgRenderEventArgs& e, pgBindFlag flag);

};