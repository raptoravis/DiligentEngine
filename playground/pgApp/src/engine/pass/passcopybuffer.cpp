#include "PassCopyBuffer.h"

namespace ade
{

PassCopyBuffer::PassCopyBuffer(std::shared_ptr<Buffer> dstBuffer,
                               std::shared_ptr<Buffer> srcBuffer)
    : base(0), m_dstBuffer(dstBuffer), m_srcBuffer(srcBuffer)
{
}

PassCopyBuffer::~PassCopyBuffer() {}

// Render a frame
void PassCopyBuffer::Render(Pipeline* pipeline)
{
    CHECK_ERR(m_srcBuffer->GetSize() <= m_dstBuffer->GetSize());

    App::s_ctx->CopyBuffer(m_srcBuffer->GetBuffer(), 0,
                             Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                             m_dstBuffer->GetBuffer(), 0, m_srcBuffer->GetSize(),
                             Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}
}    // namespace ade