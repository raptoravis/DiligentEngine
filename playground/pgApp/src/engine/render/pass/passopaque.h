#pragma once

#include "SampleBase.h"

#include "../../engine.h"

#include "passrender.h"
#include "../light.h"

using namespace Diligent;

struct OpaquePassCreateInfo : public pgPassRenderCreateInfo {
	OpaquePassCreateInfo(const pgPassRenderCreateInfo& ci)
		: pgPassRenderCreateInfo(ci)
	{
	}
};


class PassOpaque : public pgPassRender {
	typedef pgPassRender base;

protected:
	//void CreatePipelineState(const pgPassRenderCreateInfo& ci);
public:
	PassOpaque(const OpaquePassCreateInfo& ci);

	virtual ~PassOpaque();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag);

	virtual bool meshFilter(pgMesh* mesh);
};

