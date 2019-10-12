#pragma once

#include "SampleBase.h"

#include "engine.h"

using namespace Diligent;

class pgCubeTexPass : public pgBasePass {
	typedef pgBasePass base;

	const TEXTURE_FORMAT		m_backbufferFormat;
	const TEXTURE_FORMAT		m_depthFormat;
	const int					width;
	const int					height;

	void CreatePipelineState();
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	void LoadTexture();

	RefCntAutoPtr<IPipelineState>         m_pPSO;
	RefCntAutoPtr<IBuffer>                m_CubeVertexBuffer;
	RefCntAutoPtr<IBuffer>                m_CubeIndexBuffer;
	RefCntAutoPtr<IBuffer>                m_VSConstants;
	RefCntAutoPtr<ITextureView>           m_TextureSRV;
	RefCntAutoPtr<IShaderResourceBinding> m_SRB;
	float4x4                              m_WorldViewProjMatrix;

public:
	pgCubeTexPass(Diligent::IRenderDevice* device, Diligent::IDeviceContext* pCtx, Diligent::IEngineFactory* factory, TEXTURE_FORMAT
		BackBufferFmt, TEXTURE_FORMAT DepthBufferFmt, 
		int w, int h);

	virtual ~pgCubeTexPass();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void Update(RenderEventArgs& e);
	virtual void Render(RenderEventArgs& e);
	virtual void UpdateUI(RenderEventArgs& e);

};

