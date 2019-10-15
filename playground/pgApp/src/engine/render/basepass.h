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

struct BasePassCreateInfo : public pgPassCreateInfo {
	//
	BasePassCreateInfo() {
		//
	}

	BasePassCreateInfo(pgPassCreateInfo& ci)
		: pgPassCreateInfo(ci)
	{
	}

	Diligent::IBuffer*		PerObjectConstants;
	Diligent::IBuffer*		MaterialConstants;
	Diligent::IBuffer*		LightsStructuredBuffer;
	Diligent::IBufferView*	LightsBufferSRV;
};

class BasePass : public pgBasePass {
	typedef pgBasePass base;

protected:
	void CreatePipelineState(const BasePassCreateInfo& ci);
	void LoadTexture();

	RefCntAutoPtr<IPipelineState>			m_pPSO;

	RefCntAutoPtr<IShaderResourceBinding>	m_pSRB;

	Diligent::RefCntAutoPtr<Diligent::ITextureView>		m_TextureSRV;
public:
	BasePass(const BasePassCreateInfo& ci);

	virtual ~BasePass();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void updateSRB(pgRenderEventArgs& e);
};

