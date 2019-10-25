#include "passopaque.h"

PassOpaque::PassOpaque(Technique* parentTechnique, std::shared_ptr<Scene> scene,
                       std::shared_ptr<Pipeline> pipeline, std::vector<Light>* lights)
    : base(parentTechnique, scene, pipeline, lights)
{
}

PassOpaque::~PassOpaque() {}

void PassOpaque::Visit(Mesh& mesh, Pipeline* pipeline)
{
    std::shared_ptr<Material> pMaterial = mesh.getMaterial();
    if (pMaterial && !pMaterial->IsTransparent()) {
        SetMaterialData(pMaterial.get());

        mesh.Render(pipeline);
    }
}

void PassOpaque::PreRender()
{
    base::PreRender();
}

void PassOpaque::Render(Pipeline* pipeline)
{
    base::Render(pipeline);
}
