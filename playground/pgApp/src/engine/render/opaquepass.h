#pragma once

#include "SampleBase.h"

#include "../engine.h"

#include "basepass.h"
#include "light.h"

using namespace Diligent;

class OpaquePass : public BasePass {
	typedef BasePass base;

protected:
	void CreatePipelineState();
	void LoadTexture();

	RefCntAutoPtr<IPipelineState>         m_pPSO;

	RefCntAutoPtr<IBuffer>                m_PerObjectConstants;
	RefCntAutoPtr<IBuffer>                m_MaterialConstants;

	RefCntAutoPtr<IBuffer>                m_LightsStructuredBuffer;

	RefCntAutoPtr<IShaderResourceBinding> m_pSRB;

	float4x4                              m_WorldViewMatrix;
	float4x4                              m_WorldViewProjMatrix;

	Diligent::RefCntAutoPtr<Diligent::ITextureView>		m_TextureSRV;
	std::vector<pgLight>								m_Lights;
public:
	OpaquePass(const BasePassCreateInfo& ci);

	virtual ~OpaquePass();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void updateSRB(pgRenderEventArgs& e);

	virtual bool meshFilter(pgMesh* mesh);
};

