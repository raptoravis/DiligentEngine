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

	//virtual void update(pgRenderEventArgs& e);
	virtual void updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag);
	//virtual void render(pgRenderEventArgs& e);

};