#include "passtest.h"

TestPass::TestPass(Technique* parentTechnique, std::shared_ptr<Scene> scene,
                   std::shared_ptr<Pipeline> pipeline)
    : base(parentTechnique), m_pScene(scene), m_pPipeline(pipeline)
{
}

TestPass::~TestPass() {}

void TestPass::SetPerObjectConstantBufferData(PerObject& perObjectData)
{
    auto perObjectCB =
        std::dynamic_pointer_cast<ConstantBuffer>(m_parentTechnique->Get(kPerObjectName));

    perObjectCB->Set(perObjectData);
}

void TestPass::BindPerObjectConstantBuffer(std::shared_ptr<Shader> shader)
{
    if (shader) {
        auto perObjectCB =
            std::dynamic_pointer_cast<ConstantBuffer>(m_parentTechnique->Get(kPerObjectName));
        shader->GetShaderParameterByName(kPerObjectName).Set(perObjectCB);
    }
}

void TestPass::PreRender()
{
    if (m_pPipeline) {
        // Make sure the per object constant buffer is bound to the vertex shader.
        BindPerObjectConstantBuffer(m_pPipeline->GetShader(Shader::VertexShader));
        m_pPipeline->Bind();
    }
}

void TestPass::Render(Pipeline* pipeline)
{
    if (m_pScene) {
        m_pScene->Accept(*this, m_pPipeline.get());
    }
}

void TestPass::PostRender()
{
    if (m_pPipeline) {
        m_pPipeline->UnBind();
    }
}

// Inherited from Visitor
void TestPass::Visit(Scene& scene, Pipeline* pipeline) {}

void TestPass::Visit(SceneNode& node, Pipeline* pipeline)
{
    Camera* camera = App::s_eventArgs.pCamera;
    if (camera) {
        PerObject perObjectData;

        // Diligent::float4x4 is column major,
        // in hlsl, when mul(mat, vec), the mat is column major like opengl
        Diligent::float4x4 viewMatrix = camera->getViewMatrix();
        Diligent::float4x4 projMatrix = camera->getProjectionMatrix();

        const Diligent::float4x4 nodeTransform = node.GetWorldTransfom();

        // the mat1 * mat2 should be left mul
        Diligent::float4x4 worldView = nodeTransform * viewMatrix;
        Diligent::float4x4 worldViewProjMatrix = worldView * projMatrix;

        // as cube.vsh, mul(vec, mat), mat should be row major, so to transpose it
        perObjectData.ModelViewProjection = worldViewProjMatrix.Transpose();

        // Update the constant buffer data
        SetPerObjectConstantBufferData(perObjectData);
    }
}

void TestPass::Visit(Mesh& mesh, Pipeline* pipeline)
{
    mesh.Render(pipeline);
}

