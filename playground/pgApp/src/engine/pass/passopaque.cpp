#include "passopaque.h"

PassOpaque::PassOpaque(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene,
                       std::shared_ptr<pgPipeline> pipeline, const std::vector<pgLight>& lights)
    : base(parentTechnique, scene, pipeline, lights)
{
}

PassOpaque::~PassOpaque() {}

void PassOpaque::Visit(pgMesh& mesh)
{
    std::shared_ptr<pgMaterial> pMaterial = mesh.getMaterial();
    if (pMaterial && !pMaterial->IsTransparent()) {
        SetMaterialData(pMaterial.get());

        mesh.Render();
    }
}
