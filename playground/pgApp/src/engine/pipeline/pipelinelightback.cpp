#include "pipelinelightback.h"

using namespace Diligent;

namespace ade
{

PipelineLightBack::PipelineLightBack(std::shared_ptr<RenderTarget> rt) : base(rt) {}

PipelineLightBack::~PipelineLightBack() {}

void PipelineLightBack::InitPSODesc()
{
    base::InitPSODesc();

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    m_PSODesc.Name = "PipelineLightBack PSO";

    // Cull back faces
    m_PSODesc.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_FRONT;
    m_PSODesc.GraphicsPipeline.RasterizerDesc.DepthClipEnable = False;
#if RIGHT_HANDED
    m_PSODesc.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = RIGHT_HANDED_FRONT_CCW;
#else
    m_PSODesc.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = LEFT_HANDED_FRONT_CCW;
#endif


    m_PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = True;
    m_PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend = BLEND_FACTOR_ONE;
    m_PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend = BLEND_FACTOR_ONE;

    // Enable depth testing and Disable depth writes
    m_PSODesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;
    m_PSODesc.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = False;
    m_PSODesc.GraphicsPipeline.DepthStencilDesc.DepthFunc = COMPARISON_FUNC_GREATER_EQUAL;

    // Render pixel if the depth function passes and the stencil was not un-marked in the previous
    // pass.
    m_PSODesc.GraphicsPipeline.DepthStencilDesc.StencilEnable = True;
    m_PSODesc.GraphicsPipeline.DepthStencilDesc.BackFace.StencilFunc = COMPARISON_FUNC_EQUAL;

    SetStencilRef(1);
}

}    // namespace ade