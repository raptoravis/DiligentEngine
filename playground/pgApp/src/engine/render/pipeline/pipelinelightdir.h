#pragma once

#include "engine/engine.h"

class PipelineLightDir : public pgPipeline {
	typedef pgPipeline base;
private:
private:
	void CreatePipelineState(const pgPipelineCreateInfo& ci);
public:
	PipelineLightDir(const pgPipelineCreateInfo& ci);
	virtual ~PipelineLightDir();

	//virtual void update(pgRenderEventArgs& e);
	virtual void updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag);
	//virtual void render(pgRenderEventArgs& e);

};