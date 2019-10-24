#include "passsetrt.h"

namespace ade
{

PassSetRT::PassSetRT(Technique* parentTechnique, std::shared_ptr<RenderTarget> rt)
    : base(parentTechnique), m_pRenderTarget(rt)
{
}

PassSetRT::~PassSetRT() {}

void PassSetRT::Render(Pipeline* pipeline)
{
    if (m_pRenderTarget) {
        m_pRenderTarget->Bind();
    }
}

}    // namespace ade