#pragma once

#include "SampleBase.h"

#include "../../engine.h"

class PassCopyBuffer : public pgPass {
	typedef pgPass base;

	std::shared_ptr<pgBuffer>         m_srcBuffer;
	std::shared_ptr<pgBuffer>         m_dstBuffer;

public:
	PassCopyBuffer(std::shared_ptr<pgBuffer> dstBuffer, std::shared_ptr<pgBuffer> srcBuffer);
	virtual ~PassCopyBuffer();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
};
