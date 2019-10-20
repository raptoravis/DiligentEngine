#include "passcopytexture.h"

PassCopyTexture::PassCopyTexture(pgTechnique* parentTechnique,
                                 std::shared_ptr<pgTexture> dstTexture,
                                 std::shared_ptr<pgTexture> srcTexture)
	: base(parentTechnique)
	, m_dstTexture(dstTexture)
	, m_srcTexture(srcTexture)
{
}

PassCopyTexture::~PassCopyTexture()
{
}

// Render a frame
void PassCopyTexture::render(pgRenderEventArgs& e) {
	Diligent::CopyTextureAttribs CopyAttribs(m_srcTexture->GetTexture(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
		m_dstTexture->GetTexture(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	CopyAttribs.SrcMipLevel = 0;
	CopyAttribs.DstMipLevel = 0;
	CopyAttribs.DstSlice = 0;

	pgApp::s_ctx->CopyTexture(CopyAttribs);
}

void PassCopyTexture::update(pgRenderEventArgs& e) {
	//
}

void PassCopyTexture::bind(pgRenderEventArgs& e, pgBindFlag flag) {
}

void PassCopyTexture::Render() {
	Diligent::CopyTextureAttribs CopyAttribs(m_srcTexture->GetTexture(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
		m_dstTexture->GetTexture(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	CopyAttribs.SrcMipLevel = 0;
	CopyAttribs.DstMipLevel = 0;
	CopyAttribs.DstSlice = 0;

	pgApp::s_ctx->CopyTexture(CopyAttribs);
}

