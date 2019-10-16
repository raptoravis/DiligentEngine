#pragma once

#include "SampleBase.h"

#include "../../engine.h"

struct PassSetRTCreateInfo : public pgPassCreateInfo {
	PassSetRTCreateInfo(const pgPassCreateInfo& ci)
		: pgPassCreateInfo(ci)
	{
	}

	std::shared_ptr<pgRenderTarget> rt;
};

class PassSetRT : public pgPass {
	typedef pgPass base;

	std::shared_ptr<pgRenderTarget> m_pRT;

public:
	PassSetRT(const PassSetRTCreateInfo& ci);
	virtual ~PassSetRT();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag);
};
