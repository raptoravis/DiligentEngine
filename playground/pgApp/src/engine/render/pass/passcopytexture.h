#pragma once

#include "SampleBase.h"

#include "../../engine.h"

struct CopyTexturePassCreateInfo : public pgPassCreateInfo {
	CopyTexturePassCreateInfo(const pgPassCreateInfo& ci)
		: pgPassCreateInfo(ci)
	{
	}

	Diligent::ITexture* dstTexture;
	Diligent::ITexture* srcTexture;
};

class PassCopyTexture : public pgPass {
	typedef pgPass base;

	Diligent::RefCntAutoPtr<Diligent::ITexture>         m_dstTexture;
	Diligent::RefCntAutoPtr<Diligent::ITexture>         m_srcTexture;

public:
	PassCopyTexture(const CopyTexturePassCreateInfo& ci);
	virtual ~PassCopyTexture();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag);
};
