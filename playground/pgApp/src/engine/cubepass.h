#pragma once

#include "SampleBase.h"

#include "engine.h"

using namespace Diligent;

class pgCubePass : public pgBasePass {
	typedef pgBasePass base;

	const TEXTURE_FORMAT		m_backbufferFormat;
	const TEXTURE_FORMAT		m_depthFormat;
	const int					width;
	const int					height;

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
	pgCubePass(Diligent::IRenderDevice* device, Diligent::IDeviceContext* pCtx, Diligent::IEngineFactory* factory, TEXTURE_FORMAT
		BackBufferFmt, TEXTURE_FORMAT DepthBufferFmt, 
		int w, int h);

	virtual ~pgCubePass();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void Update(float CurrTime, float ElapsedTime);
	virtual void Render(Camera* pCamera);
	virtual void UpdateUI();

};

