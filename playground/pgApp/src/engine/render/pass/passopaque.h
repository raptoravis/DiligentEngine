#pragma once

#include "SampleBase.h"

#include "../../engine.h"

#include "passrender.h"
#include "../light.h"

using namespace Diligent;

struct PassOpaqueCreateInfo : public pgPassRenderCreateInfo {
	PassOpaqueCreateInfo(const pgPassRenderCreateInfo& ci)
		: pgPassRenderCreateInfo(ci)
	{
	}
};


class PassOpaque : public pgPassRender {
	typedef pgPassRender base;

protected:
	//void CreatePipelineState(const pgPassRenderCreateInfo& ci);
public:
	PassOpaque(const PassOpaqueCreateInfo& ci);

	virtual ~PassOpaque();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
	virtual void unbind(pgRenderEventArgs& e, pgBindFlag flag);

	virtual bool meshFilter(pgMesh* mesh);
};

