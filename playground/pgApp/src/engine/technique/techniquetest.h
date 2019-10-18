#pragma once 

#include "SampleBase.h"

#include "engine/engine.h"

using namespace Diligent;

class TechniqueTest : public pgTechnique {
	typedef pgTechnique base;

	std::shared_ptr<ConstantBuffer>             m_VSConstants;

public:
	TechniqueTest(std::shared_ptr<pgRenderTarget> rt, std::shared_ptr<pgTexture> backBuffer);
	virtual ~TechniqueTest();

	virtual void update(pgRenderEventArgs& e);

	// Render the scene using the passes that have been configured.
	virtual void render(pgRenderEventArgs& e);
	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
	virtual void unbind(pgRenderEventArgs& e, pgBindFlag flag);

};