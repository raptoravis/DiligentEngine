#include "passsetrt.h"

PassSetRT::PassSetRT(pgTechnique* parentTechnique, std::shared_ptr<pgRenderTarget> rt)
    : base(parentTechnique), m_pRenderTarget(rt)
{
}

PassSetRT::~PassSetRT() {}

void PassSetRT::Render(pgPipeline* pipeline)
{
    if (m_pRenderTarget) {
        m_pRenderTarget->Bind();
    }
}
