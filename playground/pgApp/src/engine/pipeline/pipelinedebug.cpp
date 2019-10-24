#include "pipelinedebug.h"

using namespace Diligent;

PipelineDebug::PipelineDebug(std::shared_ptr<pgRenderTarget> rt) : base(rt) {}

PipelineDebug::~PipelineDebug() {}

void PipelineDebug::InitPSODesc()
{
    base::InitPSODesc();
    // Pipeline state object encompasses configuration of all GPU stages

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    m_PSODesc.Name = "PipelineDebug PSO";

    m_PSODesc.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;

    m_PSODesc.GraphicsPipeline.DSVFormat = Diligent::TEX_FORMAT_UNKNOWN;
}

