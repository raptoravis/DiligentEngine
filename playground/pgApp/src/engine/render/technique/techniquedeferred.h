#pragma once 

#include "SampleBase.h"

#include "../../engine.h"
#include "../pass/passrender.h"
#include "../light.h"

using namespace Diligent;

class TechniqueDeferred : public pgTechnique {
	typedef pgTechnique base;

	void createGBuffers();
	void createBuffers();

	std::shared_ptr<pgRenderTarget>		 m_pGBufferRT;
	std::shared_ptr<pgRenderTarget>		 m_pDepthOnlyRT;
	std::shared_ptr<pgTexture>			 m_depthStencilTexture;

	RefCntAutoPtr<IBuffer>				 m_LightParamsCB;
	RefCntAutoPtr<IBuffer>				 m_ScreenToViewParamsCB;
public:
	TechniqueDeferred(std::shared_ptr<pgRenderTarget> rt, std::shared_ptr<pgTexture> backBuffer);
	virtual ~TechniqueDeferred();

	void init(const pgPassRenderCreateInfo& prci, const std::vector<pgLight>& lights);

	virtual void update(pgRenderEventArgs& e);

	// Render the scene using the passes that have been configured.
	virtual void render(pgRenderEventArgs& e);

};