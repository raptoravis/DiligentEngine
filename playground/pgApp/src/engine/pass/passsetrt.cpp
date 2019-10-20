#include "passsetrt.h"

PassSetRT::PassSetRT(pgTechnique* parentTechnique, std::shared_ptr<pgRenderTarget> rt)
    : base(parentTechnique), m_pRT(rt)
{
}

PassSetRT::~PassSetRT() {}

void PassSetRT::Render()
{
    if (m_pRT) {
        m_pRT->Bind();
    }
}
