#include "passtransparent.h"


PassTransparent::PassTransparent(Technique* parentTechnique, std::shared_ptr<Scene> scene,
                                 std::shared_ptr<Pipeline> pipeline, std::vector<Light>* lights)
    : base(parentTechnique, scene, pipeline, lights)
{
}

PassTransparent::~PassTransparent() {}

void PassTransparent::Visit(Mesh& mesh, Pipeline* pipeline)
{
    std::shared_ptr<Material> pMaterial = mesh.getMaterial();
    if (pMaterial && pMaterial->IsTransparent()) {
        SetMaterialData(pMaterial.get());

        mesh.Render(pipeline);
    }
}

