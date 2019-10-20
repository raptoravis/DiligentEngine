#include "passdispatch.h"


PassDispatch::PassDispatch(std::shared_ptr<Shader> computeShader, const Diligent::uint3& numGroups)
    : base(nullptr), m_pComputeShader(computeShader), m_NumGroups(numGroups)
{
}

PassDispatch::~PassDispatch() {}

// Render a frame
void PassDispatch::Render()
{
    m_pComputeShader->Dispatch(m_NumGroups);
}
