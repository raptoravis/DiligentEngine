#pragma once

#include "SampleBase.h"

#include "../engine.h"

using namespace Diligent;

class pgOpaquePass : public pgBasePass {
	typedef pgBasePass base;

	void CreatePipelineState();
	void LoadTexture();

	RefCntAutoPtr<IPipelineState>         m_pPSO;
	RefCntAutoPtr<IBuffer>                m_VSConstants;
	RefCntAutoPtr<IShaderResourceBinding> m_SRB;
	float4x4                              m_WorldViewProjMatrix;

	Diligent::RefCntAutoPtr<Diligent::ITextureView>           m_TextureSRV;

public:
	pgOpaquePass(const pgPassCreateInfo& ci);

	virtual ~pgOpaquePass();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void Update(pgRenderEventArgs& e);
	virtual void Render(pgRenderEventArgs& e);

};

