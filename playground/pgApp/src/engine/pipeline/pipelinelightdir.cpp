#include "pipelinelightdir.h"

using namespace Diligent;

namespace ade
{

PipelineLightDir::PipelineLightDir(std::shared_ptr<pgRenderTarget> rt) : base(rt) {}

PipelineLightDir::~PipelineLightDir() {}

void PipelineLightDir::InitPSODesc()
{
    base::InitPSODesc();

    m_PSODesc.Name = "PipelineLightDir PSO";

    auto color0 = m_pRenderTarget->GetTexture(pgRenderTarget::AttachmentPoint::Color0);
    auto color0Format =
        color0 ? color0->GetTexture()->GetDesc().Format : Diligent::TEX_FORMAT_UNKNOWN;

    auto ds = m_pRenderTarget->GetTexture(pgRenderTarget::AttachmentPoint::DepthStencil);
    auto dsFormat = ds ? ds->GetTexture()->GetDesc().Format : pgApp::s_desc.DepthBufferFormat;

    // This tutorial will render to a single render target
    m_PSODesc.GraphicsPipeline.NumRenderTargets = 1;
    // Set render target format which is the format of the swap chain's color buffer
    m_PSODesc.GraphicsPipeline.RTVFormats[0] = color0Format;
    // Set depth buffer format which is the format of the swap chain's back buffer
    m_PSODesc.GraphicsPipeline.DSVFormat = dsFormat;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    m_PSODesc.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Cull back faces
    m_PSODesc.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
#if RIGHT_HANDED
    m_PSODesc.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = RIGHT_HANDED_FRONT_CCW;
#else
    m_PSODesc.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = LEFT_HANDED_FRONT_CCW;
#endif


    m_PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = True;
    m_PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend = BLEND_FACTOR_ONE;
    m_PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend = BLEND_FACTOR_ONE;

    // Enable depth testing
    m_PSODesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;
    m_PSODesc.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = False;
    m_PSODesc.GraphicsPipeline.DepthStencilDesc.DepthFunc = COMPARISON_FUNC_GREATER;
}

}    // namespace ade