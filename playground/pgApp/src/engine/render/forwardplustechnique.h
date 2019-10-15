#pragma once 

#include "SampleBase.h"

#include "../engine.h"
#include "renderpass.h"
#include "light.h"

using namespace Diligent;

class ForwardPlusTechnique : public pgTechnique {
	typedef pgTechnique base;

public:
	ForwardPlusTechnique(const pgTechniqueCreateInfo& ci);
	virtual ~ForwardPlusTechnique();

	void init(const RenderPassCreateInfo& ci, const std::vector<pgLight>& lights);

	virtual void update(pgRenderEventArgs& e);

	// Render the scene using the passes that have been configured.
	virtual void render(pgRenderEventArgs& e);

};