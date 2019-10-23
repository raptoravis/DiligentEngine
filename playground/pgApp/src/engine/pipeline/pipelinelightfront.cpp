#include "PipelineLightFront.h"

using namespace Diligent;

PipelineLightFront::PipelineLightFront(std::shared_ptr<pgRenderTarget> rt) : base(rt) {}

PipelineLightFront::~PipelineLightFront() {}

void PipelineLightFront::InitPSODesc()
{
    base::InitPSODesc();

    m_PSODesc.Name = "PipelineLightFront PSO";

    // auto color0 = m_pRenderTarget->GetTexture(pgRenderTarget::AttachmentPoint::Color0);
    // auto color0Format =
    //    color0 ? color0->GetTexture()->GetDesc().Format : Diligent::TEX_FORMAT_UNKNOWN;

    auto ds = m_pRenderTarget->GetTexture(pgRenderTarget::AttachmentPoint::DepthStencil);
    auto dsFormat = ds ? ds->GetTexture()->GetDesc().Format : pgApp::s_desc.DepthBufferFormat;

    // This tutorial will render to a single render target
    m_PSODesc.GraphicsPipeline.NumRenderTargets = 0;
    // Set render target format which is the format of the swap chain's color buffer
    // m_PSODesc.GraphicsPipeline.RTVFormats[0] = pgApp::s_desc.ColorBufferFormat;
    // Set depth buffer format which is the format of the swap chain's back buffer
    m_PSODesc.GraphicsPipeline.DSVFormat = dsFormat;

    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    m_PSODesc.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    // Cull back faces
    m_PSODesc.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_BACK;
    m_PSODesc.GraphicsPipeline.RasterizerDesc.DepthClipEnable = True;
#if RIGHT_HANDED
    m_PSODesc.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = RIGHT_HANDED_FRONT_CCW;
#else
    m_PSODesc.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = LEFT_HANDED_FRONT_CCW;
#endif

    // Enable depth testing
    m_PSODesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;
    m_PSODesc.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = False;
    m_PSODesc.GraphicsPipeline.DepthStencilDesc.DepthFunc = COMPARISON_FUNC_GREATER;
    m_PSODesc.GraphicsPipeline.DepthStencilDesc.StencilEnable = True;
    m_PSODesc.GraphicsPipeline.DepthStencilDesc.FrontFace.StencilPassOp = STENCIL_OP_DECR_SAT;

    SetStencilRef(1);
}
