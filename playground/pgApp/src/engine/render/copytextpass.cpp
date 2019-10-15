#include "../app.h"
#include "copytextpass.h"

CopyTexturePass::CopyTexturePass(const CopyTexturePassCreateInfo& ci)
	: base(ci)
	, m_dstTexture(ci.dstTexture)
	, m_srcTexture(ci.srcTexture)
{
}

CopyTexturePass::~CopyTexturePass()
{
}

// Render a frame
void CopyTexturePass::render(pgRenderEventArgs& e) {
	Diligent::CopyTextureAttribs CopyAttribs(m_srcTexture, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
		m_dstTexture, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	CopyAttribs.SrcMipLevel = 0;
	CopyAttribs.DstMipLevel = 0;
	CopyAttribs.DstSlice = 0;

	m_pImmediateContext->CopyTexture(CopyAttribs);
}

void CopyTexturePass::update(pgRenderEventArgs& e) {
	//
}

void CopyTexturePass::updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag) {
}