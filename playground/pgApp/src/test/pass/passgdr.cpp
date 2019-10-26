#include "passgdr.h"
#include "../pipeline/pipelinegdr.h"

using namespace ade;


PassGdr::PassGdr(Technique* parentTechnique, std::shared_ptr<Scene> scene,
                 std::shared_ptr<Pipeline> pipeline)
    : base(parentTechnique), m_pScene(scene), m_pPipeline(pipeline)
{
}

PassGdr::~PassGdr() {}

void PassGdr::SetColorsMaterialPerObjectConstantBufferData(ColorsMaterial& data)
{
    auto cb = std::dynamic_pointer_cast<ConstantBuffer>(
        m_parentTechnique->Get(PipelineGdr::kColorsMaterialName));

    cb->Set(data);
}

void PassGdr::SetPerObjectConstantBufferData(PerObject& perObjectData)
{
    auto perObjectCB = std::dynamic_pointer_cast<ConstantBuffer>(
        m_parentTechnique->Get(PipelineGdr::kPerObjectName));

    perObjectCB->Set(perObjectData);
}

void PassGdr::BindPerObjectConstantBuffer(std::shared_ptr<Shader> shader)
{
    if (shader) {
        auto perObjectCB = std::dynamic_pointer_cast<ConstantBuffer>(
            m_parentTechnique->Get(PipelineGdr::kPerObjectName));
        shader->GetShaderParameterByName(PipelineGdr::kPerObjectName).Set(perObjectCB);
    }
}

void PassGdr::BindColorsMaterialConstantBuffer(std::shared_ptr<ade::Shader> shader)
{
    if (shader) {
        auto cb = std::dynamic_pointer_cast<ConstantBuffer>(
            m_parentTechnique->Get(PipelineGdr::kColorsMaterialName));
        shader->GetShaderParameterByName(PipelineGdr::kColorsMaterialName).Set(cb);
    }
}

void PassGdr::PreRender()
{
    if (m_pPipeline) {
        // Make sure the per object constant buffer is bound to the vertex shader.
        BindPerObjectConstantBuffer(m_pPipeline->GetShader(Shader::VertexShader));
        BindColorsMaterialConstantBuffer(m_pPipeline->GetShader(Shader::VertexShader));
        BindColorsMaterialConstantBuffer(m_pPipeline->GetShader(Shader::PixelShader));
        m_pPipeline->Bind();
    }
}

void PassGdr::Render(Pipeline* pipeline)
{
    if (m_pScene) {
        m_pScene->Accept(*this, m_pPipeline.get());
    }
}

void PassGdr::PostRender()
{
    if (m_pPipeline) {
        m_pPipeline->UnBind();
    }
}

// Inherited from Visitor
void PassGdr::Visit(Scene& scene, Pipeline* pipeline) {}

void PassGdr::Visit(SceneNode& node, Pipeline* pipeline)
{
    Camera* camera = App::s_eventArgs.pCamera;
    if (camera) {
        PerObject perObjectData;

        // Diligent::float4x4 is column major,
        // in hlsl, when mul(mat, vec), the mat is column major like opengl
        Diligent::float4x4 viewMatrix = camera->getViewMatrix();
        Diligent::float4x4 projMatrix = camera->getProjectionMatrix();

        const Diligent::float4x4 nodeTransform = node.getWorldTransfom();

        // the mat1 * mat2 should be left mul
        Diligent::float4x4 viewProjMatrix = viewMatrix * projMatrix;

        perObjectData.Model = nodeTransform;
        perObjectData.ViewProjection = viewProjMatrix;

        // Update the constant buffer data
        SetPerObjectConstantBufferData(perObjectData);
    }
}

void PassGdr::Visit(Mesh& mesh, Pipeline* pipeline)
{
    mesh.Render(pipeline);
}
