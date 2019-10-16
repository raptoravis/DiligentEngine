#pragma once

#include "SampleBase.h"

#include "../../engine.h"

#include "passrender.h"
#include "../light.h"

using namespace Diligent;

struct GeometryPassCreateInfo : public pgPassRenderCreateInfo {
	GeometryPassCreateInfo(const pgPassRenderCreateInfo& ci)
		: pgPassRenderCreateInfo(ci)
	{
	}

	std::shared_ptr<pgRenderTarget> rt;
};


class PassGeometry : public pgPassRender {
	typedef pgPassRender base;

protected:
	void CreatePipelineState(const pgPassRenderCreateInfo& ci, PipelineStateDesc& PSODesc);

	std::shared_ptr<pgRenderTarget> m_pGBufferRT;

public:
	PassGeometry(const GeometryPassCreateInfo& ci);

	virtual ~PassGeometry();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);

	virtual bool meshFilter(pgMesh* mesh);
};

