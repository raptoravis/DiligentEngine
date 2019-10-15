#pragma once 

#include "SampleBase.h"

#include "engine/engine.h"

using namespace Diligent;

class TestTechnique : public pgTechnique {
	typedef pgTechnique base;

public:
	TestTechnique(const pgTechniqueCreateInfo& ci);
	virtual ~TestTechnique();

	virtual void update(pgRenderEventArgs& e);

	// Render the scene using the passes that have been configured.
	virtual void render(pgRenderEventArgs& e);

};