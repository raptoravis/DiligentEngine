#pragma once

#include "engine/engine.h"

class PipelineColorVertex : public pgPipeline {
	typedef pgPipeline base;
private:
	Diligent::RefCntAutoPtr<Diligent::IBuffer>               m_VSConstants;

private:
	void CreatePipelineState(const pgPipelineCreateInfo& ci);

public:
	PipelineColorVertex(const pgPipelineCreateInfo& ci);
	virtual ~PipelineColorVertex();

	//virtual void update(pgRenderEventArgs& e);
	virtual void updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag);
	//virtual void render(pgRenderEventArgs& e);

};