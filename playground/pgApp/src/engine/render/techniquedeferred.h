#pragma once 

#include "SampleBase.h"

#include "../engine.h"
#include "pass/passrender.h"
#include "light.h"

using namespace Diligent;

class TechniqueDeferred : public pgTechnique {
	typedef pgTechnique base;

	void createGBuffers(const pgCreateInfo& ci);

	std::shared_ptr<pgRenderTarget>		 m_pGBufferRT;
	std::shared_ptr<pgRenderTarget>		 m_pDepthOnlyRT;
	std::shared_ptr<pgTexture>			 m_depthStencilTexture;
public:
	TechniqueDeferred(const pgTechniqueCreateInfo& ci);
	virtual ~TechniqueDeferred();

	void init(const pgPassRenderCreateInfo& ci, const std::vector<pgLight>& lights);

	virtual void update(pgRenderEventArgs& e);

	// Render the scene using the passes that have been configured.
	virtual void render(pgRenderEventArgs& e);

};