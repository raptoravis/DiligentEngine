#include "passopaque.h"

namespace ade
{

PassOpaque::PassOpaque(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene,
                       std::shared_ptr<pgPipeline> pipeline, std::vector<pgLight>* lights)
    : base(parentTechnique, scene, pipeline, lights)
{
}

PassOpaque::~PassOpaque() {}

void PassOpaque::Visit(pgMesh& mesh, pgPipeline* pipeline)
{
    std::shared_ptr<pgMaterial> pMaterial = mesh.getMaterial();
    if (pMaterial && !pMaterial->IsTransparent()) {
        SetMaterialData(pMaterial.get());

        mesh.Render(pipeline);
    }
}

void PassOpaque::PreRender()
{
    base::PreRender();
}

void PassOpaque::Render(pgPipeline* pipeline)
{
    base::Render(pipeline);
}
}    // namespace ade