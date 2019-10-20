#include "PipelineTransparent.h"

using namespace Diligent;

PipelineTransparent::PipelineTransparent(std::shared_ptr<pgRenderTarget> rt) : base(rt) {}

PipelineTransparent::~PipelineTransparent() {}

void PipelineTransparent::InitPSODesc()
{
    base::InitPSODesc();

    m_PSODesc.Name = "PipelineTransparent PSO";

    auto& alphaBlending = m_PSODesc.GraphicsPipeline.BlendDesc;
    alphaBlending.RenderTargets[0].BlendEnable = True;
    alphaBlending.RenderTargets[0].SrcBlend = BLEND_FACTOR_SRC_ALPHA;
    alphaBlending.RenderTargets[0].DestBlend = BLEND_FACTOR_INV_SRC_ALPHA;
    alphaBlending.RenderTargets[0].SrcBlendAlpha = BLEND_FACTOR_ZERO;
    alphaBlending.RenderTargets[0].DestBlendAlpha = BLEND_FACTOR_ONE;

    auto& DepthStencilDesc = m_PSODesc.GraphicsPipeline.DepthStencilDesc;
    DepthStencilDesc.DepthEnable = True;
    DepthStencilDesc.DepthWriteEnable = False;
    DepthStencilDesc.DepthFunc = COMPARISON_FUNC_LESS_EQUAL;

    auto RasterizerDesc = m_PSODesc.GraphicsPipeline.RasterizerDesc;
    RasterizerDesc.CullMode = CULL_MODE_NONE;
}
