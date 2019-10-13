#pragma once

#include "SampleBase.h"

#include "../engine.h"

using namespace Diligent;

class pgCubeTexPass : public pgBasePass {
	typedef pgBasePass base;

	void CreatePipelineState();
	void LoadTexture();

	RefCntAutoPtr<IPipelineState>         m_pPSO;
	RefCntAutoPtr<IBuffer>                m_VSConstants;
	RefCntAutoPtr<IShaderResourceBinding> m_pSRB;

	Diligent::RefCntAutoPtr<Diligent::ITextureView>           m_TextureSRV;

public:
	pgCubeTexPass(const pgPassCreateInfo& ci);

	virtual ~pgCubeTexPass();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void updateSRB(pgSceneNode* sceneNode, pgRenderEventArgs& e);
};

