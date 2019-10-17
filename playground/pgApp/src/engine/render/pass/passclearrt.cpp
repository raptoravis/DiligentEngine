#include "passclearrt.h"

PassClearRT::PassClearRT(const PassClearRTCreateInfo& ci)
	: base(ci)
	, m_RenderTarget(ci.rt)
	, m_ClearFlags(ci.clearFlags)
	, m_ClearColor(ci.clearColor)
	, m_ClearDepth(ci.clearDepth)
	, m_ClearStencil(ci.clearStencil)
{
}

PassClearRT::~PassClearRT()
{
}

// Render a frame
void PassClearRT::render(pgRenderEventArgs& e) {
	if (m_RenderTarget) {
		m_RenderTarget->Clear(m_ClearFlags, m_ClearColor, m_ClearDepth, m_ClearStencil);
	}

	if (m_Texture) {
		m_Texture->Clear(m_ClearFlags, m_ClearColor, m_ClearDepth, m_ClearStencil);
	}
}

void PassClearRT::update(pgRenderEventArgs& e) {
	//
}

void PassClearRT::bind(pgRenderEventArgs& e, pgBindFlag flag) {
}