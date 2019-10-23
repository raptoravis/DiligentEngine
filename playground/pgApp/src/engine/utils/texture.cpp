#include "../engine.h"

uint16_t pgTexture::GetWidth() const
{
	auto tex = m_pTexture;
	auto& desc = m_pTexture->GetDesc();
	return (uint16_t)desc.Width;
}

uint16_t pgTexture::GetHeight() const
{
	auto tex = m_pTexture;

	auto& desc = tex->GetDesc();
	return (uint16_t)desc.Height;
}

uint16_t pgTexture::GetDepth() const
{
	auto tex = m_pTexture;

	auto& desc = tex->GetDesc();
	return (uint16_t)desc.Depth;
}

uint8_t pgTexture::GetBPP() const
{
	//auto& desc = m_pResource->GetDesc();
	//desc.Format;

	return 4;
}

bool pgTexture::IsTransparent() const
{
	//auto& desc = m_pResource->GetDesc();
	//desc.Format;

	return false;
}

pgTexture::pgTexture(Diligent::ITexture* texture)
{
	m_pTexture.Attach(texture);
	m_pTexture->AddRef();
}

pgTexture::~pgTexture() {
    m_pTexture.Release();
    m_pTexture.Detach();
}

Diligent::ITexture* pgTexture::GetTexture() {
	return m_pTexture;
}


Diligent::ITextureView* pgTexture::GetShaderResourceView()
{
	auto tex = m_pTexture;

	auto srv = tex->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
	return srv;
}

Diligent::ITextureView* pgTexture::GetDepthStencilView() 
{
	auto tex = m_pTexture;

	auto dsv = tex->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
	return dsv;
}

Diligent::ITextureView* pgTexture::GetRenderTargetView() 
{
	auto tex = m_pTexture;

	auto rtv = tex->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);

	return rtv;
}

Diligent::ITextureView* pgTexture::GetUnorderedAccessView() 
{
	auto tex = m_pTexture;

	auto uav = tex->GetDefaultView(Diligent::TEXTURE_VIEW_UNORDERED_ACCESS);
	return uav;
}


void pgTexture::Clear(pgClearFlags clearFlags, const Diligent::float4& color, float depth, uint8_t stencil) {
	if (((int)clearFlags & (int)pgClearFlags::Color) != 0) {
		auto rtv = GetRenderTargetView();
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

		auto dsv = GetDepthStencilView();

		if (dsv) {
			pgApp::s_ctx->ClearDepthStencil(dsv, (Diligent::CLEAR_DEPTH_STENCIL_FLAGS)flags,
				depth, stencil, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		}
	}
}

void pgTexture::Copy(pgTexture* dstTexture) {
	Diligent::CopyTextureAttribs CopyAttribs(this->GetTexture(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
		dstTexture->GetTexture(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
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
