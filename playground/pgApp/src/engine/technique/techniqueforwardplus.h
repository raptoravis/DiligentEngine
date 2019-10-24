#pragma once

#include "SampleBase.h"

#include "../engine.h"
#include "../pass/passrender.h"

using namespace Diligent;

const uint16_t m_LightCullingBlockSize = 16;

// Constant buffer to store the number of groups executed in a dispatch.
__declspec(align(16)) struct DispatchParams {
    Diligent::uint3 m_NumThreadGroups;
    uint32_t m_Padding0;    // Pad to 16 bytes.
    Diligent::uint3 m_NumThreads;
    uint32_t m_Padding1;    // Pad to 16 bytes.
};

__declspec(align(16)) struct ScreenToViewParams {
    Diligent::float4x4 m_InverseProjectionMatrix;
    Diligent::float2 m_ScreenDimensions;
    Diligent::float2 padding;
};


__declspec(align(16)) struct Frustum {
    Diligent::float4 planes[4];    // 64 Bytes
};


class TechniqueForwardPlus : public pgTechnique
{
    typedef pgTechnique base;

    std::shared_ptr<Shader> m_pVertexShader;
    // For light culling in compute shader
    std::shared_ptr<Shader> m_pLightCullingComputeShader;
    // Compute the frustums for light culling.
    std::shared_ptr<Shader> m_pComputeFrustumsComputeShader;
    std::shared_ptr<Shader> m_pForwardPlusPixelShader;

    std::shared_ptr<pgRenderTarget> m_pDepthOnlyRenderTarget;
    std::shared_ptr<pgRenderTarget> m_pColorOnlyRenderTarget;

    std::shared_ptr<pgPipeline> m_pDepthPrepassPipeline;
    std::shared_ptr<pgPipeline> m_pForwardPlusOpaquePipeline;
    std::shared_ptr<pgPipeline> m_pForwardPlusTransparentPipeline;

    std::shared_ptr<ConstantBuffer> m_pDispatchParamsConstantBuffer;
    std::shared_ptr<ConstantBuffer> m_pScreenToViewParamsConstantBuffer;

    // Grid frustums for light culling.
    std::shared_ptr<StructuredBuffer> m_pGridFrustums;
    // The light index list stores the light indices per tile.
    // The light index list is produced by the light culling compute shader
    // and consumed by the forward+ pixel shader.
    std::shared_ptr<StructuredBuffer> m_pLightIndexListOpaque;
    std::shared_ptr<StructuredBuffer> m_pLightIndexListTransparent;

    // Keep track of the current index in the light list.
    std::shared_ptr<StructuredBuffer> m_pLightListIndexCounterOpaque;
    std::shared_ptr<StructuredBuffer> m_pLightListIndexCounterTransparent;

    std::shared_ptr<pgTexture> m_pLightGridOpaque;
    std::shared_ptr<pgTexture> m_pLightGridTransparent;

    // For debugging of the light culling shader.
    std::shared_ptr<pgTexture> m_pLightCullingDebugTexture;
    // Heatmap texture for light culling debug.
    std::shared_ptr<pgTexture> m_pLightCullingHeatMap;

    std::shared_ptr<SamplerState> m_LinearRepeatSampler;
    std::shared_ptr<SamplerState> m_LinearClampSampler;


    void UpdateGridFrustums(std::shared_ptr<pgCamera> pCamera);
    std::shared_ptr<pgTexture> LoadTexture(const std::string& path);

  public:
    TechniqueForwardPlus(std::shared_ptr<pgRenderTarget> rt, std::shared_ptr<pgTexture> backBuffer);
    virtual ~TechniqueForwardPlus();

    void init(const std::shared_ptr<pgScene> scene, std::vector<pgLight>* lights,
              std::shared_ptr<pgCamera> pCamera);
};