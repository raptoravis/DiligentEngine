#pragma once

#include "engine/engine.h"

class PipelineTexVertex : public pgPipeline {
	typedef pgPipeline base;
private:
	Diligent::RefCntAutoPtr<Diligent::IBuffer>				m_VSConstants;
	Diligent::RefCntAutoPtr<Diligent::ITextureView>         m_TextureSRV;
private:
	void CreatePipelineState(const pgPipelineCreateInfo& ci);
	void LoadTexture();
public:
	PipelineTexVertex(const pgPipelineCreateInfo& ci);
	virtual ~PipelineTexVertex();

	//virtual void update(pgRenderEventArgs& e);
	virtual void updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag);
	//virtual void render(pgRenderEventArgs& e);

};