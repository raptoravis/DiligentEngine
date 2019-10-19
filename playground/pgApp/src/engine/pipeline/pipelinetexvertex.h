#pragma once

#include "engine/engine.h"

class PipelineTexVertex : public pgPipeline {
	typedef pgPipeline base;
private:
	std::shared_ptr<Shader>							m_pVS;
	std::shared_ptr<Shader>							m_pPS;

	std::shared_ptr<pgTexture>						m_Texture;

public:
	PipelineTexVertex(std::shared_ptr<pgRenderTarget> rt);
	virtual ~PipelineTexVertex();

	void LoadTexture();

	//virtual void update(pgRenderEventArgs& e);
	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
	//virtual void render(pgRenderEventArgs& e);

};