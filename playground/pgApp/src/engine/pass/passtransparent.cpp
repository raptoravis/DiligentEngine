#include "passtransparent.h"

PassTransparent::PassTransparent(std::shared_ptr<pgScene> scene,
                                 std::shared_ptr<pgPipeline> pipeline,
                                 const std::vector<pgLight>& lights)
    : base(scene, pipeline, lights)
{
}

PassTransparent::~PassTransparent() {}

bool PassTransparent::meshFilter(pgMesh* mesh)
{
    auto mat = mesh->getMaterial();
    auto bTransparent = mat->IsTransparent();
    return bTransparent;
}

// Render a frame
void PassTransparent::render(pgRenderEventArgs& e)
{
    m_pScene->_render(e);
}

void PassTransparent::update(pgRenderEventArgs& e)
{
    //
}

void PassTransparent::bind(pgRenderEventArgs& e, pgBindFlag flag)
{
    base::bind(e, flag);
}

void PassTransparent::unbind(pgRenderEventArgs& e, pgBindFlag flag)
{
    base::unbind(e, flag);
}

void PassTransparent::Visit(pgMesh& mesh)
{
    std::shared_ptr<pgMaterial> pMaterial = mesh.getMaterial();
    if (pMaterial && pMaterial->IsTransparent()) {
        mesh.Render();
    }
}
