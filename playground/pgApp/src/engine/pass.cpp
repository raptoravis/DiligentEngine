#include "engine.h"

pgPass::pgPass(pgTechnique* parentTechnique)
    : m_bEnabled(true), m_parentTechnique(parentTechnique)
{
}

pgPass::~pgPass()
{
    //
}

void pgPass::_render(pgRenderEventArgs& e)
{
    // keep to restore it
    auto oldPass = e.pPass;

    auto currentPass = this;
    e.pPass = currentPass;
    currentPass->bind(e, pgBindFlag::pgBindFlag_Pass);

    currentPass->render(e);

    currentPass->unbind(e, pgBindFlag::pgBindFlag_Pass);
    e.pPass = oldPass;
}

void pgPass::update(pgRenderEventArgs& e)
{
    //
}

void pgPass::render(pgRenderEventArgs& e)
{
    //
}


void pgPass::bind(pgRenderEventArgs& e, pgBindFlag flag)
{
    //
}

void pgPass::unbind(pgRenderEventArgs& e, pgBindFlag flag)
{
    //
}

void pgPass::PreRender()
{
    //
}

void pgPass::Render()
{
    //
}

void pgPass::PostRender()
{
    //
}

void pgPass::Visit(pgScene& scene)
{
    //
}

void pgPass::Visit(pgSceneNode& node)
{
    //
}

void pgPass::Visit(pgMesh& mesh)
{
    //
}


BasePass::BasePass(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene,
                   std::shared_ptr<pgPipeline> pipeline)
    : base(parentTechnique), m_pScene(scene), m_pPipeline(pipeline)
{
    m_PerObjectData = (PerObject*)_aligned_malloc(sizeof(PerObject), 16);
}

BasePass::~BasePass()
{
    _aligned_free(m_PerObjectData);
}

void BasePass::SetPerObjectConstantBufferData(PerObject& perObjectData)
{
    auto perObjectCB = std::dynamic_pointer_cast<ConstantBuffer>(
        pgApp::s_reources[pgApp::RESOURCE_SLOT_CB_PEROBJECT]);

    perObjectCB->Set(perObjectData);
}

void BasePass::BindPerObjectConstantBuffer(std::shared_ptr<Shader> shader)
{
    if (shader) {
        const char* perObjectCBName = pgApp::s_reourceNames[pgApp::RESOURCE_SLOT_CB_PEROBJECT];

        auto perObjectCB = std::dynamic_pointer_cast<ConstantBuffer>(
            pgApp::s_reources[pgApp::RESOURCE_SLOT_CB_PEROBJECT]);
        shader->GetShaderParameterByName(perObjectCBName).SetResource(perObjectCB);
    }
}

void BasePass::PreRender()
{
    pgApp::s_eventArgs.pPipeline = m_pPipeline.get();

    if (m_pPipeline) {
        // Make sure the per object constant buffer is bound to the vertex shader.
        BindPerObjectConstantBuffer(m_pPipeline->GetShader(Shader::VertexShader));
        m_pPipeline->Bind();
    }
}

void BasePass::Render()
{
    if (m_pScene) {
        m_pScene->Accept(*this);
    }
}

void BasePass::PostRender()
{
    if (m_pPipeline) {
        m_pPipeline->UnBind();
    }
}

// Inherited from Visitor
void BasePass::Visit(pgScene& scene) {}

void BasePass::Visit(pgSceneNode& node)
{
    pgCamera* camera = pgApp::s_eventArgs.pCamera;
    if (camera) {
        PerObject perObjectData;
        // Update the constant buffer data for the node.
        Diligent::float4x4 viewMatrix = camera->getViewMatrix();
        perObjectData.ModelView = viewMatrix * node.getWorldTransfom();
        perObjectData.ModelViewProjection = camera->getProjectionMatrix() * perObjectData.ModelView;

        // Update the constant buffer data
        SetPerObjectConstantBufferData(perObjectData);
    }
}

void BasePass::Visit(pgMesh& mesh)
{
    std::shared_ptr<pgMaterial> pMaterial = mesh.getMaterial();
    if (pMaterial) {
        mesh.Render();
    }
}


TestPass::TestPass(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene,
                   std::shared_ptr<pgPipeline> pipeline)
    : base(parentTechnique), m_pScene(scene), m_pPipeline(pipeline)
{
}

TestPass::~TestPass() {}

void TestPass::SetPerObjectConstantBufferData(PerObject& perObjectData)
{
    auto perObjectCB = std::dynamic_pointer_cast<ConstantBuffer>(
        pgApp::s_reources[pgApp::RESOURCE_SLOT_CB_PEROBJECT]);

    perObjectCB->Set(perObjectData);
}

void TestPass::BindPerObjectConstantBuffer(std::shared_ptr<Shader> shader)
{
    if (shader) {
        const char* perObjectCBName = pgApp::s_reourceNames[pgApp::RESOURCE_SLOT_CB_PEROBJECT];

        auto perObjectCB = std::dynamic_pointer_cast<ConstantBuffer>(
            pgApp::s_reources[pgApp::RESOURCE_SLOT_CB_PEROBJECT]);
        shader->GetShaderParameterByName(perObjectCBName).SetResource(perObjectCB);
    }
}

void TestPass::PreRender()
{
    // pgApp::s_eventArgs.pPass = this;
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


OpaquePass::OpaquePass(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene,
                       std::shared_ptr<pgPipeline> pipeline)
    : base(parentTechnique, scene, pipeline)
{
}

OpaquePass::~OpaquePass() {}

void OpaquePass::Visit(pgMesh& mesh)
{
    std::shared_ptr<pgMaterial> pMaterial = mesh.getMaterial();
    if (pMaterial && !pMaterial->IsTransparent()) {
        mesh.Render();
    }
}
