#include "passdispatch.h"
#include "../pipeline/pipelinedispatch.h"

namespace ade
{

PassDispatch::PassDispatch(Technique* parentTechnique, std::shared_ptr<PipelineDispatch> pipeline)
    : base(parentTechnique, nullptr, pipeline)
{
}

PassDispatch::~PassDispatch() {}

void PassDispatch::PreRender()
{
    base::PreRender();
}


void PassDispatch::Render(Pipeline* pipeline)
{
    base::Render(pipeline);

    std::shared_ptr<PipelineDispatch> dispatchPipeline =
        std::dynamic_pointer_cast<PipelineDispatch>(m_pPipeline);

    const auto& numGroups = dispatchPipeline->GetNumGroups();

    Diligent::DispatchComputeAttribs DispatAttribs;
    DispatAttribs.ThreadGroupCountX = numGroups.x;
    DispatAttribs.ThreadGroupCountY = numGroups.y;
    DispatAttribs.ThreadGroupCountZ = numGroups.z;

    App::s_ctx->DispatchCompute(DispatAttribs);
}

void PassDispatch::Dispatch()
{
    PreRender();
    Render(nullptr);
    PostRender();
}
}    // namespace ade