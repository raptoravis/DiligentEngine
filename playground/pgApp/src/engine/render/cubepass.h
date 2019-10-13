#pragma once

#include "SampleBase.h"

#include "../engine.h"

using namespace Diligent;

class pgCubePass : public pgBasePass {
	typedef pgBasePass base;

	void CreatePipelineState();

	RefCntAutoPtr<IPipelineState>         m_pPSO;
	RefCntAutoPtr<IShaderResourceBinding> m_pSRB;
	RefCntAutoPtr<IBuffer>                m_VSConstants;

public:
	pgCubePass(const pgPassCreateInfo& ci);
	virtual ~pgCubePass();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void updateSRB(pgSceneNode* sceneNode, pgRenderEventArgs& e);
};

