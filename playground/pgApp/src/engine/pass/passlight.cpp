#include "passlight.h"
#include "../scene/sceneass.h"

#include "../utils/mathutils.h"

namespace ade
{

const char* PassLight::kLightIndexBuffer = "LightIndexBuffer";

static void InitShaderParams(Technique* parentTechnique, Pipeline* pipeline,
                             std::shared_ptr<RenderTarget> GbufferRT)
{
    std::shared_ptr<Shader> pixelShader = pipeline->GetShader(Shader::PixelShader);
    bool bInitGBuffer = !!GbufferRT;
    if (pixelShader) {
        auto lightIndexCB = std::dynamic_pointer_cast<ConstantBuffer>(
            parentTechnique->Get(PassLight::kLightIndexBuffer));
        auto screenToViewParamsCB = std::dynamic_pointer_cast<ConstantBuffer>(
            parentTechnique->Get(PassRender::kScreenToViewParams));

        pixelShader->GetShaderParameterByName(PassLight::kLightIndexBuffer).Set(lightIndexCB);
        pixelShader->GetShaderParameterByName(PassRender::kScreenToViewParams)
            .Set(screenToViewParamsCB);

        auto lightsSB = std::dynamic_pointer_cast<StructuredBuffer>(
            parentTechnique->Get(PassRender::kLightsName));

        pixelShader->GetShaderParameterByName(PassRender::kLightsName).Set(lightsSB);

        if (bInitGBuffer) {
            auto diffuseTex = GbufferRT->GetTexture(RenderTarget::AttachmentPoint::Color1);
            auto specularTex = GbufferRT->GetTexture(RenderTarget::AttachmentPoint::Color2);
            auto normalTex = GbufferRT->GetTexture(RenderTarget::AttachmentPoint::Color3);
            auto depthTex = GbufferRT->GetTexture(RenderTarget::AttachmentPoint::DepthStencil);

            pixelShader->GetShaderParameterByName("DiffuseTextureVS").Set(diffuseTex);
            pixelShader->GetShaderParameterByName("SpecularTextureVS").Set(specularTex);

            pixelShader->GetShaderParameterByName("NormalTextureVS").Set(normalTex);
            pixelShader->GetShaderParameterByName("DepthTextureVS").Set(depthTex);
        }
    }
}


PassLight::PassLight(Technique* parentTechnique, std::shared_ptr<RenderTarget> pGBufferRT,
                     std::shared_ptr<PipelineLightFront> front,
                     std::shared_ptr<PipelineLightBack> back, std::shared_ptr<PipelineLightDir> dir,
                     std::vector<Light>* Lights)
    : base(parentTechnique), m_pGBufferRT(pGBufferRT), m_pLights(Lights), m_LightPipeline0(front),
      m_LightPipeline1(back), m_DirectionalLightPipeline(dir)
{
    m_pPointLightScene = SceneAss::CreateSphere(1.0f);
    m_pSpotLightScene = SceneAss::CreateCylinder(0.0f, 1.0f, 1.0f, float3(0, 0, 1));
#if RIGHT_HANDED
    m_pDirectionalLightScene = SceneAss::CreateScreenQuad(-1, 1, -1, 1, 1);
#else
    m_pDirectionalLightScene = SceneAss::CreateScreenQuad(-1, 1, -1, 1, 1);
#endif

    m_pTechniqueSphere = std::make_shared<Technique>(nullptr, nullptr);
    m_pTechniqueSpot = std::make_shared<Technique>(nullptr, nullptr);
    m_pTechniqueDir = std::make_shared<Technique>(nullptr, nullptr);

    m_pSubPassSphere0 = std::make_shared<PassPilpeline>(m_pTechniqueSphere.get(),
                                                          m_pPointLightScene, m_LightPipeline0);
    m_pSubPassSphere1 = std::make_shared<PassPilpeline>(m_pTechniqueSphere.get(),
                                                          m_pPointLightScene, m_LightPipeline1);

    m_pSubPassSpot0 = std::make_shared<PassPilpeline>(m_pTechniqueSpot.get(), m_pSpotLightScene,
                                                        m_LightPipeline0);
    m_pSubPassSpot1 = std::make_shared<PassPilpeline>(m_pTechniqueSpot.get(), m_pSpotLightScene,
                                                        m_LightPipeline1);

    m_pSubPassDir = std::make_shared<PassPilpeline>(
        m_pTechniqueDir.get(), m_pDirectionalLightScene, m_DirectionalLightPipeline);

    m_pTechniqueSphere->AddPass(m_pSubPassSphere0);
    m_pTechniqueSphere->AddPass(m_pSubPassSphere1);

    m_pTechniqueSpot->AddPass(m_pSubPassSpot0);
    m_pTechniqueSpot->AddPass(m_pSubPassSpot1);

    m_pTechniqueDir->AddPass(m_pSubPassDir);

    InitShaderParams(m_parentTechnique, m_LightPipeline0.get(), nullptr);
    InitShaderParams(m_parentTechnique, m_LightPipeline1.get(), m_pGBufferRT);
    InitShaderParams(m_parentTechnique, m_DirectionalLightPipeline.get(), m_pGBufferRT);
}

PassLight::~PassLight() {}

void PassLight::updateLightParams(const LightParams& lightParam, const Light& light)
{
    RenderEventArgs& e = App::s_eventArgs;

    {
        auto lightIndexCB =
            std::dynamic_pointer_cast<ConstantBuffer>(m_parentTechnique->Get(kLightIndexBuffer));

        LightParams lightParamData;

        lightParamData.m_LightIndex = lightParam.m_LightIndex;
        lightIndexCB->Set(lightParamData);
    }

    {
        auto perObjectCB = std::dynamic_pointer_cast<ConstantBuffer>(
            m_parentTechnique->Get(PassRender::kPerObjectName));

        PassRender::PerObject perObjectData;

        if (light.m_Type == Light::LightType::Directional) {
// CBConstants->ModelViewProjection = m_WorldViewProjMatrix.Transpose();
// CBConstants->ModelView = m_WorldViewMatrix.Transpose();
// bool IsGL = App::s_device->GetDeviceCaps().IsGLDevice();
#if RIGHT_HANDED
            bool IsGL = true;
#else
            bool IsGL = false;
#endif
            Diligent::float4x4 othoMat = Diligent::float4x4::Ortho(2.0f, 2.0f, 0.f, 1.f, IsGL);

            perObjectData.ModelViewProjection = othoMat;
            perObjectData.ModelView = float4x4::Identity();
        } else {
            auto& Proj = e.pCamera->getProjectionMatrix();
            // const float4x4 nodeTransform = e.pSceneNode->getLocalTransform();
            const float4x4 nodeTransform = Diligent::float4x4::Identity();

            Diligent::float4x4 translation =
                Diligent::float4x4::Translation(Diligent::float3(light.m_PositionWS));
            // Create a rotation matrix that rotates the model towards the direction of the light.
            Diligent::float4x4 rotation =
                MakeQuaternionFromTwoVec3(Diligent::float3(0, 0, 1),
                                          normalize(Diligent::float3(light.m_DirectionWS)))
                    .ToMatrix();

            // Compute the scale depending on the light type.
            float scaleX, scaleY, scaleZ;
            // For point lights, we want to scale the geometry by the range of the light.
            scaleX = scaleY = scaleZ = light.m_Range;
            if (light.m_Type == Light::LightType::Spot) {
                // For spotlights, we want to scale the base of the cone by the spotlight angle.
                scaleX = scaleY =
                    std::tan((PI_F / 180.0f) * (light.m_SpotlightAngle)) * light.m_Range;
            }

            Diligent::float4x4 scale =
                Diligent::float4x4::Scale(Diligent::float3(scaleX, scaleY, scaleZ));

            Diligent::float4x4 modelViewMat =
                nodeTransform * scale * rotation * translation * e.pCamera->getViewMatrix();
            perObjectData.ModelView = modelViewMat;
            perObjectData.ModelViewProjection = modelViewMat * Proj;
        }

        perObjectCB->Set(perObjectData);
    }
}

void PassLight::updateScreenToViewParams()
{
    RenderEventArgs& e = App::s_eventArgs;

    {
        auto screenToViewParamsCB = std::dynamic_pointer_cast<ConstantBuffer>(
            m_parentTechnique->Get(PassRender::kScreenToViewParams));

        ScreenToViewParams screenToViewParamsData;

        auto& Proj = e.pCamera->getProjectionMatrix();
        // CBConstants->ModelViewProjection = m_WorldViewProjMatrix.Transpose();
        // CBConstants->ModelView = m_WorldViewMatrix.Transpose();
        screenToViewParamsData.m_InverseProjectionMatrix = Proj.Inverse();
        screenToViewParamsData.m_ScreenDimensions =
            float2((float)App::s_desc.Width, (float)App::s_desc.Height);

        screenToViewParamsCB->Set(screenToViewParamsData);
    }
}

// Render a frame
void PassLight::Render(Pipeline* pipeline)
{
    updateScreenToViewParams();

    if (m_pLights) {
        LightParams lightParams;

        lightParams.m_LightIndex = 0;

        for (const Light& light : *m_pLights) {
            if (light.m_Enabled) {
                // Update the constant buffer for the per-light data.
                updateLightParams(lightParams, light);

                // Clear the stencil buffer for the next light
                m_LightPipeline0->getRenderTarget()->Clear(ClearFlags::Stencil,
                                                           Diligent::float4(0, 0, 0, 0), 1.0f, 1);
                // The other pipelines should have the same render target.. so no need to clear it 3
                // times.

                switch (light.m_Type) {
                case Light::LightType::Point:
                    m_pTechniqueSphere->Render();
                    break;
                case Light::LightType::Spot:
                    m_pTechniqueSpot->Render();
                    break;
                case Light::LightType::Directional:
                    m_pTechniqueDir->Render();
                    break;
                }
            }

            lightParams.m_LightIndex++;
        }
    }
}

void PassLight::PreRender()
{
    // Bind the G-buffer textures to the pixel shader pipeline stage.
    // m_DiffuseTexture->Bind(0, Shader::PixelShader, ShaderParameter::Type::Texture);
    // m_SpecularTexture->Bind(1, Shader::PixelShader, ShaderParameter::Type::Texture);
    // m_NormalTexture->Bind(2, Shader::PixelShader, ShaderParameter::Type::Texture);
    // m_DepthTexture->Bind(3, Shader::PixelShader, ShaderParameter::Type::Texture);
}


void PassLight::PostRender()
{
    base::PostRender();
}

void PassLight::Visit(Scene& scene, Pipeline* pipeline)
{
    //
}

void PassLight::Visit(SceneNode& node, Pipeline* pipeline)
{
    base::Visit(node, pipeline);
}
}    // namespace ade