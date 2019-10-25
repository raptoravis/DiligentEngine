#include "pipelinegdr.h"

using namespace Diligent;
using namespace ade;

PipelineGdr::PipelineGdr(std::shared_ptr<RenderTarget> rt) : base(rt) {}

PipelineGdr::~PipelineGdr() {}

void PipelineGdr::InitPSODesc()
{
    base::InitPSODesc();

    m_PSODesc.Name = "PipelineGdr PSO";

    auto& DepthStencilDesc = m_PSODesc.GraphicsPipeline.DepthStencilDesc;
    DepthStencilDesc.DepthEnable = True;
    DepthStencilDesc.DepthWriteEnable = False;
    DepthStencilDesc.DepthFunc = COMPARISON_FUNC_LESS_EQUAL;
}

