#pragma once 

#include "SampleBase.h"

#include "../engine.h"
#include "../pass/passrender.h"
#include "../utils/light.h"

using namespace Diligent;

const uint16_t g_LightCullingBlockSize = 16;

// Constant buffer to store the number of groups executed in a dispatch.
__declspec(align(16)) struct DispatchParams
{
	Diligent::uint3 m_NumThreadGroups;
	uint32_t m_Padding0;        // Pad to 16 bytes.
	Diligent::uint3 m_NumThreads;
	uint32_t m_Padding1;        // Pad to 16 bytes.
};

__declspec(align(16)) struct ScreenToViewParams
{
	Diligent::float4x4 m_InverseProjectionMatrix;
	Diligent::float2 m_ScreenDimensions;
	Diligent::float2 padding;
};


__declspec(align(16)) struct Frustum
{
	Diligent::float4 planes[4];    // 64 Bytes
};


class TechniqueForwardPlus : public pgTechnique {
	typedef pgTechnique base;

	std::shared_ptr<Shader> g_pVertexShader;
	// For light culling in compute shader
	std::shared_ptr<Shader> g_pLightCullingComputeShader;
	// Compute the frustums for light culling.
	std::shared_ptr<Shader> g_pComputeFrustumsComputeShader;

	std::shared_ptr<Shader> g_pForwardPlusPixelShader;

	std::shared_ptr<pgPipeline> g_pForwardPlusOpaquePipeline;
	std::shared_ptr<pgPipeline> g_pForwardPlusTransparentPipeline;

	std::shared_ptr<ConstantBuffer> g_pDispatchParamsConstantBuffer;
	std::shared_ptr<ConstantBuffer> g_pScreenToViewParamsConstantBuffer;

	// Grid frustums for light culling.
	std::shared_ptr<StructuredBuffer> g_pGridFrustums;
	// The light index list stores the light indices per tile.
	// The light index list is produced by the light culling compute shader
	// and consumed by the forward+ pixel shader.
	std::shared_ptr<StructuredBuffer> g_pLightIndexListOpaque;
	std::shared_ptr<StructuredBuffer> g_pLightIndexListTransparent;

	// Keep track of the current index in the light list.
	std::shared_ptr<StructuredBuffer> g_pLightListIndexCounterOpaque;
	std::shared_ptr<StructuredBuffer> g_pLightListIndexCounterTransparent;
public:
	TechniqueForwardPlus(std::shared_ptr<pgRenderTarget> rt, std::shared_ptr<pgTexture> backBuffer);
	virtual ~TechniqueForwardPlus();

	void init(const pgPassRenderCreateInfo& ci, const std::vector<pgLight>& lights);

	virtual void update(pgRenderEventArgs& e);

	// Render the scene using the passes that have been configured.
	virtual void render(pgRenderEventArgs& e);

};