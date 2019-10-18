#include "../engine.h"

uint16_t pgTexture::GetWidth() const
{
	auto& desc = m_pTexture->GetDesc();
	return (uint16_t)desc.Width;
}

uint16_t pgTexture::GetHeight() const
{
	auto& desc = m_pTexture->GetDesc();
	return (uint16_t)desc.Height;
}

uint16_t pgTexture::GetDepth() const
{
	auto& desc = m_pTexture->GetDesc();
	return (uint16_t)desc.Depth;
}

uint8_t pgTexture::GetBPP() const
{
	//auto& desc = m_pTexture->GetDesc();
	//desc.Format;

	return 4;
}

bool pgTexture::IsTransparent() const
{
	//auto& desc = m_pTexture->GetDesc();
	//desc.Format;

	return false;
}

pgTexture::pgTexture(Diligent::ITexture* texture)
	: m_pTexture(texture)
{
}

pgTexture::~pgTexture() {
	//
}

Diligent::ITextureView* pgTexture::getShaderResourceView()
{
	auto srv = m_pTexture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
	return srv;
}

Diligent::ITextureView* pgTexture::getDepthStencilView() 
{
	auto dsv = m_pTexture->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
	return dsv;
}

Diligent::ITextureView* pgTexture::getRenderTargetView() 
{
	auto rtv = m_pTexture->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);

	return rtv;
}

Diligent::ITextureView* pgTexture::getUnorderedAccessView() 
{
	auto uav = m_pTexture->GetDefaultView(Diligent::TEXTURE_VIEW_UNORDERED_ACCESS);
	return uav;
}


void pgTexture::Clear(pgClearFlags clearFlags, const Diligent::float4& color, float depth, uint8_t stencil) {
	if (((int)clearFlags & (int)pgClearFlags::Color) != 0) {
		auto rtv = getRenderTargetView();
		if (rtv) {
			pgApp::s_ctx->ClearRenderTarget(rtv, &color.r,
				Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		}
	}

	if ((((int)clearFlags & (int)pgClearFlags::Depth) != 0) ||
		(((int)clearFlags & (int)pgClearFlags::Stencil) != 0)) {
		uint32_t flags = 0;
		flags |= ((int)clearFlags & (int)pgClearFlags::Depth) != 0 ? Diligent::CLEAR_DEPTH_FLAG : 0;
		flags |= ((int)clearFlags & (int)pgClearFlags::Stencil) != 0 ? Diligent::CLEAR_STENCIL_FLAG : 0;

		auto dsv = getDepthStencilView();

		if (dsv) {
			pgApp::s_ctx->ClearDepthStencil(dsv, (Diligent::CLEAR_DEPTH_STENCIL_FLAGS)flags,
				depth, stencil, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		}
	}
}

void pgTexture::Copy(pgTexture* dstTexture) {
	Diligent::CopyTextureAttribs CopyAttribs(this->getTexture(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
		dstTexture->getTexture(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	CopyAttribs.SrcMipLevel = 0;
	CopyAttribs.DstMipLevel = 0;
	CopyAttribs.DstSlice = 0;

	pgApp::s_ctx->CopyTexture(CopyAttribs);
}

void pgTexture::Bind(uint32_t ID, Shader::ShaderType shaderType, ShaderParameter::Type parameterType) {
	//
}

void pgTexture::UnBind(uint32_t ID, Shader::ShaderType shaderType, ShaderParameter::Type parameterType) {
	//
}
