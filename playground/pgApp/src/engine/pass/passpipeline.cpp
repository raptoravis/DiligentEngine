#include "../engine.h"

namespace ade
{

pgPassPilpeline::pgPassPilpeline(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene,
                                 std::shared_ptr<pgPipeline> pipeline)
    : base(parentTechnique), m_pScene(scene), m_pPipeline(pipeline)
{
    CHECK_ERR(m_pPipeline);
}

pgPassPilpeline::~pgPassPilpeline() {}

void pgPassPilpeline::PreRender()
{
    if (m_pPipeline) {
        m_pPipeline->Bind();
    }
}


void pgPassPilpeline::Render(pgPipeline* pipeline)
{
    if (m_pScene) {
        m_pScene->Accept(*this, m_pPipeline.get());
    }
}
}    // namespace ade