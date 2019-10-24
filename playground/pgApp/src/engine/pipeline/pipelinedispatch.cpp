#include "pipelinedispatch.h"

using namespace Diligent;

namespace ade
{


PipelineDispatch::PipelineDispatch(const Diligent::uint3& numGroups)
    : base(nullptr), m_NumGroups(numGroups)
{
}

PipelineDispatch::~PipelineDispatch() {}

void PipelineDispatch::InitPSODesc()
{
    m_PSODesc.Name = "PipelineDispatch PSO";
    // This is a compute pipeline
    m_PSODesc.IsComputePipeline = true;
}

// void SetNumGroups(const Diligent::uint3& numGroups);
const Diligent::uint3 PipelineDispatch::GetNumGroups() const
{
    return m_NumGroups;
}


void PipelineDispatch::Bind()
{
    base::Bind();
}

void PipelineDispatch::UnBind()
{
    base::UnBind();
}


}    // namespace ade