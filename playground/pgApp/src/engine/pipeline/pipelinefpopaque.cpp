#include "pipelinefpopaque.h"

using namespace Diligent;

namespace ade
{

PipelineFPOpaque::PipelineFPOpaque(std::shared_ptr<pgRenderTarget> rt) : base(rt) {}

PipelineFPOpaque::~PipelineFPOpaque() {}

void PipelineFPOpaque::InitPSODesc()
{
    base::InitPSODesc();

    m_PSODesc.Name = "PipelineFPOpaque PSO";

    auto& DepthStencilDesc = m_PSODesc.GraphicsPipeline.DepthStencilDesc;
    DepthStencilDesc.DepthEnable = True;
    DepthStencilDesc.DepthWriteEnable = False;
    DepthStencilDesc.DepthFunc = COMPARISON_FUNC_LESS_EQUAL;
}

}    // namespace ade