#include "../engine.h"

namespace ade
{
Pass::Pass(Technique* parentTechnique) : m_bEnabled(true), m_parentTechnique(parentTechnique)
{
}

Pass::~Pass()
{
    //
}

void Pass::PreRender()
{
    //
}

void Pass::Render(Pipeline* pipeline)
{
    //
}

void Pass::PostRender()
{
    //
}

void Pass::Visit(Scene& scene, Pipeline* pipeline)
{
    //
}

void Pass::Visit(SceneNode& node, Pipeline* pipeline)
{
    //
}

void Pass::Visit(Mesh& mesh, Pipeline* pipeline)
{
    mesh.Render(pipeline);
}
}    // namespace ade
