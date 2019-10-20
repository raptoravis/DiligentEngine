#pragma once

#include "engine/engine.h"

class PipelineFPOpaque : public pgPipeline {
	typedef pgPipeline base;
private:
	Diligent::RefCntAutoPtr<Diligent::IBuffer>				m_VSConstants;
	Diligent::RefCntAutoPtr<Diligent::ITextureView>         m_TextureSRV;
private:
	void CreatePipelineState();
	void LoadTexture();
public:
	PipelineFPOpaque(std::shared_ptr<pgRenderTarget> rt);
	virtual ~PipelineFPOpaque();

};