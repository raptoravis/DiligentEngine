#pragma once

#include "SampleBase.h"

#include "../../engine.h"

using namespace Diligent;

// PerObject constant buffer data.
__declspec(align(16)) struct PerObject
{
	float4x4 ModelViewProjection;
	float4x4 ModelView;
};

struct pgPassRenderCreateInfo {
	std::shared_ptr<pgScene>  scene;
	IBuffer*	              PerObjectConstants;
	IBuffer*		          MaterialConstants;
	IBuffer*			      LightsStructuredBuffer;
	IBufferView*			  LightsBufferSRV;
};

class pgPassRender : public pgPass {
	typedef pgPass base;

protected:
	void CreatePipelineState(PipelineStateDesc& PSODesc);
	void LoadTexture();

	RefCntAutoPtr<IPipelineState>         m_pPSO;
	RefCntAutoPtr<IShaderResourceBinding> m_pSRB;

	RefCntAutoPtr<IBuffer>                m_PerObjectConstants;
	RefCntAutoPtr<IBuffer>                m_MaterialConstants;

	RefCntAutoPtr<IBuffer>                m_LightsStructuredBuffer;
	RefCntAutoPtr<IBufferView>			  m_LightsBufferSRV;

	Diligent::RefCntAutoPtr<Diligent::ITextureView>		m_TextureSRV;
public:
	pgPassRender(const pgPassRenderCreateInfo& ci);

	virtual ~pgPassRender();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
	virtual void unbind(pgRenderEventArgs& e, pgBindFlag flag);
};

