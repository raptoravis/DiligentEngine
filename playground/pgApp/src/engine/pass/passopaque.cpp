#include "passopaque.h"

PassOpaque::PassOpaque(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene, std::shared_ptr<pgPipeline> pipeline,
                       const std::vector<pgLight>& lights)
    : base(parentTechnique, scene, pipeline, lights)
{
}

PassOpaque::~PassOpaque() {}

bool PassOpaque::meshFilter(pgMesh* mesh)
{
    auto mat = mesh->getMaterial();
    auto bTransparent = mat->IsTransparent();
    return !bTransparent;
}

// Render a frame
void PassOpaque::render(pgRenderEventArgs& e)
{
    m_pScene->_render(e);
}

void PassOpaque::update(pgRenderEventArgs& e)
{
    //
}

void PassOpaque::bind(pgRenderEventArgs& e, pgBindFlag flag)
{
    base::bind(e, flag);
}

void PassOpaque::unbind(pgRenderEventArgs& e, pgBindFlag flag)
{
    base::unbind(e, flag);
}

void PassOpaque::Visit(pgMesh& mesh)
{
    std::shared_ptr<pgMaterial> pMaterial = mesh.getMaterial();
    if (pMaterial && !pMaterial->IsTransparent()) {
        mesh.Render();
    }
}
