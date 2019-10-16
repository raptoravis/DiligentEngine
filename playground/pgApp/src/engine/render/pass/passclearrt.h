#pragma once

#include "SampleBase.h"

#include "../../engine.h"

struct PassClearRTCreateInfo : public pgPassCreateInfo {
	PassClearRTCreateInfo(const pgPassCreateInfo& ci)
		: pgPassCreateInfo(ci)
	{
	}

	Diligent::ITexture* dstTexture;
	Diligent::ITexture* srcTexture;
};

class PassClearRT : public pgPass {
	typedef pgPass base;

	std::shared_ptr<pgRenderTarget> m_RenderTarget;
	std::shared_ptr<pgTexture> m_Texture;
	pgClearFlags		m_ClearFlags;
	Diligent::float4	m_ClearColor;
	float				m_ClearDepth;
	uint8_t				m_ClearStencil;

public:
	PassClearRT(const PassClearRTCreateInfo& ci);
	virtual ~PassClearRT();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag);
};
