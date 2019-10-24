#include "../engine.h"

namespace ade
{

PassPilpeline::PassPilpeline(Technique* parentTechnique, std::shared_ptr<Scene> scene,
                                 std::shared_ptr<Pipeline> pipeline)
    : base(parentTechnique), m_pScene(scene), m_pPipeline(pipeline)
{
    CHECK_ERR(m_pPipeline);
}

PassPilpeline::~PassPilpeline() {}

void PassPilpeline::PreRender()
{
    if (m_pPipeline) {
        m_pPipeline->Bind();
    }
}


void PassPilpeline::Render(Pipeline* pipeline)
{
    if (m_pScene) {
        m_pScene->Accept(*this, m_pPipeline.get());
    }
}
}    // namespace ade