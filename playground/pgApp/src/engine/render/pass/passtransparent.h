#pragma once

#include "SampleBase.h"

#include "../../engine.h"

#include "passrender.h"
#include "../light.h"

using namespace Diligent;

class TransparentPass : public pgRenderPass {
	typedef pgRenderPass base;

protected:
	//void CreatePipelineState(const RenderPassCreateInfo& ci);
public:
	TransparentPass(const RenderPassCreateInfo& ci);

	virtual ~TransparentPass();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag);

	virtual bool meshFilter(pgMesh* mesh);
};

