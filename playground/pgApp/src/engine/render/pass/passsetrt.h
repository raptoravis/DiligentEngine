#pragma once

#include "SampleBase.h"

#include "../../engine.h"

class PassSetRT : public pgPass {
	typedef pgPass base;

	std::shared_ptr<pgRenderTarget> m_pRT;

public:
	PassSetRT(std::shared_ptr<pgRenderTarget> rt);
	virtual ~PassSetRT();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
};
