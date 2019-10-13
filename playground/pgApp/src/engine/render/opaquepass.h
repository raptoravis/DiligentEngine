#pragma once

#include "SampleBase.h"

#include "../engine.h"

using namespace Diligent;

class pgOpaquePass : public pgBasePass {
	typedef pgBasePass base;

protected:
	void CreatePipelineState();
	void LoadTexture();

	RefCntAutoPtr<IPipelineState>         m_pPSO;
	RefCntAutoPtr<IBuffer>                m_PerObjectConstants;
	RefCntAutoPtr<IShaderResourceBinding> m_SRB;

	float4x4                              m_WorldViewMatrix;
	float4x4                              m_WorldViewProjMatrix;

	Diligent::RefCntAutoPtr<Diligent::ITextureView>           m_TextureSRV;

	// PerObject constant buffer data.
	__declspec(align(16)) struct PerObject
	{
		float4x4 ModelViewProjection;
		float4x4 ModelView;
	};
public:
	pgOpaquePass(const pgPassCreateInfo& ci);

	virtual ~pgOpaquePass();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void updateSRB(pgRenderEventArgs& e);
};

