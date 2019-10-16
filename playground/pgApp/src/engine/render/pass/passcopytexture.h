#pragma once

#include "SampleBase.h"

#include "../../engine.h"

struct CopyTexturePassCreateInfo : public pgPassCreateInfo {
	CopyTexturePassCreateInfo(const pgPassCreateInfo& ci)
		: pgPassCreateInfo(ci)
	{
	}

	std::shared_ptr<pgTexture> srcTexture;
	std::shared_ptr<pgTexture> dstTexture;
};

class PassCopyTexture : public pgPass {
	typedef pgPass base;

	std::shared_ptr<pgTexture>         m_srcTexture;
	std::shared_ptr<pgTexture>         m_dstTexture;

public:
	PassCopyTexture(const CopyTexturePassCreateInfo& ci);
	virtual ~PassCopyTexture();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
};
