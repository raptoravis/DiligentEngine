#include "passsetrt.h"

PassSetRT::PassSetRT(pgTechnique* parentTechnique, std::shared_ptr<pgRenderTarget> rt)
    : base(parentTechnique)
	, m_pRT(rt)
{
}

PassSetRT::~PassSetRT()
{
}

// Render a frame
void PassSetRT::render(pgRenderEventArgs& e) {
	if (m_pRT) {
		m_pRT->bind();
	}
}

void PassSetRT::update(pgRenderEventArgs& e) {
	//
}

void PassSetRT::bind(pgRenderEventArgs& e, pgBindFlag flag) {
}

void PassSetRT::Render() {
	if (m_pRT) {
		m_pRT->bind();
	}
}
