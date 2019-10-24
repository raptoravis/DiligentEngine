#include <Texture.h>

#include "passpostprocess.h"

PassPostprocess::PassPostprocess(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene,
                                 std::shared_ptr<pgPipeline> pipeline,
                                 const Diligent::float4x4& projectionMatrix,
                                 std::shared_ptr<pgTexture> texture)
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

void PassPostprocess::Render(pgPipeline* pipeline)
{
    PerObject perObjectData;
    perObjectData.ModelView = Diligent::float4x4::Identity();
    perObjectData.ModelViewProjection = m_ProjectionMatrix;

    SetPerObjectConstantBufferData(perObjectData);

    base::Render(pipeline);
}

void PassPostprocess::Visit(pgSceneNode& node, pgPipeline* pipeline)
{
    // Do nothing in this case
}
