#include <Texture.h>

#include "passpostprocess.h"

namespace ade
{

PassPostprocess::PassPostprocess(Technique* parentTechnique, std::shared_ptr<Scene> scene,
                                 std::shared_ptr<Pipeline> pipeline,
                                 const Diligent::float4x4& projectionMatrix,
                                 std::shared_ptr<Texture> texture)
    : base(parentTechnique, scene, pipeline, nullptr), m_ProjectionMatrix(projectionMatrix),
      m_Texture(texture)
{
}

void PassPostprocess::PreRender()
{
    auto ps = m_pPipeline->GetShader(Shader::ShaderType::PixelShader);

    if (ps) {
        ps->GetShaderParameterByName("DebugTexture").Set(m_Texture);
    }

    base::PreRender();
}

void PassPostprocess::Render(Pipeline* pipeline)
{
    PerObject perObjectData;
    perObjectData.ModelView = Diligent::float4x4::Identity();
    perObjectData.ModelViewProjection = m_ProjectionMatrix;

    SetPerObjectConstantBufferData(perObjectData);

    base::Render(pipeline);
}

void PassPostprocess::Visit(SceneNode& node, Pipeline* pipeline)
{
    // Do nothing in this case
}
}    // namespace ade