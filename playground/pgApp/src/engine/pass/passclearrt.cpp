#include "passclearrt.h"

namespace ade
{
PassClearRT::PassClearRT(Technique* parentTechnique, std::shared_ptr<RenderTarget> rt,
                         ClearFlags clearFlags, Diligent::float4 clearColor, float clearDepth,
                         uint8_t clearStencil)
    : base(parentTechnique), m_RenderTarget(rt), m_ClearFlags(clearFlags), m_ClearColor(clearColor),
      m_ClearDepth(clearDepth), m_ClearStencil(clearStencil)
{
}

PassClearRT::~PassClearRT() {}

void PassClearRT::Render(Pipeline* pipeline)
{
    if (m_RenderTarget) {
        m_RenderTarget->Clear(m_ClearFlags, m_ClearColor, m_ClearDepth, m_ClearStencil);
    }

    if (m_Texture) {
        m_Texture->Clear(m_ClearFlags, m_ClearColor, m_ClearDepth, m_ClearStencil);
    }
}
}    // namespace ade