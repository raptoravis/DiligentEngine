#include "../../app.h"
#include "passcopytexture.h"

PassCopyTexture::PassCopyTexture(const CopyTexturePassCreateInfo& ci)
	: base(ci)
	, m_dstTexture(ci.dstTexture)
	, m_srcTexture(ci.srcTexture)
{
}

PassCopyTexture::~PassCopyTexture()
{
}

// Render a frame
void PassCopyTexture::render(pgRenderEventArgs& e) {
	Diligent::CopyTextureAttribs CopyAttribs(m_srcTexture, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
		m_dstTexture, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	CopyAttribs.SrcMipLevel = 0;
	CopyAttribs.DstMipLevel = 0;
	CopyAttribs.DstSlice = 0;

	m_pImmediateContext->CopyTexture(CopyAttribs);
}

void PassCopyTexture::update(pgRenderEventArgs& e) {
	//
}

void PassCopyTexture::updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag) {
}