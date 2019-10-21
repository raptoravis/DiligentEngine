#include "TechniqueForwardPlus.h"

#include "../pass/passclearrt.h"
#include "../pass/passcopybuffer.h"
#include "../pass/passcopytexture.h"
#include "../pass/passdispatch.h"
#include "../pass/passinvokefunction.h"
#include "../pass/passopaque.h"
#include "../pass/passsetrt.h"
#include "../pass/passtransparent.h"

#include "../pipeline/pipelinebase.h"
#include "../pipeline/pipelinetransparent.h"

TechniqueForwardPlus::TechniqueForwardPlus(std::shared_ptr<pgRenderTarget> rt,
                                           std::shared_ptr<pgTexture> backBuffer)
    : base(rt, backBuffer)
{
}

TechniqueForwardPlus::~TechniqueForwardPlus() {}

void TechniqueForwardPlus::UpdateGridFrustums()
{
    // Make sure we can create at least 1 thread (even if the window is minimized)
    uint32_t screenWidth = std::max(pgApp::s_desc.Width, 1u);
    uint32_t screenHeight = std::max(pgApp::s_desc.Height, 1u);

    // To compute the frustums for the grid tiles, each thread will compute a single
    // frustum for the tile.
    Diligent::uint3 numThreads =
        (Diligent::uint3((uint32_t)ceil(screenWidth / (float)g_LightCullingBlockSize),
                         (uint32_t)ceil(screenHeight / (float)g_LightCullingBlockSize), 1));
    Diligent::uint3 numThreadGroups =
        Diligent::uint3((uint32_t)ceil(numThreads.x / (float)g_LightCullingBlockSize),
                        (uint32_t)ceil(numThreads.y / (float)g_LightCullingBlockSize), 1);

    // Update the number of thread groups for the compute frustums compute shader.
    DispatchParams dispatchParams;
    dispatchParams.m_NumThreadGroups = numThreadGroups;
    dispatchParams.m_NumThreads = numThreads;
    g_pDispatchParamsConstantBuffer->Set(dispatchParams);

    // Create a new RWStructuredBuffer for storing the grid frustums.
    // We need 1 frustum for each grid cell.
    // For 1280x720 screen resolution and 16x16 tile size, results in 80x45 grid
    // for a total of 3,600 frustums.
    g_pGridFrustums =
        std::make_shared<StructuredBuffer>(nullptr, numThreads.x * numThreads.y * numThreads.z,
                                           (uint32_t)sizeof(Frustum), CPUAccess::None, true);

    // Dispatch the compute shader to recompute the grid frustums.
    g_pComputeFrustumsComputeShader->GetShaderParameterByName("DispatchParams")
        .Set(g_pDispatchParamsConstantBuffer);
    g_pComputeFrustumsComputeShader->GetShaderParameterByName("ScreenToViewParams")
        .Set(g_pScreenToViewParamsConstantBuffer);
    g_pComputeFrustumsComputeShader->GetShaderParameterByName("out_Frustums").Set(g_pGridFrustums);

    g_pComputeFrustumsComputeShader->Bind();
    g_pComputeFrustumsComputeShader->Dispatch(numThreadGroups);
    g_pComputeFrustumsComputeShader->UnBind();

    // Update the light culling compute shader with the computed grid frustums StructuredBuffer.
    g_pLightCullingComputeShader->GetShaderParameterByName("in_Frustums").Set(g_pGridFrustums);
}


void TechniqueForwardPlus::init(const std::shared_ptr<pgScene> scene,
                                std::vector<pgLight>* lights)
{
    //
    std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(this, m_pRT);
    addPass(pSetRTPass);

    std::shared_ptr<PassClearRT> pClearRTPass = std::make_shared<PassClearRT>(this, m_pRT);
    addPass(pClearRTPass);

    g_pVertexShader = std::make_shared<Shader>();
    g_pVertexShader->LoadShaderFromFile(Shader::VertexShader, "ForwardRendering.hlsl", "VS_main",
                                        "./resources/shaders");

    g_pForwardPlusPixelShader = std::make_shared<Shader>();
    g_pForwardPlusPixelShader->LoadShaderFromFile(Shader::PixelShader, "ForwardRendering.hlsl",
                                                  "PS_main", "./resources/shaders");
    // g_pForwardPlusPixelShader->LoadShaderFromFile(
    //    Shader::Shader::PixelShader, "ForwardPlusRendering.hlsl", "PS_main",
    //    "./resources/shaders");
#if 0
    // Will be mapped to the "DispatchParams" in the Forward+ compute shaders.
    g_pDispatchParamsConstantBuffer =
        std::make_shared<ConstantBuffer>((uint32_t)sizeof(DispatchParams));
    // Will be mapped to the "ScreenToViewParams" in the CommonInclude.hlsl shader.
    g_pScreenToViewParamsConstantBuffer =
        std::make_shared<ConstantBuffer>((uint32_t)sizeof(ScreenToViewParams));

    // Light culling pass

    // Light list index counter (initial value buffer - required to reset the light list index
    // counters back to 0)
    uint32_t lightListIndexCounterInitialValue = 0;
    std::shared_ptr<StructuredBuffer> lightListIndexCounterInitialBuffer =
        std::make_shared<StructuredBuffer>(&lightListIndexCounterInitialValue, 1,
                                           (uint32_t)sizeof(uint32_t));

    // This one will be used as a RWStructuredBuffer in the compute shader.
    g_pLightListIndexCounterOpaque = std::make_shared<StructuredBuffer>(
        &lightListIndexCounterInitialValue, 1, (uint32_t)sizeof(uint32_t), CPUAccess::None, true);
    g_pLightListIndexCounterTransparent = std::make_shared<StructuredBuffer>(
        &lightListIndexCounterInitialValue, 1, (uint32_t)sizeof(uint32_t), CPUAccess::None, true);

    g_pLightCullingComputeShader = std::make_shared<Shader>();
    g_pLightCullingComputeShader->LoadShaderFromFile(
        Shader::ComputeShader, "ForwardPlusRendering.hlsl", "CS_main", "./resources/shaders");

    g_pComputeFrustumsComputeShader = std::make_shared<Shader>();
    g_pComputeFrustumsComputeShader->LoadShaderFromFile(
        Shader::ComputeShader, "ForwardPlusRendering.hlsl", "CS_ComputeFrustums",
        "./resources/shaders");

    auto numThreadGroups =
        Diligent::uint3((uint32_t)ceil(pgApp::s_desc.Width / (float)g_LightCullingBlockSize),
                        (uint32_t)ceil(pgApp::s_desc.Height / (float)g_LightCullingBlockSize), 1);

	auto lightGridFormat = Diligent::TEX_FORMAT_RG32_UINT;
    g_pLightGridOpaque = pgScene::CreateTexture2D(
        (uint16_t)numThreadGroups.x, (uint16_t)numThreadGroups.y, (uint16_t)numThreadGroups.z,
        lightGridFormat, CPUAccess::None, true);
    g_pLightGridTransparent = pgScene::CreateTexture2D(
        (uint16_t)numThreadGroups.x, (uint16_t)numThreadGroups.y, (uint16_t)numThreadGroups.z,
        lightGridFormat, CPUAccess::None, true);

    g_pDispatchParamsConstantBuffer = std::make_shared<ConstantBuffer>((uint32_t)sizeof(DispatchParams));

	// Will be mapped to the "ScreenToViewParams" in the CommonInclude.hlsl shader.
    g_pScreenToViewParamsConstantBuffer = std::make_shared<ConstantBuffer>((uint32_t)sizeof(ScreenToViewParams));

	UpdateGridFrustums();

    DispatchParams dispatchParams;
    dispatchParams.m_NumThreadGroups = numThreadGroups;
    dispatchParams.m_NumThreads =
        numThreadGroups * Diligent::uint3(g_LightCullingBlockSize, g_LightCullingBlockSize, 1);
    g_pDispatchParamsConstantBuffer->Set(dispatchParams);
    g_pLightCullingComputeShader->GetShaderParameterByName("DispatchParams")
        .Set(g_pDispatchParamsConstantBuffer);

    g_pLightCullingComputeShader->GetShaderParameterByName("o_LightGrid")
        .Set(g_pLightGridOpaque);
    g_pLightCullingComputeShader->GetShaderParameterByName("t_LightGrid")
        .Set(g_pLightGridTransparent);

    g_pLightCullingComputeShader->GetShaderParameterByName("o_LightIndexList")
        .Set(g_pLightIndexListOpaque);
    g_pLightCullingComputeShader->GetShaderParameterByName("t_LightIndexList")
        .Set(g_pLightIndexListTransparent);

    std::shared_ptr<pgTexture> depthStencilBuffer =
        m_pRT->GetTexture(pgRenderTarget::AttachmentPoint::DepthStencil);
    g_pLightCullingComputeShader->GetShaderParameterByName("DepthTextureVS")
        .Set(depthStencilBuffer);
    g_pLightCullingComputeShader->GetShaderParameterByName("o_LightIndexCounter")
        .Set(g_pLightListIndexCounterOpaque);
    g_pLightCullingComputeShader->GetShaderParameterByName("t_LightIndexCounter")
        .Set(g_pLightListIndexCounterTransparent);

    //
    g_pDepthPrepassPipeline = std::make_shared<pgPipeline>(g_pDepthOnlyRenderTarget);

    g_pDepthPrepassPipeline->SetShader(Shader::VertexShader, g_pVertexShader);
    g_pDepthPrepassPipeline->SetRenderTarget(g_pDepthOnlyRenderTarget);

    g_pDepthOnlyRenderTarget = std::make_shared<pgRenderTarget>();
    g_pDepthOnlyRenderTarget->AttachTexture(
        pgRenderTarget::AttachmentPoint::DepthStencil,
        m_pRT->GetTexture(pgRenderTarget::AttachmentPoint::DepthStencil));

    addPass(std::make_shared<PassOpaque>(this, scene, g_pDepthPrepassPipeline, lights));

    addPass(std::make_shared<PassCopyBuffer>(g_pLightListIndexCounterOpaque,
                                             lightListIndexCounterInitialBuffer));
    addPass(std::make_shared<PassCopyBuffer>(g_pLightListIndexCounterTransparent,
                                             lightListIndexCounterInitialBuffer));

    std::shared_ptr<PassDispatch> g_LightCullingDispatchPass =
        std::make_shared<PassDispatch>(this, g_pLightCullingComputeShader, numThreadGroups);
    addPass(g_LightCullingDispatchPass);

    addPass(std::make_shared<PassInvokeFunction>(this, [=]() {
        // Make sure the pixel shader has the right parameters set before executing the opaque pass.
        g_pForwardPlusPixelShader->GetShaderParameterByName("LightIndexList")
            .Set(g_pLightIndexListOpaque);
        g_pForwardPlusPixelShader->GetShaderParameterByName("LightGrid")
            .Set(g_pLightGridOpaque);
    }));
#endif
    //
    g_pForwardPlusOpaquePipeline = std::make_shared<PipelineBase>(m_pRT);

    g_pForwardPlusOpaquePipeline->SetShader(Shader::VertexShader, g_pVertexShader);
    g_pForwardPlusOpaquePipeline->SetShader(Shader::PixelShader, g_pForwardPlusPixelShader);
    g_pForwardPlusOpaquePipeline->SetRenderTarget(m_pRT);

    g_pForwardPlusTransparentPipeline = std::make_shared<PipelineTransparent>(m_pRT);
    g_pForwardPlusTransparentPipeline->SetShader(Shader::VertexShader, g_pVertexShader);
    g_pForwardPlusTransparentPipeline->SetShader(Shader::PixelShader, g_pForwardPlusPixelShader);
    g_pForwardPlusTransparentPipeline->SetRenderTarget(m_pRT);

    addPass(std::make_shared<PassOpaque>(this, scene, g_pForwardPlusOpaquePipeline, lights));

    addPass(
        std::make_shared<PassTransparent>(this, scene, g_pForwardPlusTransparentPipeline, lights));

    {
        auto srcTexture = m_pRT->GetTexture(pgRenderTarget::AttachmentPoint::Color0);
        auto dstTexture = m_pBackBuffer;

        std::shared_ptr<PassCopyTexture> pCopyTexPass =
            std::make_shared<PassCopyTexture>(this, dstTexture, srcTexture);
        addPass(pCopyTexPass);
    }
}
