#pragma once 

#include "SampleBase.h"

#include "engine/engine.h"

using namespace Diligent;

class TechniqueTest : public pgTechnique {
	typedef pgTechnique base;

public:
	TechniqueTest(const pgTechniqueCreateInfo& ci);
	virtual ~TechniqueTest();

	virtual void update(pgRenderEventArgs& e);

	// Render the scene using the passes that have been configured.
	virtual void render(pgRenderEventArgs& e);
	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
	virtual void unbind(pgRenderEventArgs& e, pgBindFlag flag);

};