#pragma once

#include "SampleBase.h"

#include "../engine.h"

#include "basepass.h"
#include "light.h"

using namespace Diligent;

class OpaquePass : public BasePass {
	typedef BasePass base;

protected:
	//void CreatePipelineState(const BasePassCreateInfo& ci);
public:
	OpaquePass(const BasePassCreateInfo& ci);

	virtual ~OpaquePass();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag);

	virtual bool meshFilter(pgMesh* mesh);
};

