#pragma once

#include "SampleBase.h"

#include "../engine.h"

using namespace Diligent;

// PerObject constant buffer data.
__declspec(align(16)) struct PerObject
{
	float4x4 ModelViewProjection;
	float4x4 ModelView;
};

struct RenderPassCreateInfo : public pgPassCreateInfo {
	//
	RenderPassCreateInfo() 
	{
		//
	}

	RenderPassCreateInfo(pgPassCreateInfo& ci)
		: pgPassCreateInfo(ci)
	{
	}

	Diligent::IBuffer*		PerObjectConstants;
	Diligent::IBuffer*		MaterialConstants;
	Diligent::IBuffer*		LightsStructuredBuffer;
	Diligent::IBufferView*	LightsBufferSRV;
};

class pgRenderPass : public pgPass {
	typedef pgPass base;

protected:
	void CreatePipelineState(const RenderPassCreateInfo& ci, PipelineStateDesc& PSODesc);
	void LoadTexture();

	RefCntAutoPtr<IPipelineState>         m_pPSO;
	RefCntAutoPtr<IShaderResourceBinding> m_pSRB;

	RefCntAutoPtr<IBuffer>                m_PerObjectConstants;
	RefCntAutoPtr<IBuffer>                m_MaterialConstants;

	RefCntAutoPtr<IBuffer>                m_LightsStructuredBuffer;

	Diligent::RefCntAutoPtr<Diligent::ITextureView>		m_TextureSRV;
public:
	pgRenderPass(const RenderPassCreateInfo& ci);

	virtual ~pgRenderPass();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag);
};

