#pragma once

#include "SampleBase.h"

#include "../../engine.h"

class PassClearRT : public pgPass {
	typedef pgPass base;

	std::shared_ptr<pgRenderTarget> m_RenderTarget;
	std::shared_ptr<pgTexture> m_Texture;
	pgClearFlags		m_ClearFlags;
	Diligent::float4	m_ClearColor;
	float				m_ClearDepth;
	uint8_t				m_ClearStencil;

public:
	PassClearRT(std::shared_ptr<pgRenderTarget> rt, 
		pgClearFlags clearFlags = pgClearFlags::All, 
		Diligent::float4 clearColor = Diligent::float4(0.39f, 0.58f, 0.93f, 1.0f), 
		float clearDepth = 1.0f, 
		uint8_t clearStencil = 0);

	virtual ~PassClearRT();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
};
