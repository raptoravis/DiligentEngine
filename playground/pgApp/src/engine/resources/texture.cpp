#include "../engine.h"

namespace ade
{

uint16_t Texture::GetWidth() const
{
    auto tex = m_pTexture;
    auto& desc = m_pTexture->GetDesc();
    return (uint16_t)desc.Width;
}

uint16_t Texture::GetHeight() const
{
    auto tex = m_pTexture;

    auto& desc = tex->GetDesc();
    return (uint16_t)desc.Height;
}

uint16_t Texture::GetDepth() const
{
    auto tex = m_pTexture;

    auto& desc = tex->GetDesc();
    return (uint16_t)desc.Depth;
}

uint8_t Texture::GetBPP() const
{
    // auto& desc = m_pResource->GetDesc();
    // desc.Format;

    return 4;
}

bool Texture::IsTransparent() const
{
    // auto& desc = m_pResource->GetDesc();
    // desc.Format;

    return false;
}

Texture::Texture(Diligent::ITexture* texture) : m_mip((uint32_t)-1)
{
    m_pTexture.Attach(texture);
    m_pTexture->AddRef();
}

Texture::~Texture()
{
    m_pTexture.Release();
    m_pTexture.Detach();
}

Diligent::ITexture* Texture::GetTexture()
{
    return m_pTexture;
}


Diligent::ITextureView* Texture::GetShaderResourceView()
{

    if (m_mip != (uint32_t)-1) {
        auto tex = m_parentTexture;

        Diligent::TextureViewDesc ViewDesc;
        ViewDesc.ViewType = Diligent::TEXTURE_VIEW_SHADER_RESOURCE;
        auto ViewName = Diligent::FormatString("Mip SRV of texture");
        ViewDesc.Name = ViewName.c_str();

        ViewDesc.MostDetailedMip = m_mip;

        Diligent::ITextureView* srv = 0;
        tex->m_pTexture->CreateView(ViewDesc, &srv);

        return srv;
    } else {
        auto tex = m_pTexture;

        auto srv = tex->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
        return srv;
    }
}

Diligent::ITextureView* Texture::GetDepthStencilView()
{
    auto tex = m_pTexture;

    auto dsv = tex->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
    return dsv;
}

Diligent::ITextureView* Texture::GetRenderTargetView()
{
    auto tex = m_pTexture;

    auto rtv = tex->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);

    return rtv;
}

Diligent::ITextureView* Texture::GetUnorderedAccessView()
{
    if (m_mip != (uint32_t)-1) {
        auto tex = m_parentTexture;

        Diligent::TextureViewDesc ViewDesc;
        ViewDesc.ViewType = Diligent::TEXTURE_VIEW_UNORDERED_ACCESS;
        auto ViewName = Diligent::FormatString("Mip SRV of texture");
        ViewDesc.Name = ViewName.c_str();

        ViewDesc.MostDetailedMip = m_mip;

        Diligent::ITextureView* uav = 0;
        tex->m_pTexture->CreateView(ViewDesc, &uav);

        return uav;
    } else {
        auto tex = m_pTexture;

        auto uav = tex->GetDefaultView(Diligent::TEXTURE_VIEW_UNORDERED_ACCESS);
        return uav;
    }
}


Texture::Texture(std::shared_ptr<Texture> parentTexture, uint32_t mip)
    : m_parentTexture(parentTexture), m_mip(mip)
{
    //
}

void Texture::Clear(ClearFlags clearFlags, const Diligent::float4& color, float depth,
                    uint8_t stencil)
{
    if (((int)clearFlags & (int)ClearFlags::Color) != 0) {
        auto rtv = GetRenderTargetView();
        if (rtv) {
            App::s_ctx->ClearRenderTarget(rtv, &color.r,
                                          Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }
    }

    if ((((int)clearFlags & (int)ClearFlags::Depth) != 0) ||
        (((int)clearFlags & (int)ClearFlags::Stencil) != 0)) {
        uint32_t flags = 0;
        flags |= ((int)clearFlags & (int)ClearFlags::Depth) != 0 ? Diligent::CLEAR_DEPTH_FLAG : 0;
        flags |=
            ((int)clearFlags & (int)ClearFlags::Stencil) != 0 ? Diligent::CLEAR_STENCIL_FLAG : 0;

        auto dsv = GetDepthStencilView();

        if (dsv) {
            App::s_ctx->ClearDepthStencil(dsv, (Diligent::CLEAR_DEPTH_STENCIL_FLAGS)flags, depth,
                                          stencil,
                                          Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }
    }
}

void Texture::Copy(Texture* dstTexture)
{
    Diligent::CopyTextureAttribs CopyAttribs(
        this->GetTexture(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
        dstTexture->GetTexture(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    CopyAttribs.SrcMipLevel = 0;
    CopyAttribs.DstMipLevel = 0;
    CopyAttribs.DstSlice = 0;

    App::s_ctx->CopyTexture(CopyAttribs);
}

void Texture::Bind(uint32_t ID, Shader::ShaderType shaderType, ShaderParameter::Type parameterType)
{
    //
}

void Texture::UnBind(uint32_t ID, Shader::ShaderType shaderType,
                     ShaderParameter::Type parameterType)
{
    //
}

}    // namespace ade