#pragma once

#include "engine/engine.h"

class PipelineFPTransparent : public pgPipeline {
	typedef pgPipeline base;
private:
	Diligent::RefCntAutoPtr<Diligent::IBuffer>				m_VSConstants;
	Diligent::RefCntAutoPtr<Diligent::ITextureView>         m_TextureSRV;
private:
	void CreatePipelineState();
	void LoadTexture();
public:
	PipelineFPTransparent(std::shared_ptr<pgRenderTarget> rt);
	virtual ~PipelineFPTransparent();

	//virtual void update(pgRenderEventArgs& e);
	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
	//virtual void render(pgRenderEventArgs& e);

};