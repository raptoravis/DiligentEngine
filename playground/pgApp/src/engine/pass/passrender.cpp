#include "passrender.h"

pgPassRender::pgPassRender(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene,
                           std::shared_ptr<pgPipeline> pipeline, const std::vector<pgLight>& lights)
    : base(parentTechnique, scene, pipeline), m_Lights(lights)
{
}

pgPassRender::~pgPassRender() {}

void pgPassRender::SetPerObjectConstantBufferData(PerObject& perObjectData)
{
    auto perObjectCB =
        std::dynamic_pointer_cast<ConstantBuffer>(m_parentTechnique->GetResource(kPerObjectName));

    perObjectCB->Set(perObjectData);
}

void pgPassRender::SetMaterialData(pgMaterial* mat) {
    auto materialCB =
        std::dynamic_pointer_cast<ConstantBuffer>(m_parentTechnique->GetResource(kMaterialName));
    
	auto matProperites = mat->GetMaterialProperties();
    materialCB->Set(*matProperites);
}

void pgPassRender::BindPerObjectConstantBuffer(std::shared_ptr<Shader> shader)
{
    if (shader) {
        auto perObjectCB = std::dynamic_pointer_cast<ConstantBuffer>(
            m_parentTechnique->GetResource(kPerObjectName));

        shader->GetShaderParameterByName(kPerObjectName).SetResource(perObjectCB);
    }
}

void pgPassRender::BindMaterialConstantBuffer(std::shared_ptr<Shader> shader)
{
    if (shader) {
        auto materialCB = std::dynamic_pointer_cast<ConstantBuffer>(
            m_parentTechnique->GetResource(kMaterialName));

        shader->GetShaderParameterByName(kMaterialName).SetResource(materialCB);
    }
}

void pgPassRender::SetLightsBufferData(std::vector<pgLight>& lights)
{
    const float4x4 viewMatrix = pgApp::s_eventArgs.pCamera->getViewMatrix();

    // Update the viewspace vectors of the light.
    for (uint32_t i = 0; i < lights.size(); i++) {
        // Update the lights so that their position and direction are in view space.
        pgLight& light = lights[i];
        light.m_PositionVS = float4(light.m_PositionWS, 1) * viewMatrix;
        light.m_DirectionVS = normalize(float4(light.m_DirectionWS, 0) * viewMatrix);
    }

    auto lightsBuffer =
        std::dynamic_pointer_cast<StructuredBuffer>(m_parentTechnique->GetResource(kLightsName));

    lightsBuffer->Set((const std::vector<pgLight>&)lights);
}

void pgPassRender::BindLightsBuffer(std::shared_ptr<Shader> shader)
{
    if (shader) {
        auto lightsBuffer = std::dynamic_pointer_cast<StructuredBuffer>(
            m_parentTechnique->GetResource(kLightsName));

        shader->GetShaderParameterByName(kLightsName).SetResource(lightsBuffer);
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
        BindLightsBuffer(m_pPipeline->GetShader(Shader::PixelShader));
        SetLightsBufferData(m_Lights);

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
    pgRenderEventArgs& e = pgApp::s_eventArgs;

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
