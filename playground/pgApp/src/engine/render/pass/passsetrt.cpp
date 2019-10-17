#include "passsetrt.h"

PassSetRT::PassSetRT(std::shared_ptr<pgRenderTarget> rt)
	: base(0)
	, m_pRT(rt)
{
}

PassSetRT::~PassSetRT()
{
}

// Render a frame
void PassSetRT::render(pgRenderEventArgs& e) {
	if (m_pRT) {
		m_pRT->Bind();
	}
}

void PassSetRT::update(pgRenderEventArgs& e) {
	//
}

void PassSetRT::bind(pgRenderEventArgs& e, pgBindFlag flag) {
}