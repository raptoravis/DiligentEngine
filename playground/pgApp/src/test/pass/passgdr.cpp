#include "passgdr.h"

PassGdr::PassGdr(Technique* parentTechnique, std::shared_ptr<Scene> scene,
                       std::shared_ptr<Pipeline> pipeline, std::vector<Light>* lights)
    : base(parentTechnique, scene, pipeline, lights)
{
}

PassGdr::~PassGdr() {}

void PassGdr::Visit(Mesh& mesh, Pipeline* pipeline)
{
    std::shared_ptr<Material> pMaterial = mesh.getMaterial();
    if (pMaterial && !pMaterial->IsTransparent()) {
        SetMaterialData(pMaterial.get());

        mesh.Render(pipeline);
    }
}

void PassGdr::PreRender()
{
    base::PreRender();
}

void PassGdr::Render(Pipeline* pipeline)
{
    base::Render(pipeline);
}
