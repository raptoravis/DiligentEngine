#pragma once

#include "engine/engine.h"

class PipelineLightFront : public pgPipeline {
	typedef pgPipeline base;
private:
private:
	void CreatePipelineState(const pgPipelineCreateInfo& ci);
public:
	PipelineLightFront(const pgPipelineCreateInfo& ci);
	virtual ~PipelineLightFront();

	//virtual void update(pgRenderEventArgs& e);
	virtual void updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag);
	//virtual void render(pgRenderEventArgs& e);

};