#pragma once

#include "SampleBase.h"

#include "../../engine.h"

#include "passrender.h"
#include "../light.h"

using namespace Diligent;

struct LightPassCreateInfo : public RenderPassCreateInfo {
	//
	LightPassCreateInfo() {
		//
	}

	LightPassCreateInfo(const RenderPassCreateInfo& ci)
		: RenderPassCreateInfo(ci)
	{
	}

	ITextureView*           ColorRTV;
	ITextureView*           DSSRV;

	ITextureView*           DiffuseSRV;
	ITextureView*           SpecularSRV;
	ITextureView*           NormalSRV;

	const std::vector<pgLight>*	Lights;

};

__declspec(align(16)) struct ScreenToViewParams
{
	float4x4 m_InverseProjectionMatrix;
	float2 m_ScreenDimensions;
	float2 padding;
};

__declspec(align(16)) struct LightParams
{
	uint32_t m_LightIndex;
	uint32_t padding[3];
};

class LightPass : public pgRenderPass {
	typedef pgRenderPass base;

protected:
	void CreatePipelineState(const RenderPassCreateInfo& ci, PipelineStateDesc& PSODesc);

	RefCntAutoPtr<ITextureView>           m_pColorRTV;
	RefCntAutoPtr<ITextureView>           m_pDSSRV;

	RefCntAutoPtr<ITextureView>           m_pDiffuseSRV;
	RefCntAutoPtr<ITextureView>           m_pSpecularSRV;
	RefCntAutoPtr<ITextureView>           m_pNormalSRV;
	const std::vector<pgLight>*			  m_pLights;

	RefCntAutoPtr<IBuffer>				m_LightParamsCB;
	RefCntAutoPtr<IBuffer>				m_ScreenToViewParamsCB;

	void updateLightParams(pgRenderEventArgs& e, const LightParams& lightParam);
	void updateScreenToViewParams(pgRenderEventArgs& e);
public:
	LightPass(const LightPassCreateInfo& ci);

	virtual ~LightPass();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag);

	virtual bool meshFilter(pgMesh* mesh);
};

