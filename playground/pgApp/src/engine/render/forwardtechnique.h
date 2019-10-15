#pragma once 

#include "SampleBase.h"

#include "../engine.h"
#include "renderpass.h"
#include "light.h"

using namespace Diligent;

class ForwardTechnique : public pgTechnique {
	typedef pgTechnique base;

public:
	ForwardTechnique(const pgTechniqueCreateInfo& ci);
	virtual ~ForwardTechnique();

	void init(const RenderPassCreateInfo& ci, const std::vector<pgLight>& lights);

	virtual void update(pgRenderEventArgs& e);

	// Render the scene using the passes that have been configured.
	virtual void render(pgRenderEventArgs& e);

};