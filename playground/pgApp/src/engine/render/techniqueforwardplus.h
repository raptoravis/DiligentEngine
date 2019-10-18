#pragma once 

#include "SampleBase.h"

#include "../engine.h"
#include "pass/passrender.h"
#include "light.h"

using namespace Diligent;

class TechniqueForwardPlus : public pgTechnique {
	typedef pgTechnique base;

	std::shared_ptr<Shader> g_pVertexShader;
	std::shared_ptr<Shader> g_pForwardPlusPixelShader;
	std::shared_ptr<pgPipeline> g_pForwardPlusOpaquePipeline;
	std::shared_ptr<pgPipeline> g_pForwardPlusTransparentPipeline;
public:
	TechniqueForwardPlus(std::shared_ptr<pgRenderTarget> rt, std::shared_ptr<pgTexture> backBuffer);
	virtual ~TechniqueForwardPlus();

	void init(const pgPassRenderCreateInfo& ci, const std::vector<pgLight>& lights);

	virtual void update(pgRenderEventArgs& e);

	// Render the scene using the passes that have been configured.
	virtual void render(pgRenderEventArgs& e);

};