#include "passtransparent.h"

PassTransparent::PassTransparent(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene,
                                 std::shared_ptr<pgPipeline> pipeline,
                                 std::vector<pgLight>* lights)
    : base(parentTechnique, scene, pipeline, lights)
{
}

PassTransparent::~PassTransparent() {}

void PassTransparent::Visit(pgMesh& mesh)
{
    std::shared_ptr<pgMaterial> pMaterial = mesh.getMaterial();
    if (pMaterial && pMaterial->IsTransparent()) {
        SetMaterialData(pMaterial.get());

        mesh.Render();
    }
}
