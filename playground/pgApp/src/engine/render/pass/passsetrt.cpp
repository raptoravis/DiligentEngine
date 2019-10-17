#include "passsetrt.h"

PassSetRT::PassSetRT(const PassSetRTCreateInfo& ci)
	: base(ci)
	, m_pRT(ci.rt)
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