#include "../engine.h"

pgRenderTarget::pgRenderTarget()
    : m_Width( 0 )
    , m_Height( 0 )
    , m_bCheckValidity( false )
{
    m_Textures.resize( (size_t)pgRenderTarget::AttachmentPoint::NumAttachmentPoints + 1 );
    m_StructuredBuffers.resize( 8 );
}

pgRenderTarget::~pgRenderTarget()
{
}

void pgRenderTarget::AttachTexture( AttachmentPoint attachment, std::shared_ptr<pgTexture> texture )
{
    m_Textures[(uint32_t)attachment] = texture;

    // Next time the render target is "bound", check that it is valid.
    m_bCheckValidity = true;
}

std::shared_ptr<pgTexture> pgRenderTarget::GetTexture( AttachmentPoint attachment )
{
    return m_Textures[(uint32_t)attachment];
}


void pgRenderTarget::Clear( AttachmentPoint attachment, pgClearFlags clearFlags, const Diligent::float4& color, float depth, uint8_t stencil )
{
    auto texture = m_Textures[(uint32_t)attachment];
    if ( texture )
    {
        texture->Clear( clearFlags, color, depth, stencil );
    }
}

void pgRenderTarget::Clear( pgClearFlags clearFlags, const Diligent::float4& color, float depth, uint8_t stencil )
{
    for ( uint32_t i = 0; i < (uint32_t)AttachmentPoint::NumAttachmentPoints; ++i )
    {
        Clear( (AttachmentPoint)i, clearFlags, color, depth, stencil );
    }
}

void pgRenderTarget::GenerateMipMaps()
{
	assert(0);
    //for ( auto texture : m_Textures )
    //{
    //    if ( texture )
    //    {
    //        texture->GenerateMipMaps();
    //    }
    //}
}

void pgRenderTarget::AttachStructuredBuffer(uint8_t slot, std::shared_ptr<pgBuffer> rwBuffer )
{
    m_StructuredBuffers[slot] = rwBuffer;

    // Next time the render target is "bound", check that it is valid.
    m_bCheckValidity = true;
}

std::shared_ptr<pgBuffer> pgRenderTarget::GetStructuredBuffer(uint8_t slot )
{
    if ( slot < m_StructuredBuffers.size() )
    {
        return m_StructuredBuffers[slot];
    }
    return std::shared_ptr<pgBuffer>();
}


void pgRenderTarget::Resize( uint16_t width, uint16_t height )
{
	assert(0);
    //if ( m_Width != width || m_Height != height )
    //{
    //    m_Width = std::max<uint16_t>( width, 1 );
    //    m_Height = std::max<uint16_t>( height, 1 );
    //    // Resize the attached textures.
    //    for ( auto texture : m_Textures )
    //    {
    //        if ( texture )
    //        {
    //            texture->Resize( m_Width, m_Height );
    //        }
    //    }
    //}
}

void pgRenderTarget::bind()
{
    if ( m_bCheckValidity )
    {
        if ( !IsValid() )
        {
            ReportError( "Invalid render target." );
        }
        m_bCheckValidity = false;
    }

    Diligent::ITextureView* renderTargetViews[8];
	uint32_t numRTVs = 0;

    for ( uint32_t i = 0; i < 8; i++ )
    {
        std::shared_ptr<pgTexture> texture = m_Textures[i];
        if ( texture )
        {
            renderTargetViews[numRTVs++] = texture->GetRenderTargetView();
        }
    }

	Diligent::IBufferView* uavViews[8];
    //uint32_t uavStartSlot = numRTVs;
    uint32_t numUAVs = 0;

    for (uint32_t i = 0; i < 8; i++ )
    {
        std::shared_ptr<pgBuffer> rwbuffer = m_StructuredBuffers[i];
        if ( rwbuffer )
        {
            uavViews[numUAVs++] = rwbuffer->GetUnorderedAccessView();
        }
    }
    
	Diligent::ITextureView* depthStencilView = nullptr;
    std::shared_ptr<pgTexture> depthTexture = m_Textures[(uint32_t)AttachmentPoint::Depth];
    std::shared_ptr<pgTexture> depthStencilTexture = m_Textures[(uint32_t)AttachmentPoint::DepthStencil];

    if ( depthTexture )
    {
        depthStencilView = depthTexture->GetDepthStencilView();
    }
    else if ( depthStencilTexture )
    {
        depthStencilView = depthStencilTexture->GetDepthStencilView();
    }

	pgApp::s_ctx->SetRenderTargets(numRTVs, renderTargetViews, depthStencilView, 
		Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void pgRenderTarget::unbind()
{
	pgApp::s_ctx->SetRenderTargets( 0, nullptr, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

bool pgRenderTarget::IsValid() const
{
    uint32_t numRTV = 0;
    int width = -1;
    int height = -1;

    for ( auto texture : m_Textures )
    {
        if ( texture )
        {
			if (texture->GetRenderTargetView()) {
				++numRTV;
			}

            if ( width == -1 || height == -1 )
            {
                width = texture->GetWidth();
                height = texture->GetHeight();
            }
            else
            {
                if ( texture->GetWidth() != width || texture->GetHeight() != height )
                {
                    return false;
                }
            }
        }
    }

	uint32_t numUAV = 0;
    for ( auto rwBuffer : m_StructuredBuffers )
    {
        if ( rwBuffer )
        {
            ++numUAV;
        }
    }

    if ( numRTV + numUAV > 8 )
    {
        return false;
    }

    return true;
}


