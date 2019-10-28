#include "passrender.h"

const char* PassRender::kPerObjectName = "PerObject";
const char* PassRender::kMaterialName = "Material";
const char* PassRender::kLightsName = "Lights";
const char* PassRender::kScreenToViewParams = "ScreenToViewParams";


PassRender::PassRender(Technique* parentTechnique, std::shared_ptr<Scene> scene,
                           std::shared_ptr<Pipeline> pipeline, std::vector<Light>* lights)
    : base(parentTechnique, scene, pipeline), m_pLights(lights)
{
}

PassRender::~PassRender() {}

void PassRender::SetPerObjectConstantBufferData(PerObject& perObjectData)
{
    auto perObjectCB =
        std::dynamic_pointer_cast<ConstantBuffer>(m_parentTechnique->Get(kPerObjectName));

    perObjectCB->Set(perObjectData);
}

void PassRender::SetMaterialData(Material* mat)
{
    auto materialCB =
        std::dynamic_pointer_cast<ConstantBuffer>(m_parentTechnique->Get(kMaterialName));

    auto matProperites = mat->GetMaterialProperties();
    materialCB->Set(*matProperites);
}

void PassRender::BindPerObjectConstantBuffer(std::shared_ptr<Shader> shader)
{
    if (shader) {
        auto perObjectCB =
            std::dynamic_pointer_cast<ConstantBuffer>(m_parentTechnique->Get(kPerObjectName));

        shader->GetShaderParameterByName(kPerObjectName).Set(perObjectCB);
    }
}

void PassRender::BindMaterialConstantBuffer(std::shared_ptr<Shader> shader)
{
    if (shader) {
        auto materialCB =
            std::dynamic_pointer_cast<ConstantBuffer>(m_parentTechnique->Get(kMaterialName));

        shader->GetShaderParameterByName(kMaterialName).Set(materialCB);
    }
}

void PassRender::SetLightsBufferData(std::vector<Light>& lights)
{
    const float4x4 viewMatrix = App::s_eventArgs.pCamera->GetViewMatrix();

    // Update the viewspace vectors of the light.
    for (uint32_t i = 0; i < lights.size(); i++) {
        // Update the lights so that their position and direction are in view space.
        Light& light = lights[i];
        light.m_PositionVS = float4(light.m_PositionWS, 1) * viewMatrix;
        light.m_DirectionVS = normalize(float4(light.m_DirectionWS, 0) * viewMatrix);
    }

    auto lightsBuffer =
        std::dynamic_pointer_cast<StructuredBuffer>(m_parentTechnique->Get(kLightsName));

    lightsBuffer->Set((const std::vector<Light>&)lights);
}

void PassRender::BindLightsBuffer(std::shared_ptr<Shader> shader)
{
    if (shader) {
        auto lightsBuffer =
            std::dynamic_pointer_cast<StructuredBuffer>(m_parentTechnique->Get(kLightsName));

        shader->GetShaderParameterByName(kLightsName).Set(lightsBuffer);
    }
}


void PassRender::PreRender()
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

void PassRender::Render(Pipeline* pipeline)
{
    if (m_pScene) {
        m_pScene->Accept(*this, m_pPipeline.get());
    }
}

void PassRender::PostRender()
{
    if (m_pPipeline) {
        m_pPipeline->UnBind();
    }
}

// Inherited from Visitor
void PassRender::Visit(Scene& scene, Pipeline* pipeline)
{
    //
}

void PassRender::Visit(SceneNode& node, Pipeline* pipeline)
{
    RenderEventArgs& e = App::s_eventArgs;

    const float4x4 view = e.pCamera->GetViewMatrix();
    // TODO: change to use world
    const float4x4 local = node.GetLocalTransform();

    // Set cube world view matrix
    float4x4 worldViewMatrix = local * view;
    auto& Proj = e.pCamera->GetProjectionMatrix();
    // Compute world-view-projection matrix
    float4x4 worldViewProjMatrix = worldViewMatrix * Proj;

    // CBConstants->ModelViewProjection = m_WorldViewProjMatrix.Transpose();
    // CBConstants->ModelView = m_WorldViewMatrix.Transpose();
    PerObject perObjectData;

    perObjectData.ModelViewProjection = worldViewProjMatrix;
    perObjectData.ModelView = worldViewMatrix;

    SetPerObjectConstantBufferData(perObjectData);
}

void PassRender::Visit(Mesh& mesh, Pipeline* pipeline)
{
    mesh.Render(m_pPipeline.get());
}

