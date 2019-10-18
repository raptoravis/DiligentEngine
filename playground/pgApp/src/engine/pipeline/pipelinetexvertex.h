#pragma once

#include "engine/engine.h"

class PipelineTexVertex : public pgPipeline {
	typedef pgPipeline base;
private:
	Diligent::RefCntAutoPtr<Diligent::IBuffer>				m_VSConstants;
	Diligent::RefCntAutoPtr<Diligent::ITextureView>         m_TextureSRV;
private:
	void CreatePipelineState();
	void LoadTexture();
public:
	PipelineTexVertex(std::shared_ptr<pgRenderTarget> rt);
	virtual ~PipelineTexVertex();

	//virtual void update(pgRenderEventArgs& e);
	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
	//virtual void render(pgRenderEventArgs& e);

};