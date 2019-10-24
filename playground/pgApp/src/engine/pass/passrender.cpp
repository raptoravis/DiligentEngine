#include "passrender.h"

const char* pgPassRender::kPerObjectName = "PerObject";
const char* pgPassRender::kMaterialName = "Material";
const char* pgPassRender::kLightsName = "Lights";
const char* pgPassRender::kScreenToViewParams = "ScreenToViewParams";


pgPassRender::pgPassRender(pgTechnique* parentTechnique, std::shared_ptr<pgScene> scene,
                           std::shared_ptr<pgPipeline> pipeline, std::vector<pgLight>* lights)
    : base(parentTechnique, scene, pipeline), m_pLights(lights)
{
}

pgPassRender::~pgPassRender() {}

void pgPassRender::SetPerObjectConstantBufferData(PerObject& perObjectData)
{
    auto perObjectCB =
        std::dynamic_pointer_cast<ConstantBuffer>(m_parentTechnique->Get(kPerObjectName));

    perObjectCB->Set(perObjectData);
}

void pgPassRender::SetMaterialData(pgMaterial* mat)
{
    auto materialCB =
        std::dynamic_pointer_cast<ConstantBuffer>(m_parentTechnique->Get(kMaterialName));

    auto matProperites = mat->GetMaterialProperties();
    materialCB->Set(*matProperites);
}

void pgPassRender::BindPerObjectConstantBuffer(std::shared_ptr<Shader> shader)
{
    if (shader) {
        auto perObjectCB = std::dynamic_pointer_cast<ConstantBuffer>(
            m_parentTechnique->Get(kPerObjectName));

        shader->GetShaderParameterByName(kPerObjectName).Set(perObjectCB);
    }
}

void pgPassRender::BindMaterialConstantBuffer(std::shared_ptr<Shader> shader)
{
    if (shader) {
        auto materialCB = std::dynamic_pointer_cast<ConstantBuffer>(
            m_parentTechnique->Get(kMaterialName));

        shader->GetShaderParameterByName(kMaterialName).Set(materialCB);
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
        std::dynamic_pointer_cast<StructuredBuffer>(m_parentTechnique->Get(kLightsName));

    lightsBuffer->Set((const std::vector<pgLight>&)lights);
}

void pgPassRender::BindLightsBuffer(std::shared_ptr<Shader> shader)
{
    if (shader) {
        auto lightsBuffer = std::dynamic_pointer_cast<StructuredBuffer>(
            m_parentTechnique->Get(kLightsName));

        shader->GetShaderParameterByName(kLightsName).Set(lightsBuffer);
    }
}


void pgPassRender::PreRender()
{
    // base::PreRender();

    if (m_pPipeline) {
        auto vertexShader = m_pPipeline->GetShader(Shader::VertexShader);
        auto pixelShader = m_pPipeline->GetShader(Shader::PixelShader);

        // Make sure the per object constant buffer is bound to the vertex shader.
        BindPerObjectConstantBuffer(vertexShader);

        BindMaterialConstantBuffer(pixelShader);

        if (m_pLights) {
            BindLightsBuffer(pixelShader);
            SetLightsBufferData(*m_pLights);
        }

        m_pPipeline->Bind();
    }
}

void pgPassRender::Render(pgPipeline* pipeline)
{
    if (m_pScene) {
        m_pScene->Accept(*this, m_pPipeline.get());
    }
}

void pgPassRender::PostRender()
{
    if (m_pPipeline) {
        m_pPipeline->UnBind();
    }
}

// Inherited from Visitor
void pgPassRender::Visit(pgScene& scene, pgPipeline* pipeline)
{
    //
}

void pgPassRender::Visit(pgSceneNode& node, pgPipeline* pipeline)
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

void pgPassRender::Visit(pgMesh& mesh, pgPipeline* pipeline)
{
    mesh.Render(m_pPipeline.get());
}
