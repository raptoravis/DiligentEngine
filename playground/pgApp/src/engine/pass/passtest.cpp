#include "passtest.h"


TestPass::TestPass(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene,
                   std::shared_ptr<pgPipeline> pipeline)
    : base(parentTechnique), m_pScene(scene), m_pPipeline(pipeline)
{
}

TestPass::~TestPass() {}

void TestPass::SetPerObjectConstantBufferData(PerObject& perObjectData)
{
    auto perObjectCB =
        std::dynamic_pointer_cast<ConstantBuffer>(m_parentTechnique->GetResource(kPerObjectName));

    perObjectCB->Set(perObjectData);
}

void TestPass::BindPerObjectConstantBuffer(std::shared_ptr<Shader> shader)
{
    if (shader) {
        auto perObjectCB = std::dynamic_pointer_cast<ConstantBuffer>(
            m_parentTechnique->GetResource(kPerObjectName));
        shader->GetShaderParameterByName(kPerObjectName).Set(perObjectCB);
    }
}

void TestPass::PreRender()
{
    pgApp::s_eventArgs.pPipeline = m_pPipeline.get();

    if (m_pPipeline) {
        // Make sure the per object constant buffer is bound to the vertex shader.
        BindPerObjectConstantBuffer(m_pPipeline->GetShader(Shader::VertexShader));
        m_pPipeline->Bind();
    }
}

void TestPass::Render()
{
    if (m_pScene) {
        m_pScene->Accept(*this);
    }
}

void TestPass::PostRender()
{
    if (m_pPipeline) {
        m_pPipeline->UnBind();
    }
}

// Inherited from Visitor
void TestPass::Visit(pgScene& scene) {}

void TestPass::Visit(pgSceneNode& node)
{
    pgCamera* camera = pgApp::s_eventArgs.pCamera;
    if (camera) {
        PerObject perObjectData;

        Diligent::float4x4 viewMatrix = camera->getViewMatrix();
        Diligent::float4x4 projMatrix = camera->getProjectionMatrix();

        const Diligent::float4x4 nodeTransform = node.getWorldTransfom();
        Diligent::float4x4 worldView = nodeTransform * viewMatrix;

        Diligent::float4x4 worldViewProjMatrix = worldView * projMatrix;
        perObjectData.ModelViewProjection = worldViewProjMatrix.Transpose();
        // Diligent::float4x4 worldViewProjMatrix = projMatrix * viewMatrix * local;
        // perObjectData.ModelViewProjection = worldViewProjMatrix;

        // Update the constant buffer data
        SetPerObjectConstantBufferData(perObjectData);
    }
}

void TestPass::Visit(pgMesh& mesh)
{
    mesh.Render();
}

