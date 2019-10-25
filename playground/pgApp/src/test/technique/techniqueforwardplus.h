#pragma once

#include "SampleBase.h"

#include "engine/engine.h"

#include "test/pass/passrender.h"
#include "test/pass/passpostprocess.h"

using namespace Diligent;
using namespace ade;

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


class TechniqueForwardPlus : public Technique
{
    typedef Technique base;

    std::shared_ptr<Shader> m_pVertexShader;
    // For light culling in compute shader
    std::shared_ptr<Shader> m_pLightCullingComputeShader;
    // Compute the frustums for light culling.
    std::shared_ptr<Shader> m_pComputeFrustumsComputeShader;
    std::shared_ptr<Shader> m_pForwardPlusPixelShader;

    std::shared_ptr<RenderTarget> m_pDepthOnlyRenderTarget;
    std::shared_ptr<RenderTarget> m_pColorOnlyRenderTarget;

	std::shared_ptr<Pipeline> m_pDebugTextureWithBlendingPipeline;
    std::shared_ptr<Pipeline> m_pDepthPrepassPipeline;
    std::shared_ptr<Pipeline> m_pForwardPlusOpaquePipeline;
    std::shared_ptr<Pipeline> m_pForwardPlusTransparentPipeline;

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

    std::shared_ptr<Texture> m_pLightGridOpaque;
    std::shared_ptr<Texture> m_pLightGridTransparent;

    // For debugging of the light culling shader.
    std::shared_ptr<Texture> m_pLightCullingDebugTexture;
    // Heatmap texture for light culling debug.
    std::shared_ptr<Texture> m_pLightCullingHeatMap;

    std::shared_ptr<SamplerState> m_LinearRepeatSampler;
    std::shared_ptr<SamplerState> m_LinearClampSampler;

	std::shared_ptr<PassPostprocess> m_DebugTexture0Pass;
    bool m_bDebugEnabled = false;

    void UpdateGridFrustums(std::shared_ptr<Camera> pCamera);
    std::shared_ptr<Texture> LoadTexture(const std::string& path);

	void initDebug();
  public:
    TechniqueForwardPlus(std::shared_ptr<RenderTarget> rt, std::shared_ptr<Texture> backBuffer);
    virtual ~TechniqueForwardPlus();

    virtual void Render();
    virtual void Update();

    void init(const std::shared_ptr<Scene> scene, std::vector<Light>* lights,
              std::shared_ptr<Camera> pCamera);
};

