#include "techniquedeferred.h"

#include "../pass/passclearrt.h"
#include "../pass/passcopytexture.h"
#include "../pass/passlight.h"
#include "../pass/passopaque.h"
#include "../pass/passsetrt.h"
#include "../pass/passtransparent.h"

#include "../pipeline/pipelinedebug.h"
#include "../pipeline/pipelinedeferredgeometry.h"
#include "../pipeline/pipelinelightback.h"
#include "../pipeline/pipelinelightdir.h"
#include "../pipeline/pipelinelightfront.h"
#include "../pipeline/pipelinetransparent.h"

#include "../scene/sceneass.h"

TechniqueDeferred::TechniqueDeferred(std::shared_ptr<pgRenderTarget> rt,
                                     std::shared_ptr<pgTexture> backBuffer)
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
    RTColorDesc.Width = pgApp::s_desc.Width;
    RTColorDesc.Height = pgApp::s_desc.Height;
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
    pgApp::s_device->CreateTexture(RTColorDesc, nullptr, &pDiffuseTex);

    std::shared_ptr<pgTexture> diffuseTexture = std::make_shared<pgTexture>(pDiffuseTex);

    RTColorDesc.Name = "GBuffer specular";
    RefCntAutoPtr<ITexture> pSpecularTex;
    pgApp::s_device->CreateTexture(RTColorDesc, nullptr, &pSpecularTex);
    std::shared_ptr<pgTexture> specularTexture = std::make_shared<pgTexture>(pSpecularTex);

    RTColorDesc.Name = "GBuffer normal";
    RTColorDesc.Format = TEX_FORMAT_RGBA32_FLOAT;

    RefCntAutoPtr<ITexture> pNormalTex;
    pgApp::s_device->CreateTexture(RTColorDesc, nullptr, &pNormalTex);

    std::shared_ptr<pgTexture> normalTexture = std::make_shared<pgTexture>(pNormalTex);

    // Create depth buffer
    TextureDesc DepthBufferDesc;
    DepthBufferDesc.Name = "GBuffer depth";
    DepthBufferDesc.Type = RESOURCE_DIM_TEX_2D;
    DepthBufferDesc.Width = pgApp::s_desc.Width;
    DepthBufferDesc.Height = pgApp::s_desc.Height;
    DepthBufferDesc.MipLevels = 1;
    DepthBufferDesc.ArraySize = 1;
    DepthBufferDesc.Format = TEX_FORMAT_D24_UNORM_S8_UINT;
    DepthBufferDesc.SampleCount = pgApp::s_desc.SamplesCount;
    DepthBufferDesc.Usage = USAGE_DEFAULT;
    DepthBufferDesc.BindFlags = BIND_DEPTH_STENCIL | BIND_SHADER_RESOURCE;
    DepthBufferDesc.CPUAccessFlags = CPU_ACCESS_NONE;
    DepthBufferDesc.MiscFlags = MISC_TEXTURE_FLAG_NONE;

    RefCntAutoPtr<ITexture> pDepthStencilTexture;
    pgApp::s_device->CreateTexture(DepthBufferDesc, nullptr, &pDepthStencilTexture);
    m_depthStencilTexture = std::make_shared<pgTexture>(pDepthStencilTexture);

    //
    m_pGBufferRT = std::make_shared<pgRenderTarget>();

    auto color0 = m_pRenderTarget->GetTexture(pgRenderTarget::AttachmentPoint::Color0);

    m_pGBufferRT->AttachTexture(pgRenderTarget::AttachmentPoint::Color0, color0);
    m_pGBufferRT->AttachTexture(pgRenderTarget::AttachmentPoint::Color1, diffuseTexture);
    m_pGBufferRT->AttachTexture(pgRenderTarget::AttachmentPoint::Color2, specularTexture);
    m_pGBufferRT->AttachTexture(pgRenderTarget::AttachmentPoint::Color3, normalTexture);
    m_pGBufferRT->AttachTexture(pgRenderTarget::AttachmentPoint::DepthStencil,
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

void TechniqueDeferred::init(const std::shared_ptr<pgScene> scene, std::vector<pgLight>* lights)
{
    this->Set(pgPassRender::kScreenToViewParams, m_ScreenToViewParamsCB);
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

    g_pVertexShader = std::make_shared<Shader>();
    g_pVertexShader->LoadShaderFromFile(Shader::VertexShader, "ForwardRendering.hlsl", "VS_main",
                                        "./resources/shaders", false, shaderMacros);

    g_pPixelShader = std::make_shared<Shader>();
    g_pPixelShader->LoadShaderFromFile(Shader::PixelShader, "ForwardRendering.hlsl", "PS_main",
                                       "./resources/shaders", false, shaderMacros);

    g_pGeometryPixelShader = std::make_shared<Shader>();
    g_pGeometryPixelShader->LoadShaderFromFile(Shader::PixelShader, "DeferredRendering.hlsl",
                                               "PS_Geometry", "./resources/shaders", false,
                                               shaderMacros);

    g_pDeferredLightingPixelShader = std::make_shared<Shader>();
    g_pDeferredLightingPixelShader->LoadShaderFromFile(
        Shader::PixelShader, "DeferredRendering.hlsl", "PS_DeferredLighting", "./resources/shaders",
        false, shaderMacros);

    SamplerDesc linearRepeatSampler{ FILTER_TYPE_LINEAR,   FILTER_TYPE_LINEAR,
                                     FILTER_TYPE_LINEAR,   TEXTURE_ADDRESS_WRAP,
                                     TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP };
    SamplerDesc linearClampSampler{ FILTER_TYPE_LINEAR,    FILTER_TYPE_LINEAR,
                                    FILTER_TYPE_LINEAR,    TEXTURE_ADDRESS_CLAMP,
                                    TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP };

    StaticSamplerDesc g_LinearRepeatSamplerDesc{ SHADER_TYPE_PIXEL, "LinearRepeatSampler",
                                                 linearRepeatSampler };
    StaticSamplerDesc g_LinearClampSamplerDesc{ SHADER_TYPE_PIXEL, "LinearClampSampler",
                                                linearClampSampler };

    g_LinearRepeatSampler = std::make_shared<SamplerState>(g_LinearRepeatSamplerDesc);
    g_LinearClampSampler = std::make_shared<SamplerState>(g_LinearClampSamplerDesc);

    g_pPixelShader->GetShaderParameterByName("LinearRepeatSampler").Set(g_LinearRepeatSampler);

    g_pGeometryPixelShader->GetShaderParameterByName("LinearRepeatSampler")
        .Set(g_LinearRepeatSampler);

    g_pGeometryPipeline = std::make_shared<PipelineDeferredGeometry>(m_pGBufferRT);
    g_pGeometryPipeline->SetShader(Shader::VertexShader, g_pVertexShader);
    g_pGeometryPipeline->SetShader(Shader::PixelShader, g_pGeometryPixelShader);
    // g_pGeometryPipeline->SetRenderTarget(m_pGBufferRT);

    // not use lights
    std::shared_ptr<PassOpaque> pPassOpaque =
        std::make_shared<PassOpaque>(this, scene, g_pGeometryPipeline, nullptr);
    AddPass(pPassOpaque);

    {
        auto srcTexture = m_depthStencilTexture;
        auto dstTexture =
            m_pRenderTarget->GetTexture(pgRenderTarget::AttachmentPoint::DepthStencil);

        std::shared_ptr<PassCopyTexture> pCopyTexPass =
            std::make_shared<PassCopyTexture>(this, dstTexture, srcTexture);
        AddPass(pCopyTexPass);
    }

    g_pDepthOnlyRenderTarget = std::make_shared<pgRenderTarget>();
    g_pDepthOnlyRenderTarget->AttachTexture(
        pgRenderTarget::AttachmentPoint::DepthStencil,
        m_pRenderTarget->GetTexture(pgRenderTarget::AttachmentPoint::DepthStencil));

    g_pColorOnlyRenderTarget = std::make_shared<pgRenderTarget>();
    g_pColorOnlyRenderTarget->AttachTexture(
        pgRenderTarget::AttachmentPoint::Color0,
        m_pRenderTarget->GetTexture(pgRenderTarget::AttachmentPoint::Color0));

    std::shared_ptr<PipelineLightFront> pFront =
        std::make_shared<PipelineLightFront>(g_pDepthOnlyRenderTarget);
    pFront->SetShader(Shader::VertexShader, g_pVertexShader);

    std::shared_ptr<PipelineLightBack> pBack = std::make_shared<PipelineLightBack>(m_pRenderTarget);
    pBack->SetShader(Shader::VertexShader, g_pVertexShader);
    pBack->SetShader(Shader::PixelShader, g_pDeferredLightingPixelShader);

    std::shared_ptr<PipelineLightDir> pDir = std::make_shared<PipelineLightDir>(m_pRenderTarget);
    pDir->SetShader(Shader::VertexShader, g_pVertexShader);
    pDir->SetShader(Shader::PixelShader, g_pDeferredLightingPixelShader);

    std::shared_ptr<PassLight> pLightPass =
        std::make_shared<PassLight>(this, m_pGBufferRT, pFront, pBack, pDir, lights);
    AddPass(pLightPass);

    g_pTransparentPipeline = std::make_shared<PipelineTransparent>(m_pRenderTarget);
    g_pTransparentPipeline->SetShader(Shader::VertexShader, g_pVertexShader);
    g_pTransparentPipeline->SetShader(Shader::PixelShader, g_pPixelShader);
    // g_pTransparentPipeline->SetRenderTarget(m_pRenderTarget);

    std::shared_ptr<PassTransparent> pTransparentPass =
        std::make_shared<PassTransparent>(this, scene, g_pTransparentPipeline, lights);
    AddPass(pTransparentPass);

    //////////////////////////////////////////////////////////////////////////
    initDebug();

    {
        auto srcTexture = m_pRenderTarget->GetTexture(pgRenderTarget::AttachmentPoint::Color0);
        auto dstTexture = m_pBackBuffer;

        std::shared_ptr<PassCopyTexture> pCopyTexPass =
            std::make_shared<PassCopyTexture>(this, dstTexture, srcTexture);
        AddPass(pCopyTexPass);
    }
}

void TechniqueDeferred::initDebug()
{
    Diligent::ShaderMacroHelper shaderMacros;

    // Pipeline for debugging textures on screen.
    g_pDebugTexturePixelShader = std::make_shared<Shader>();
    g_pDebugTexturePixelShader->LoadShaderFromFile(Shader::PixelShader, "DeferredRendering.hlsl",
                                                   "PS_DebugTexture", "./resources/shaders", false,
                                                   shaderMacros);
    g_pDebugTexturePixelShader->GetShaderParameterByName("LinearRepeatSampler")
        .Set(g_LinearRepeatSampler);
    g_pDebugTexturePipeline = std::make_shared<PipelineDebug>(g_pColorOnlyRenderTarget);
    g_pDebugTexturePipeline->SetShader(Shader::VertexShader, g_pVertexShader);
    g_pDebugTexturePipeline->SetShader(Shader::PixelShader, g_pDebugTexturePixelShader);

    g_pDebugDepthTexturePixelShader = std::make_shared<Shader>();
    g_pDebugDepthTexturePixelShader->LoadShaderFromFile(
        Shader::PixelShader, "DeferredRendering.hlsl", "PS_DebugDepthTexture",
        "./resources/shaders", false, shaderMacros);
    g_pDebugDepthTexturePipeline = std::make_shared<PipelineDebug>(g_pColorOnlyRenderTarget);
    g_pDebugDepthTexturePipeline->SetShader(Shader::VertexShader, g_pVertexShader);
    g_pDebugDepthTexturePipeline->SetShader(Shader::PixelShader, g_pDebugDepthTexturePixelShader);

    Diligent::DepthStencilStateDesc DSStateDesc;
    DSStateDesc.DepthEnable = False;

    g_pDebugDepthTexturePipeline->SetDepthStencilState(DSStateDesc);
    g_pDebugTexturePipeline->SetDepthStencilState(DSStateDesc);

#if RIGHT_HANDED
    bool IsGL = true;
#else
    bool IsGL = false;
#endif

    Diligent::float4x4 orthographicProjection = Diligent::float4x4::Ortho(2.f, 2, 0.f, 1.f, IsGL);

    auto diffuseTexture = m_pGBufferRT->GetTexture(pgRenderTarget::AttachmentPoint::Color1);
    auto specularTexture = m_pGBufferRT->GetTexture(pgRenderTarget::AttachmentPoint::Color2);
    auto normalTexture = m_pGBufferRT->GetTexture(pgRenderTarget::AttachmentPoint::Color3);
    auto depthStencilTexture =
        m_pGBufferRT->GetTexture(pgRenderTarget::AttachmentPoint::DepthStencil);

#define TRANS_SSX(x) (2 * (x) - 1.0f)
#define TRANS_SSY(x) (1.0f - (2 * (x)))

    std::shared_ptr<pgScene> debugTextureScene = pgSceneAss::CreateScreenQuad(TRANS_SSX(20 / 1920.f), TRANS_SSX(475 / 1920.f),
                                     TRANS_SSY(1060 / 1280.f), TRANS_SSY(815 / 1280.f), 1);
    g_DebugTexture0Pass = std::make_shared<PassPostprocess>(
        this, debugTextureScene, g_pDebugTexturePipeline, orthographicProjection, diffuseTexture);
    g_DebugTexture0Pass->SetEnabled(false);    // Initially disabled. Enabled with the F1 key.
    AddPass(g_DebugTexture0Pass);

    debugTextureScene = pgSceneAss::CreateScreenQuad(TRANS_SSX(495 / 1920.f), TRANS_SSX(950 / 1920.f),
                                     TRANS_SSY(1060 / 1280.f), TRANS_SSY(815 / 1280.f), 1);
    g_DebugTexture1Pass = std::make_shared<PassPostprocess>(
        this, debugTextureScene, g_pDebugTexturePipeline, orthographicProjection, specularTexture);
    g_DebugTexture1Pass->SetEnabled(false);    // Initial disabled. Enabled with the F2 key.
    AddPass(g_DebugTexture1Pass);

    debugTextureScene = pgSceneAss::CreateScreenQuad(TRANS_SSX(970 / 1920.f), TRANS_SSX(1425 / 1920.f),
                                     TRANS_SSY(1060 / 1280.f), TRANS_SSY(815 / 1280.f), 1);
    g_DebugTexture2Pass = std::make_shared<PassPostprocess>(
        this, debugTextureScene, g_pDebugTexturePipeline, orthographicProjection, normalTexture);
    g_DebugTexture2Pass->SetEnabled(false);    // Initially disabled. Enabled with the F3 key.
    AddPass(g_DebugTexture2Pass);

    debugTextureScene = pgSceneAss::CreateScreenQuad(TRANS_SSX(1445 / 1920.f), TRANS_SSX(1900 / 1920.f),
                                     TRANS_SSY(1060 / 1280.f), TRANS_SSY(815 / 1280.f), 1);
    g_DebugTexture3Pass =
        std::make_shared<PassPostprocess>(this, debugTextureScene, g_pDebugDepthTexturePipeline,
                                          orthographicProjection, depthStencilTexture);
    g_DebugTexture3Pass->SetEnabled(false);    // Initially disabled. Enabled with the F4 key.
    AddPass(g_DebugTexture3Pass);
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

    g_DebugTexture0Pass->SetEnabled(m_bDiffuse);
    g_DebugTexture1Pass->SetEnabled(m_bSpecular);
    g_DebugTexture2Pass->SetEnabled(m_bNormal);
    g_DebugTexture3Pass->SetEnabled(m_bDepth);
}