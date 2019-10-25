#include "techniquedeferred.h"

#include "engine/pass/passclearrt.h"
#include "engine/pass/passcopytexture.h"
#include "engine/pass/passsetrt.h"
#include "engine/scene/sceneass.h"
#include "engine/pipeline/pipelinedebug.h"

#include "../pass/passlight.h"
#include "../pass/passopaque.h"
#include "../pass/passtransparent.h"
#include "../pipeline/pipelinedeferredgeometry.h"
#include "../pipeline/pipelinelightback.h"
#include "../pipeline/pipelinelightdir.h"
#include "../pipeline/pipelinelightfront.h"
#include "../pipeline/pipelinetransparent.h"

namespace ade
{

TechniqueDeferred::TechniqueDeferred(std::shared_ptr<RenderTarget> rt,
                                     std::shared_ptr<Texture> backBuffer)
    : base(rt, backBuffer), m_bDepth(false), m_bDiffuse(false), m_bSpecular(false), m_bNormal(false)

{
    createGBuffers();
    createBuffers();
}

TechniqueDeferred::~TechniqueDeferred() {}

void TechniqueDeferred::createGBuffers()
{
    // Create window-size offscreen render target
    TextureDesc RTColorDesc;
    RTColorDesc.Name = "GBuffer diffuse";
    RTColorDesc.Type = RESOURCE_DIM_TEX_2D;
    RTColorDesc.Width = App::s_desc.Width;
    RTColorDesc.Height = App::s_desc.Height;
    RTColorDesc.MipLevels = 1;
    RTColorDesc.Format = TEX_FORMAT_RGBA8_UNORM;
    // The render target can be bound as a shader resource and as a render target
    RTColorDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
    // Define optimal clear value
    RTColorDesc.ClearValue.Format = RTColorDesc.Format;
    RTColorDesc.ClearValue.Color[0] = 0.f;
    RTColorDesc.ClearValue.Color[1] = 0.f;
    RTColorDesc.ClearValue.Color[2] = 0.f;
    RTColorDesc.ClearValue.Color[3] = 1.f;

    RefCntAutoPtr<ITexture> pDiffuseTex;
    App::s_device->CreateTexture(RTColorDesc, nullptr, &pDiffuseTex);

    std::shared_ptr<Texture> diffuseTexture = std::make_shared<Texture>(pDiffuseTex);

    RTColorDesc.Name = "GBuffer specular";
    RefCntAutoPtr<ITexture> pSpecularTex;
    App::s_device->CreateTexture(RTColorDesc, nullptr, &pSpecularTex);
    std::shared_ptr<Texture> specularTexture = std::make_shared<Texture>(pSpecularTex);

    RTColorDesc.Name = "GBuffer normal";
    RTColorDesc.Format = TEX_FORMAT_RGBA32_FLOAT;

    RefCntAutoPtr<ITexture> pNormalTex;
    App::s_device->CreateTexture(RTColorDesc, nullptr, &pNormalTex);

    std::shared_ptr<Texture> normalTexture = std::make_shared<Texture>(pNormalTex);

    // Create depth buffer
    TextureDesc DepthBufferDesc;
    DepthBufferDesc.Name = "GBuffer depth";
    DepthBufferDesc.Type = RESOURCE_DIM_TEX_2D;
    DepthBufferDesc.Width = App::s_desc.Width;
    DepthBufferDesc.Height = App::s_desc.Height;
    DepthBufferDesc.MipLevels = 1;
    DepthBufferDesc.ArraySize = 1;
    DepthBufferDesc.Format = TEX_FORMAT_D24_UNORM_S8_UINT;
    DepthBufferDesc.SampleCount = App::s_desc.SamplesCount;
    DepthBufferDesc.Usage = USAGE_DEFAULT;
    DepthBufferDesc.BindFlags = BIND_DEPTH_STENCIL | BIND_SHADER_RESOURCE;
    DepthBufferDesc.CPUAccessFlags = CPU_ACCESS_NONE;
    DepthBufferDesc.MiscFlags = MISC_TEXTURE_FLAG_NONE;

    RefCntAutoPtr<ITexture> pDepthStencilTexture;
    App::s_device->CreateTexture(DepthBufferDesc, nullptr, &pDepthStencilTexture);
    m_depthStencilTexture = std::make_shared<Texture>(pDepthStencilTexture);

    //
    m_pGBufferRT = std::make_shared<RenderTarget>();

    auto color0 = m_pRenderTarget->GetTexture(RenderTarget::AttachmentPoint::Color0);

    m_pGBufferRT->AttachTexture(RenderTarget::AttachmentPoint::Color0, color0);
    m_pGBufferRT->AttachTexture(RenderTarget::AttachmentPoint::Color1, diffuseTexture);
    m_pGBufferRT->AttachTexture(RenderTarget::AttachmentPoint::Color2, specularTexture);
    m_pGBufferRT->AttachTexture(RenderTarget::AttachmentPoint::Color3, normalTexture);
    m_pGBufferRT->AttachTexture(RenderTarget::AttachmentPoint::DepthStencil,
                                m_depthStencilTexture);
}


void TechniqueDeferred::createBuffers()
{
    {
        uint32_t bufferSize = sizeof(LightParams);

        m_LightParamsCB = std::make_shared<ConstantBuffer>(bufferSize);
    }

    {
        uint32_t bufferSize = sizeof(ScreenToViewParams);
        m_ScreenToViewParamsCB = std::make_shared<ConstantBuffer>(bufferSize);
    }
}

void TechniqueDeferred::init(const std::shared_ptr<Scene> scene, std::vector<Light>* lights)
{
    this->Set(PassRender::kScreenToViewParams, m_ScreenToViewParamsCB);
    this->Set(PassLight::kLightIndexBuffer, m_LightParamsCB);

    std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(this, m_pGBufferRT);
    AddPass(pSetRTPass);

    std::shared_ptr<PassClearRT> pClearRTPass = std::make_shared<PassClearRT>(this, m_pGBufferRT);
    AddPass(pClearRTPass);

    uint32_t numLights = (uint32_t)lights->size();

    Diligent::ShaderMacroHelper shaderMacros;
    shaderMacros.AddShaderMacro("NUM_LIGHTS", numLights);

#if RIGHT_HANDED
    bool bRightHanded = false;
#else
    bool bRightHanded = false;
#endif
    shaderMacros.AddShaderMacro("RIGHT_HANDED", bRightHanded);

    m_pVertexShader = std::make_shared<Shader>();
    m_pVertexShader->LoadShaderFromFile(Shader::VertexShader, "ForwardRendering.hlsl", "VS_main",
                                        "./resources/shaders", false, shaderMacros);

    m_pPixelShader = std::make_shared<Shader>();
    m_pPixelShader->LoadShaderFromFile(Shader::PixelShader, "ForwardRendering.hlsl", "PS_main",
                                       "./resources/shaders", false, shaderMacros);

    m_pGeometryPixelShader = std::make_shared<Shader>();
    m_pGeometryPixelShader->LoadShaderFromFile(Shader::PixelShader, "DeferredRendering.hlsl",
                                               "PS_Geometry", "./resources/shaders", false,
                                               shaderMacros);

    m_pDeferredLightingPixelShader = std::make_shared<Shader>();
    m_pDeferredLightingPixelShader->LoadShaderFromFile(
        Shader::PixelShader, "DeferredRendering.hlsl", "PS_DeferredLighting", "./resources/shaders",
        false, shaderMacros);

    SamplerDesc linearRepeatSampler{ FILTER_TYPE_LINEAR,   FILTER_TYPE_LINEAR,
                                     FILTER_TYPE_LINEAR,   TEXTURE_ADDRESS_WRAP,
                                     TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP };
    SamplerDesc linearClampSampler{ FILTER_TYPE_LINEAR,    FILTER_TYPE_LINEAR,
                                    FILTER_TYPE_LINEAR,    TEXTURE_ADDRESS_CLAMP,
                                    TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP };

    StaticSamplerDesc m_LinearRepeatSamplerDesc{ SHADER_TYPE_PIXEL, "LinearRepeatSampler",
                                                 linearRepeatSampler };
    StaticSamplerDesc m_LinearClampSamplerDesc{ SHADER_TYPE_PIXEL, "LinearClampSampler",
                                                linearClampSampler };

    m_LinearRepeatSampler = std::make_shared<SamplerState>(m_LinearRepeatSamplerDesc);
    m_LinearClampSampler = std::make_shared<SamplerState>(m_LinearClampSamplerDesc);

    m_pPixelShader->GetShaderParameterByName("LinearRepeatSampler").Set(m_LinearRepeatSampler);

    m_pGeometryPixelShader->GetShaderParameterByName("LinearRepeatSampler")
        .Set(m_LinearRepeatSampler);

    //////////////////////////////////////////////////////////////////////////
    // gbuffer pass
    m_pGeometryPipeline = std::make_shared<PipelineDeferredGeometry>(m_pGBufferRT);
    m_pGeometryPipeline->SetShader(Shader::VertexShader, m_pVertexShader);
    m_pGeometryPipeline->SetShader(Shader::PixelShader, m_pGeometryPixelShader);
    // m_pGeometryPipeline->SetRenderTarget(m_pGBufferRT);

    // not use lights
    std::shared_ptr<PassOpaque> pPassOpaque =
        std::make_shared<PassOpaque>(this, scene, m_pGeometryPipeline, nullptr);
    AddPass(pPassOpaque);

    //////////////////////////////////////////////////////////////////////////
    {
        auto srcTexture = m_depthStencilTexture;
        auto dstTexture =
            m_pRenderTarget->GetTexture(RenderTarget::AttachmentPoint::DepthStencil);

        std::shared_ptr<PassCopyTexture> pCopyTexPass =
            std::make_shared<PassCopyTexture>(this, dstTexture, srcTexture);
        AddPass(pCopyTexPass);
    }

    //////////////////////////////////////////////////////////////////////////
    // light pass
    m_pDepthOnlyRenderTarget = std::make_shared<RenderTarget>();
    m_pDepthOnlyRenderTarget->AttachTexture(
        RenderTarget::AttachmentPoint::DepthStencil,
        m_pRenderTarget->GetTexture(RenderTarget::AttachmentPoint::DepthStencil));

    m_pColorOnlyRenderTarget = std::make_shared<RenderTarget>();
    m_pColorOnlyRenderTarget->AttachTexture(
        RenderTarget::AttachmentPoint::Color0,
        m_pRenderTarget->GetTexture(RenderTarget::AttachmentPoint::Color0));

    std::shared_ptr<PipelineLightFront> pFront =
        std::make_shared<PipelineLightFront>(m_pDepthOnlyRenderTarget);
    pFront->SetShader(Shader::VertexShader, m_pVertexShader);

    std::shared_ptr<PipelineLightBack> pBack = std::make_shared<PipelineLightBack>(m_pRenderTarget);
    pBack->SetShader(Shader::VertexShader, m_pVertexShader);
    pBack->SetShader(Shader::PixelShader, m_pDeferredLightingPixelShader);

    std::shared_ptr<PipelineLightDir> pDir = std::make_shared<PipelineLightDir>(m_pRenderTarget);
    pDir->SetShader(Shader::VertexShader, m_pVertexShader);
    pDir->SetShader(Shader::PixelShader, m_pDeferredLightingPixelShader);

    std::shared_ptr<PassLight> pLightPass =
        std::make_shared<PassLight>(this, m_pGBufferRT, pFront, pBack, pDir, lights);
    AddPass(pLightPass);

    //////////////////////////////////////////////////////////////////////////
    //
    m_pTransparentPipeline = std::make_shared<PipelineTransparent>(m_pRenderTarget);
    m_pTransparentPipeline->SetShader(Shader::VertexShader, m_pVertexShader);
    m_pTransparentPipeline->SetShader(Shader::PixelShader, m_pPixelShader);
    // m_pTransparentPipeline->SetRenderTarget(m_pRenderTarget);

    std::shared_ptr<PassTransparent> pTransparentPass =
        std::make_shared<PassTransparent>(this, scene, m_pTransparentPipeline, lights);
    AddPass(pTransparentPass);

    //////////////////////////////////////////////////////////////////////////
    initDebug();

    {
        auto srcTexture = m_pRenderTarget->GetTexture(RenderTarget::AttachmentPoint::Color0);
        auto dstTexture = m_pBackBuffer;

        std::shared_ptr<PassCopyTexture> pCopyTexPass =
            std::make_shared<PassCopyTexture>(this, dstTexture, srcTexture);
        AddPass(pCopyTexPass);
    }
}

void TechniqueDeferred::initDebug()
{
    Diligent::ShaderMacroHelper shaderMacros;

    Diligent::DepthStencilStateDesc DSStateDesc;
    DSStateDesc.DepthEnable = False;

    // Pipeline for debugging textures on screen.
    // duplicate the DebugTexturePixelShader as otherwise they would share the same debugged texture
    std::shared_ptr<Shader> pDebugTexturePixelShader1 = std::make_shared<Shader>();
    pDebugTexturePixelShader1->LoadShaderFromFile(Shader::PixelShader, "DeferredRendering.hlsl",
                                                  "PS_DebugTexture", "./resources/shaders", false,
                                                  shaderMacros);
    pDebugTexturePixelShader1->GetShaderParameterByName("LinearRepeatSampler")
        .Set(m_LinearRepeatSampler);

    std::shared_ptr<Shader> pDebugTexturePixelShader2 = std::make_shared<Shader>();
    pDebugTexturePixelShader2->LoadShaderFromFile(Shader::PixelShader, "DeferredRendering.hlsl",
                                                  "PS_DebugTexture", "./resources/shaders", false,
                                                  shaderMacros);
    pDebugTexturePixelShader2->GetShaderParameterByName("LinearRepeatSampler")
        .Set(m_LinearRepeatSampler);

    std::shared_ptr<Shader> pDebugTexturePixelShader3 = std::make_shared<Shader>();
    pDebugTexturePixelShader3->LoadShaderFromFile(Shader::PixelShader, "DeferredRendering.hlsl",
                                                  "PS_DebugTexture", "./resources/shaders", false,
                                                  shaderMacros);
    pDebugTexturePixelShader3->GetShaderParameterByName("LinearRepeatSampler")
        .Set(m_LinearRepeatSampler);

    std::shared_ptr<PipelineDebug> pDebugTexturePipeline1 =
        std::make_shared<PipelineDebug>(m_pColorOnlyRenderTarget);
    pDebugTexturePipeline1->SetShader(Shader::VertexShader, m_pVertexShader);
    pDebugTexturePipeline1->SetShader(Shader::PixelShader, pDebugTexturePixelShader1);
    pDebugTexturePipeline1->SetDepthStencilState(DSStateDesc);

    std::shared_ptr<PipelineDebug> pDebugTexturePipeline2 =
        std::make_shared<PipelineDebug>(m_pColorOnlyRenderTarget);
    pDebugTexturePipeline2->SetShader(Shader::VertexShader, m_pVertexShader);
    pDebugTexturePipeline2->SetShader(Shader::PixelShader, pDebugTexturePixelShader2);
    pDebugTexturePipeline2->SetDepthStencilState(DSStateDesc);

    std::shared_ptr<PipelineDebug> pDebugTexturePipeline3 =
        std::make_shared<PipelineDebug>(m_pColorOnlyRenderTarget);
    pDebugTexturePipeline3->SetShader(Shader::VertexShader, m_pVertexShader);
    pDebugTexturePipeline3->SetShader(Shader::PixelShader, pDebugTexturePixelShader3);
    pDebugTexturePipeline3->SetDepthStencilState(DSStateDesc);

    m_pDebugDepthTexturePixelShader = std::make_shared<Shader>();
    m_pDebugDepthTexturePixelShader->LoadShaderFromFile(
        Shader::PixelShader, "DeferredRendering.hlsl", "PS_DebugDepthTexture",
        "./resources/shaders", false, shaderMacros);
    m_pDebugDepthTexturePixelShader->GetShaderParameterByName("LinearRepeatSampler")
        .Set(m_LinearRepeatSampler);
    m_pDebugDepthTexturePipeline = std::make_shared<PipelineDebug>(m_pColorOnlyRenderTarget);
    m_pDebugDepthTexturePipeline->SetShader(Shader::VertexShader, m_pVertexShader);
    m_pDebugDepthTexturePipeline->SetShader(Shader::PixelShader, m_pDebugDepthTexturePixelShader);
    m_pDebugDepthTexturePipeline->SetDepthStencilState(DSStateDesc);

#if RIGHT_HANDED
    bool IsGL = true;
#else
    bool IsGL = false;
#endif

    Diligent::float4x4 orthographicProjection = Diligent::float4x4::Ortho(2.f, 2, 0.f, 1.f, IsGL);

    auto diffuseTexture = m_pGBufferRT->GetTexture(RenderTarget::AttachmentPoint::Color1);
    auto specularTexture = m_pGBufferRT->GetTexture(RenderTarget::AttachmentPoint::Color2);
    auto normalTexture = m_pGBufferRT->GetTexture(RenderTarget::AttachmentPoint::Color3);
    auto depthStencilTexture =
        m_pGBufferRT->GetTexture(RenderTarget::AttachmentPoint::DepthStencil);

#define TRANS_SSX(x) (2 * (x)-1.0f)
#define TRANS_SSY(x) (1.0f - (2 * (x)))

    std::shared_ptr<Scene> debugTextureScene =
        SceneAss::CreateScreenQuad(TRANS_SSX(20 / 1920.f), TRANS_SSX(475 / 1920.f),
                                     TRANS_SSY(1060 / 1280.f), TRANS_SSY(815 / 1280.f), 1);
    m_DebugTexture0Pass = std::make_shared<PassPostprocess>(
        this, debugTextureScene, pDebugTexturePipeline1, orthographicProjection, diffuseTexture);
    m_DebugTexture0Pass->SetEnabled(false);    // Initially disabled.
    AddPass(m_DebugTexture0Pass);

    debugTextureScene =
        SceneAss::CreateScreenQuad(TRANS_SSX(495 / 1920.f), TRANS_SSX(950 / 1920.f),
                                     TRANS_SSY(1060 / 1280.f), TRANS_SSY(815 / 1280.f), 1);
    m_DebugTexture1Pass = std::make_shared<PassPostprocess>(
        this, debugTextureScene, pDebugTexturePipeline2, orthographicProjection, specularTexture);
    m_DebugTexture1Pass->SetEnabled(false);    // Initial disabled.
    AddPass(m_DebugTexture1Pass);

    debugTextureScene =
        SceneAss::CreateScreenQuad(TRANS_SSX(970 / 1920.f), TRANS_SSX(1425 / 1920.f),
                                     TRANS_SSY(1060 / 1280.f), TRANS_SSY(815 / 1280.f), 1);
    m_DebugTexture2Pass = std::make_shared<PassPostprocess>(
        this, debugTextureScene, pDebugTexturePipeline3, orthographicProjection, normalTexture);
    m_DebugTexture2Pass->SetEnabled(false);    // Initially disabled.
    AddPass(m_DebugTexture2Pass);

    debugTextureScene =
        SceneAss::CreateScreenQuad(TRANS_SSX(1445 / 1920.f), TRANS_SSX(1900 / 1920.f),
                                     TRANS_SSY(1060 / 1280.f), TRANS_SSY(815 / 1280.f), 1);
    m_DebugTexture3Pass =
        std::make_shared<PassPostprocess>(this, debugTextureScene, m_pDebugDepthTexturePipeline,
                                          orthographicProjection, depthStencilTexture);
    m_DebugTexture3Pass->SetEnabled(false);    // Initially disabled.
    AddPass(m_DebugTexture3Pass);
}

void TechniqueDeferred::Update()
{
    ImGui::Separator();

    ImGui::Checkbox("diffuse", &m_bDiffuse);
    ImGui::SameLine();

    ImGui::Checkbox("specular", &m_bSpecular);
    ImGui::SameLine();

    ImGui::Checkbox("normal", &m_bNormal);
    ImGui::SameLine();

    ImGui::Checkbox("depth", &m_bDepth);

    m_DebugTexture0Pass->SetEnabled(m_bDiffuse);
    m_DebugTexture1Pass->SetEnabled(m_bSpecular);
    m_DebugTexture2Pass->SetEnabled(m_bNormal);
    m_DebugTexture3Pass->SetEnabled(m_bDepth);
}

}    // namespace ade