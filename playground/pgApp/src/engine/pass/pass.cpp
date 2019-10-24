#include "../engine.h"

namespace ade
{
pgPass::pgPass(pgTechnique* parentTechnique) : m_bEnabled(true), m_parentTechnique(parentTechnique)
{
}

pgPass::~pgPass()
{
    //
}

void pgPass::PreRender()
{
    //
}

void pgPass::Render(pgPipeline* pipeline)
{
    //
}

void pgPass::PostRender()
{
    //
}

void pgPass::Visit(pgScene& scene, pgPipeline* pipeline)
{
    //
}

void pgPass::Visit(pgSceneNode& node, pgPipeline* pipeline)
{
    //
}

void pgPass::Visit(pgMesh& mesh, pgPipeline* pipeline)
{
    mesh.Render(pipeline);
}
}    // namespace ade
