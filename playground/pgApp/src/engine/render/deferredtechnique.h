#pragma once 

#include "SampleBase.h"

#include "../engine.h"
#include "renderpass.h"
#include "light.h"

using namespace Diligent;

class DeferredTechnique : public pgTechnique {
	typedef pgTechnique base;

	void createGBuffers();

	RefCntAutoPtr<ITextureView>           m_pColorRTV;
	RefCntAutoPtr<ITextureView>           m_pDSRTV;

	RefCntAutoPtr<ITextureView>           m_pDiffuseRTV;
	RefCntAutoPtr<ITextureView>           m_pSpecularRTV;
	RefCntAutoPtr<ITextureView>           m_pNormalRTV;

	//
	RefCntAutoPtr<ITextureView>           m_pDSSRV;

	RefCntAutoPtr<ITextureView>           m_pDiffuseSRV;
	RefCntAutoPtr<ITextureView>           m_pSpecularSRV;
	RefCntAutoPtr<ITextureView>           m_pNormalSRV;

public:
	DeferredTechnique(const pgTechniqueCreateInfo& ci);
	virtual ~DeferredTechnique();

	void init(const RenderPassCreateInfo& ci, const std::vector<pgLight>& lights);

	virtual void update(pgRenderEventArgs& e);

	// Render the scene using the passes that have been configured.
	virtual void render(pgRenderEventArgs& e);

};