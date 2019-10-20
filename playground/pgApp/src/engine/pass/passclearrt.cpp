#include "passclearrt.h"

PassClearRT::PassClearRT(pgTechnique* parentTechnique, std::shared_ptr<pgRenderTarget> rt,
                         pgClearFlags clearFlags, Diligent::float4 clearColor, float clearDepth,
                         uint8_t clearStencil)
    : base(parentTechnique), m_RenderTarget(rt), m_ClearFlags(clearFlags), m_ClearColor(clearColor),
      m_ClearDepth(clearDepth), m_ClearStencil(clearStencil)
{
}

PassClearRT::~PassClearRT() {}

void PassClearRT::Render()
{
    if (m_RenderTarget) {
        m_RenderTarget->Clear(m_ClearFlags, m_ClearColor, m_ClearDepth, m_ClearStencil);
    }

    if (m_Texture) {
        m_Texture->Clear(m_ClearFlags, m_ClearColor, m_ClearDepth, m_ClearStencil);
    }
}
