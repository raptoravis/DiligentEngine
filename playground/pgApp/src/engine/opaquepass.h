#pragma once

#include "SampleBase.h"

#include "engine.h"

using namespace Diligent;

class pgOpaquePass : public pgBasePass {
	typedef pgBasePass base;

	void CreatePipelineState();
	void CreateVertexBuffer();
	void CreateIndexBuffer();

	RefCntAutoPtr<IPipelineState>         m_pPSO;
	RefCntAutoPtr<IShaderResourceBinding> m_pSRB;
	RefCntAutoPtr<IBuffer>                m_CubeVertexBuffer;
	RefCntAutoPtr<IBuffer>                m_CubeIndexBuffer;
	RefCntAutoPtr<IBuffer>                m_VSConstants;
	float4x4                              m_WorldViewProjMatrix;

public:
	pgOpaquePass(const pgPassCreateInfo& ci);

	virtual ~pgOpaquePass();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void Update(RenderEventArgs& e);
	virtual void Render(RenderEventArgs& e);

};

