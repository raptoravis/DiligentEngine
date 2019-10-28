#include "TechniqueForwardPlus.h"

#include "engine/pass/passclearrt.h"
#include "engine/pass/passcopybuffer.h"
#include "engine/pass/passcopytexture.h"
#include "engine/pass/passdispatch.h"
#include "engine/pass/passinvokefunction.h"
#include "engine/pass/passsetrt.h"

#include "engine/pipeline/pipelinebase.h"
#include "engine/pipeline/pipelinedebug.h"
#include "engine/pipeline/pipelinedispatch.h"

#include "../pass/passpostprocess.h"
#include "../pass/passopaque.h"
#include "../pass/passtransparent.h"

#include "../pipeline/pipelinefpopaque.h"
#include "../pipeline/pipelinetransparent.h"

#include "engine/scene/sceneass.h"


const uint32_t AVERAGE_OVERLAPPING_LIGHTS_PER_TILE = 200u;

TechniqueForwardPlus::TechniqueForwardPlus(std::shared_ptr<RenderTarget> rt,
                                           std::shared_ptr<Texture> backBuffer)
    : base(rt, backBuffer)
{
}

TechniqueForwardPlus::~TechniqueForwardPlus() {}

void TechniqueForwardPlus::UpdateGridFrustums(std::shared_ptr<Camera> pCamera)
{
    // Make sure we can create at least 1 thread (even if the window is minimized)
    uint32_t screenWidth = std::max(App::s_desc.Width, 1u);
    uint32_t screenHeight = std::max(App::s_desc.Height, 1u);

    ScreenToViewParams screenToViewParams;
    screenToViewParams.m_InverseProjectionMatrix = pCamera->GetProjectionMatrix().Inverse();
    screenToViewParams.m_ScreenDimensions =
        Diligent::float2((float)screenWidth, (float)screenHeight);

    m_pScreenToViewParamsConstantBuffer->Set(screenToViewParams);

    // To compute the frustums for the grid tiles, each thread will compute a single
    // frustum for the tile.
    Diligent::uint3 numThreads =
        (Diligent::uint3((uint32_t)ceil(screenWidth / (float)m_LightCullingBlockSize),
                         (uint32_t)ceil(screenHeight / (float)m_LightCullingBlockSize), 1));
    Diligent::uint3 numThreadGroups =
        Diligent::uint3((uint32_t)ceil(numThreads.x / (float)m_LightCullingBlockSize),
                        (uint32_t)ceil(numThreads.y / (float)m_LightCullingBlockSize), 1);

    m_pLightIndexListOpaque = std::make_shared<StructuredBuffer>(
        nullptr, numThreads.x * numThreads.y * numThreads.z * AVERAGE_OVERLAPPING_LIGHTS_PER_TILE,
        (uint32_t)sizeof(uint32_t), CPUAccess::None, true);
    m_pLightIndexListTransparent = std::make_shared<StructuredBuffer>(
        nullptr, numThreads.x * numThreads.y * numThreads.z * AVERAGE_OVERLAPPING_LIGHTS_PER_TILE,
        (uint32_t)sizeof(uint32_t), CPUAccess::None, true);

    // Update the number of thread groups for the compute frustums compute shader.
    DispatchParams dispatchParams;
    dispatchParams.m_NumThreadGroups = numThreadGroups;
    dispatchParams.m_NumThreads = numThreads;
    m_pDispatchParamsConstantBuffer->Set(dispatchParams);

    // Create a new RWStructuredBuffer for storing the grid frustums.
    // We need 1 frustum for each grid cell.
    // For 1280x720 screen resolution and 16x16 tile size, results in 80x45 grid
    // for a total of 3,600 frustums.
    m_pGridFrustums =
        std::make_shared<StructuredBuffer>(nullptr, numThreads.x * numThreads.y * numThreads.z,
                                           (uint32_t)sizeof(Frustum), CPUAccess::None, true);

    // Dispatch the compute shader to recompute the grid frustums.
    m_pComputeFrustumsComputeShader->GetShaderParameterByName("DispatchParams")
        .Set(m_pDispatchParamsConstantBuffer);
    m_pComputeFrustumsComputeShader->GetShaderParameterByName(PassRender::kScreenToViewParams)
        .Set(m_pScreenToViewParamsConstantBuffer);
    m_pComputeFrustumsComputeShader->GetShaderParameterByName("out_Frustums").Set(m_pGridFrustums);

    std::shared_ptr<PipelineDispatch> dispatchPipeline =
        std::make_shared<PipelineDispatch>(numThreadGroups);
    dispatchPipeline->SetShader(Shader::ComputeShader, m_pComputeFrustumsComputeShader);

    std::shared_ptr<PassDispatch> frustumComputeDispatchPass =
        std::make_shared<PassDispatch>(this, dispatchPipeline);

    frustumComputeDispatchPass->Dispatch();

    // Update the light culling compute shader with the computed grid frustums StructuredBuffer.
    m_pLightCullingComputeShader->GetShaderParameterByName("in_Frustums").Set(m_pGridFrustums);
}

std::shared_ptr<Texture> TechniqueForwardPlus::LoadTexture(const std::string& path)
{
    TextureLoadInfo loadInfo;
    loadInfo.IsSRGB = false;
    RefCntAutoPtr<ITexture> Tex;
    // CreateTextureFromFile("DGLogo.png", loadInfo, App::s_device, &Tex);
    CreateTextureFromFile(path.c_str(), loadInfo, App::s_device, &Tex);

    std::shared_ptr<Texture> ret = std::make_shared<Texture>(Tex);

    return ret;
}


void TechniqueForwardPlus::init(const std::shared_ptr<Scene> scene, std::vector<Light>* lights,
                                std::shared_ptr<Camera> pCamera)
{
    //
    std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(this, m_pRenderTarget);
    AddPass(pSetRTPass);

    std::shared_ptr<PassClearRT> pClearRTPass =
        std::make_shared<PassClearRT>(this, m_pRenderTarget);
    AddPass(pClearRTPass);

    uint32_t numLights = (uint32_t)lights->size();

    Diligent::ShaderMacroHelper shaderMacros;
    shaderMacros.AddShaderMacro("NUM_LIGHTS", numLights);
    shaderMacros.AddShaderMacro("BLOCK_SIZE", m_LightCullingBlockSize);

    bool bRightHanded = false;
    shaderMacros.AddShaderMacro("RIGHT_HANDED", bRightHanded);

    m_pVertexShader = std::make_shared<Shader>();
    m_pVertexShader->LoadShaderFromFile(Shader::VertexShader, "ForwardRendering.hlsl", "VS_main",
                                        "./resources/shaders", false, shaderMacros);

    m_pForwardPlusPixelShader = std::make_shared<Shader>();
    m_pForwardPlusPixelShader->LoadShaderFromFile(Shader::Shader::PixelShader,
                                                  "ForwardPlusRendering.hlsl", "PS_main",
                                                  "./resources/shaders", false, shaderMacros);

    // Will be mapped to the "DispatchParams" in the Forward+ compute shaders.
    m_pDispatchParamsConstantBuffer =
        std::make_shared<ConstantBuffer>((uint32_t)sizeof(DispatchParams));
    // Will be mapped to the "ScreenToViewParams" in the CommonInclude.hlsl shader.
    m_pScreenToViewParamsConstantBuffer =
        std::make_shared<ConstantBuffer>((uint32_t)sizeof(ScreenToViewParams));

    // Light culling pass

    // Light list index counter (initial value buffer - required to reset the light list index
    // counters back to 0)
    uint32_t lightListIndexCounterInitialValue = 0;
    std::shared_ptr<StructuredBuffer> lightListIndexCounterInitialBuffer =
        std::make_shared<StructuredBuffer>(&lightListIndexCounterInitialValue, 1,
                                           (uint32_t)sizeof(uint32_t));

    // This one will be used as a RWStructuredBuffer in the compute shader.
    m_pLightListIndexCounterOpaque = std::make_shared<StructuredBuffer>(
        &lightListIndexCounterInitialValue, 1, (uint32_t)sizeof(uint32_t), CPUAccess::None, true);
    m_pLightListIndexCounterTransparent = std::make_shared<StructuredBuffer>(
        &lightListIndexCounterInitialValue, 1, (uint32_t)sizeof(uint32_t), CPUAccess::None, true);

    m_pLightCullingComputeShader = std::make_shared<Shader>();
    m_pLightCullingComputeShader->LoadShaderFromFile(Shader::ComputeShader,
                                                     "ForwardPlusRendering.hlsl", "CS_main",
                                                     "./resources/shaders", false, shaderMacros);

    m_pComputeFrustumsComputeShader = std::make_shared<Shader>();
    m_pComputeFrustumsComputeShader->LoadShaderFromFile(
        Shader::ComputeShader, "ForwardPlusRendering.hlsl", "CS_ComputeFrustums",
        "./resources/shaders", false, shaderMacros);

    SamplerDesc linearRepeatSampler{ FILTER_TYPE_LINEAR,   FILTER_TYPE_LINEAR,
                                     FILTER_TYPE_LINEAR,   TEXTURE_ADDRESS_WRAP,
                                     TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP };
    SamplerDesc linearClampSampler{ FILTER_TYPE_LINEAR,    FILTER_TYPE_LINEAR,
                                    FILTER_TYPE_LINEAR,    TEXTURE_ADDRESS_CLAMP,
                                    TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP };

    StaticSamplerDesc m_LinearRepeatSamplerDesc{ SHADER_TYPE_PIXEL, "LinearRepeatSampler",
                                                 linearRepeatSampler };
    StaticSamplerDesc m_LinearClampSamplerDesc{ SHADER_TYPE_PIXEL | SHADER_TYPE_COMPUTE,
                                                "LinearClampSampler", linearClampSampler };

    m_LinearRepeatSampler = std::make_shared<SamplerState>(m_LinearRepeatSamplerDesc);
    m_LinearClampSampler = std::make_shared<SamplerState>(m_LinearClampSamplerDesc);

    m_pLightCullingComputeShader->GetShaderParameterByName("LinearClampSampler")
        .Set(m_LinearClampSampler);
    m_pForwardPlusPixelShader->GetShaderParameterByName("LinearRepeatSampler")
        .Set(m_LinearRepeatSampler);

    auto numThreadGroups =
        Diligent::uint3((uint32_t)ceil(App::s_desc.Width / (float)m_LightCullingBlockSize),
                        (uint32_t)ceil(App::s_desc.Height / (float)m_LightCullingBlockSize), 1);

    auto lightGridFormat = Diligent::TEX_FORMAT_RG32_UINT;
    m_pLightGridOpaque =
        Scene::CreateTexture2D((uint16_t)numThreadGroups.x, (uint16_t)numThreadGroups.y,
                               (uint16_t)numThreadGroups.z, lightGridFormat, CPUAccess::None, true);
    m_pLightGridTransparent =
        Scene::CreateTexture2D((uint16_t)numThreadGroups.x, (uint16_t)numThreadGroups.y,
                               (uint16_t)numThreadGroups.z, lightGridFormat, CPUAccess::None, true);

    UpdateGridFrustums(pCamera);

    DispatchParams dispatchParams;
    dispatchParams.m_NumThreadGroups = numThreadGroups;
    dispatchParams.m_NumThreads =
        numThreadGroups * Diligent::uint3(m_LightCullingBlockSize, m_LightCullingBlockSize, 1);
    m_pDispatchParamsConstantBuffer->Set(dispatchParams);
    m_pLightCullingComputeShader->GetShaderParameterByName("DispatchParams")
        .Set(m_pDispatchParamsConstantBuffer);
    m_pLightCullingComputeShader->GetShaderParameterByName(PassRender::kScreenToViewParams)
        .Set(m_pScreenToViewParamsConstantBuffer);

    m_pLightCullingComputeShader->GetShaderParameterByName("o_LightGrid").Set(m_pLightGridOpaque);
    m_pLightCullingComputeShader->GetShaderParameterByName("t_LightGrid")
        .Set(m_pLightGridTransparent);

    m_pLightCullingComputeShader->GetShaderParameterByName("o_LightIndexList")
        .Set(m_pLightIndexListOpaque);
    m_pLightCullingComputeShader->GetShaderParameterByName("t_LightIndexList")
        .Set(m_pLightIndexListTransparent);

    std::shared_ptr<Texture> depthStencilBuffer =
        m_pRenderTarget->GetTexture(RenderTarget::AttachmentPoint::DepthStencil);
    m_pLightCullingComputeShader->GetShaderParameterByName("DepthTextureVS")
        .Set(depthStencilBuffer);
    m_pLightCullingComputeShader->GetShaderParameterByName("o_LightIndexCounter")
        .Set(m_pLightListIndexCounterOpaque);
    m_pLightCullingComputeShader->GetShaderParameterByName("t_LightIndexCounter")
        .Set(m_pLightListIndexCounterTransparent);

    m_pLightCullingDebugTexture =
        Scene::CreateTexture2D((uint16_t)App::s_desc.Width, (uint16_t)App::s_desc.Height, 1,
                               Diligent::TEX_FORMAT_RGBA32_FLOAT, CPUAccess::None, true);
    m_pLightCullingComputeShader->GetShaderParameterByName("DebugTexture")
        .Set(m_pLightCullingDebugTexture);

    // m_pLightCullingHeatMap = LoadTexture("./resources/textures/LightCountHeatMap.psd");
    m_pLightCullingHeatMap = LoadTexture("./resources/textures/LightCountHeatMap.png");
    m_pLightCullingComputeShader->GetShaderParameterByName("LightCountHeatMap")
        .Set(m_pLightCullingHeatMap);

    auto lightsSB = std::dynamic_pointer_cast<StructuredBuffer>(this->Get(PassRender::kLightsName));

    m_pLightCullingComputeShader->GetShaderParameterByName(PassRender::kLightsName).Set(lightsSB);

    //
    m_pDepthOnlyRenderTarget = std::make_shared<RenderTarget>();
    m_pDepthOnlyRenderTarget->AttachTexture(
        RenderTarget::AttachmentPoint::DepthStencil,
        m_pRenderTarget->GetTexture(RenderTarget::AttachmentPoint::DepthStencil));

    m_pDepthPrepassPipeline = std::make_shared<PipelineBase>(m_pDepthOnlyRenderTarget);

    m_pDepthPrepassPipeline->SetShader(Shader::VertexShader, m_pVertexShader);
    // m_pDepthPrepassPipeline->SetRenderTarget(m_pDepthOnlyRenderTarget);

    AddPass(std::make_shared<PassOpaque>(this, scene, m_pDepthPrepassPipeline, lights));

    AddPass(std::make_shared<PassCopyBuffer>(m_pLightListIndexCounterOpaque,
                                             lightListIndexCounterInitialBuffer));
    AddPass(std::make_shared<PassCopyBuffer>(m_pLightListIndexCounterTransparent,
                                             lightListIndexCounterInitialBuffer));

    std::shared_ptr<PipelineDispatch> dispatchPipeline =
        std::make_shared<PipelineDispatch>(numThreadGroups);
    dispatchPipeline->SetShader(Shader::ComputeShader, m_pLightCullingComputeShader);

    std::shared_ptr<PassDispatch> m_LightCullingDispatchPass =
        std::make_shared<PassDispatch>(this, dispatchPipeline);
    AddPass(m_LightCullingDispatchPass);

    //
    m_pForwardPlusOpaquePipeline = std::make_shared<PipelineFPOpaque>(m_pRenderTarget);

    m_pForwardPlusOpaquePipeline->SetShader(Shader::VertexShader, m_pVertexShader);
    m_pForwardPlusOpaquePipeline->SetShader(Shader::PixelShader, m_pForwardPlusPixelShader);
    // m_pForwardPlusOpaquePipeline->SetRenderTarget(m_pRenderTarget);

    m_pForwardPlusTransparentPipeline = std::make_shared<PipelineTransparent>(m_pRenderTarget);
    m_pForwardPlusTransparentPipeline->SetShader(Shader::VertexShader, m_pVertexShader);
    m_pForwardPlusTransparentPipeline->SetShader(Shader::PixelShader, m_pForwardPlusPixelShader);
    // m_pForwardPlusTransparentPipeline->SetRenderTarget(m_pRenderTarget);

    AddPass(std::make_shared<PassInvokeFunction>(this, [=]() {
        // Make sure the pixel shader has the right parameters set before executing the opaque pass.
        m_pForwardPlusPixelShader->GetShaderParameterByName("LightIndexList")
            .Set(m_pLightIndexListOpaque);
        m_pForwardPlusPixelShader->GetShaderParameterByName("LightGrid").Set(m_pLightGridOpaque);
    }));

    AddPass(std::make_shared<PassOpaque>(this, scene, m_pForwardPlusOpaquePipeline, lights));

    AddPass(std::make_shared<PassInvokeFunction>(this, [=]() {
        // Make sure the pixel shader has the right parameters set before executing the opaque pass.
        m_pForwardPlusPixelShader->GetShaderParameterByName("LightIndexList")
            .Set(m_pLightIndexListTransparent);
        m_pForwardPlusPixelShader->GetShaderParameterByName("LightGrid")
            .Set(m_pLightGridTransparent);
    }));

    AddPass(
        std::make_shared<PassTransparent>(this, scene, m_pForwardPlusTransparentPipeline, lights));

    initDebug();

    {
        auto srcTexture = m_pRenderTarget->GetTexture(RenderTarget::AttachmentPoint::Color0);
        auto dstTexture = m_pBackBuffer;

        std::shared_ptr<PassCopyTexture> pCopyTexPass =
            std::make_shared<PassCopyTexture>(this, dstTexture, srcTexture);
        AddPass(pCopyTexPass);
    }
}


void TechniqueForwardPlus::initDebug()
{
    bool IsGL = RIGHT_HANDED;

    Diligent::DepthStencilStateDesc DSStateDesc;
    DSStateDesc.DepthEnable = False;

	Diligent::BlendStateDesc BlendDesc;
    BlendDesc.RenderTargets[0].BlendEnable = True;
    BlendDesc.RenderTargets[0].SrcBlend = BLEND_FACTOR_SRC_ALPHA;
    BlendDesc.RenderTargets[0].DestBlend = BLEND_FACTOR_INV_SRC_ALPHA;
    BlendDesc.RenderTargets[0].SrcBlendAlpha = BLEND_FACTOR_ZERO;
    BlendDesc.RenderTargets[0].DestBlendAlpha = BLEND_FACTOR_ONE;

    Diligent::float4x4 orthographicProjection = Diligent::float4x4::Ortho(2.f, 2, 0.f, 1.f, IsGL);

    std::shared_ptr<Shader> pDebugTexturePixelShader1 = std::make_shared<Shader>();
    pDebugTexturePixelShader1->LoadShaderFromFile(Shader::PixelShader, "DeferredRendering.hlsl",
                                                  "PS_DebugTexture", "./resources/shaders");
    pDebugTexturePixelShader1->GetShaderParameterByName("LinearRepeatSampler")
        .Set(m_LinearRepeatSampler);

    m_pDebugTextureWithBlendingPipeline = std::make_shared<PipelineDebug>(m_pRenderTarget);
    m_pDebugTextureWithBlendingPipeline->SetShader(Shader::VertexShader, m_pVertexShader);
    m_pDebugTextureWithBlendingPipeline->SetShader(Shader::PixelShader, pDebugTexturePixelShader1);
    m_pDebugTextureWithBlendingPipeline->SetDepthStencilState(DSStateDesc);

    m_pDebugTextureWithBlendingPipeline->SetBlendState(BlendDesc);

    std::shared_ptr<Scene> debugTextureScene = SceneAss::CreateScreenQuad(-1, 1, -1, 1, 1);
    m_DebugTexture0Pass = std::make_shared<PassPostprocess>(
        this, debugTextureScene, m_pDebugTextureWithBlendingPipeline, orthographicProjection,
        m_pLightCullingDebugTexture);
    m_DebugTexture0Pass->SetEnabled(false);    // Initially disabled.
    AddPass(m_DebugTexture0Pass);
}

void TechniqueForwardPlus::Render()
{
    base::Render();
}

void TechniqueForwardPlus::Update()
{
    ImGui::Separator();

    ImGui::Checkbox("debug", &m_bDebugEnabled);
    ImGui::Separator();

    m_DebugTexture0Pass->SetEnabled(m_bDebugEnabled);
}


