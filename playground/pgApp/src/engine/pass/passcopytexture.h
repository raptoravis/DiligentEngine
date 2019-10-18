#pragma once

#include "SampleBase.h"

#include "../engine.h"

class PassCopyTexture : public pgPass {
	typedef pgPass base;

	std::shared_ptr<pgTexture>         m_srcTexture;
	std::shared_ptr<pgTexture>         m_dstTexture;

public:
	PassCopyTexture(std::shared_ptr<pgTexture> dstTexture, std::shared_ptr<pgTexture> srcTexture);
	virtual ~PassCopyTexture();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);

	virtual void Render();

};
