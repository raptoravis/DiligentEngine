#include "passrender.h"

pgPassRender::pgPassRender(const pgPassRenderCreateInfo& ci)
    : base(ci.scene, ci.pipeline)
	, m_PerObjectConstants(ci.PerObjectConstants)
	, m_MaterialConstants(ci.MaterialConstants)
	, m_LightsStructuredBuffer(ci.LightsStructuredBuffer)
{
}

pgPassRender::~pgPassRender() {}

// Render a frame
void pgPassRender::render(pgRenderEventArgs& e)
{
    m_pScene->_render(e);
}

void pgPassRender::update(pgRenderEventArgs& e)
{
    //
}

void pgPassRender::bind(pgRenderEventArgs& e, pgBindFlag flag) {}

void pgPassRender::unbind(pgRenderEventArgs& e, pgBindFlag flag)
{
    //
}

void pgPassRender::SetPerObjectConstantBufferData(PerObject& perObjectData)
{
    auto perObjectCB = std::dynamic_pointer_cast<ConstantBuffer>(
        pgApp::s_reources[pgApp::RESOURCE_SLOT_CB_PEROBJECT]);

    perObjectCB->Set(perObjectData);
}

void pgPassRender::BindPerObjectConstantBuffer(std::shared_ptr<Shader> shader)
{
    if (shader) {
        const char* perObjectCBName = pgApp::s_reourceNames[pgApp::RESOURCE_SLOT_CB_PEROBJECT];

        auto perObjectCB = std::dynamic_pointer_cast<ConstantBuffer>(
            pgApp::s_reources[pgApp::RESOURCE_SLOT_CB_PEROBJECT]);
        shader->GetShaderParameterByName(perObjectCBName).SetResource(perObjectCB);
    }
}

void pgPassRender::SetMaterialConstantBufferData(pgMaterial::MaterialProperties& materialData)
{
    auto materialCB = std::dynamic_pointer_cast<ConstantBuffer>(
        pgApp::s_reources[pgApp::RESOURCE_SLOT_CB_MATERIAL]);

    materialCB->Set(materialData);
}

void pgPassRender::BindMaterialConstantBuffer(std::shared_ptr<Shader> shader)
{
    if (shader) {
        const char* materialCBName = pgApp::s_reourceNames[pgApp::RESOURCE_SLOT_CB_MATERIAL];

        auto materialCB = std::dynamic_pointer_cast<ConstantBuffer>(
            pgApp::s_reources[pgApp::RESOURCE_SLOT_CB_MATERIAL]);
        shader->GetShaderParameterByName(materialCBName).SetResource(materialCB);
    }
}



void pgPassRender::PreRender()
{
    // pgApp::s_eventArgs.pPass = this;
    pgApp::s_eventArgs.pPipeline = m_pPipeline.get();

    if (m_pPipeline) {
        // Make sure the per object constant buffer is bound to the vertex shader.
        BindPerObjectConstantBuffer(m_pPipeline->GetShader(Shader::VertexShader));
        BindMaterialConstantBuffer(m_pPipeline->GetShader(Shader::PixelShader));
        m_pPipeline->Bind();
    }
}

void pgPassRender::Render()
{
    if (m_pScene) {
        m_pScene->Accept(*this);
    }
}

void pgPassRender::PostRender()
{
    if (m_pPipeline) {
        m_pPipeline->UnBind();
    }
}

// Inherited from Visitor
void pgPassRender::Visit(pgScene& scene)
{
    //
}

void pgPassRender::Visit(pgSceneNode& node)
{
    auto e = pgApp::s_eventArgs;

    const float4x4 view = e.pCamera->getViewMatrix();
	// TODO: change to use world
    const float4x4 local = node.getLocalTransform();

    // Set cube world view matrix
    float4x4 worldViewMatrix = local * view;
    auto& Proj = e.pCamera->getProjectionMatrix();
    // Compute world-view-projection matrix
    float4x4 worldViewProjMatrix = worldViewMatrix * Proj;

    // CBConstants->ModelViewProjection = m_WorldViewProjMatrix.Transpose();
    // CBConstants->ModelView = m_WorldViewMatrix.Transpose();
    PerObject perObjectData;

    perObjectData.ModelViewProjection = worldViewProjMatrix;
    perObjectData.ModelView = worldViewMatrix;

    SetPerObjectConstantBufferData(perObjectData);
}

void pgPassRender::Visit(pgMesh& mesh)
{
    mesh.Render();
}
