#pragma once 

#include "SampleBase.h"

#include "../engine.h"
#include "pass/passrender.h"
#include "light.h"

using namespace Diligent;

class TechniqueForwardPlus : public pgTechnique {
	typedef pgTechnique base;

public:
	TechniqueForwardPlus(std::shared_ptr<pgRenderTarget> rt, std::shared_ptr<pgTexture> backBuffer);
	virtual ~TechniqueForwardPlus();

	void init(const pgPassRenderCreateInfo& ci, const std::vector<pgLight>& lights);

	virtual void update(pgRenderEventArgs& e);

	// Render the scene using the passes that have been configured.
	virtual void render(pgRenderEventArgs& e);

};