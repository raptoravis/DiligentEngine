#pragma once

#include "SampleBase.h"

#include "../engine.h"

#include "passrender.h"
#include "../utils/light.h"

using namespace Diligent;

class PassTransparent : public pgPassRender {
	typedef pgPassRender base;

public:
    PassTransparent(std::shared_ptr<pgScene> scene, std::shared_ptr<pgPipeline> pipeline,
               const std::vector<pgLight>& lights);

	virtual ~PassTransparent();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
	virtual void unbind(pgRenderEventArgs& e, pgBindFlag flag);

	virtual bool meshFilter(pgMesh* mesh);

	virtual void Visit(pgMesh& mesh);
};

