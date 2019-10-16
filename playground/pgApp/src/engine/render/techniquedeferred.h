#pragma once 

#include "SampleBase.h"

#include "../engine.h"
#include "pass/passrender.h"
#include "light.h"

using namespace Diligent;

class TechniqueDeferred : public pgTechnique {
	typedef pgTechnique base;

	void createGBuffers();

	RefCntAutoPtr<ITextureView>           m_pColorRTV;

	RefCntAutoPtr<ITextureView>           m_pDiffuseRTV;
	RefCntAutoPtr<ITextureView>           m_pSpecularRTV;
	RefCntAutoPtr<ITextureView>           m_pNormalRTV;

	//
	RefCntAutoPtr<ITexture>				  m_pDepthBuffer;
	RefCntAutoPtr<ITextureView>           m_pDSRTV;
	RefCntAutoPtr<ITextureView>           m_pDSSRV;

	RefCntAutoPtr<ITextureView>           m_pDiffuseSRV;
	RefCntAutoPtr<ITextureView>           m_pSpecularSRV;
	RefCntAutoPtr<ITextureView>           m_pNormalSRV;

public:
	TechniqueDeferred(const pgTechniqueCreateInfo& ci);
	virtual ~TechniqueDeferred();

	void init(const pgPassRenderCreateInfo& ci, const std::vector<pgLight>& lights);

	virtual void update(pgRenderEventArgs& e);

	// Render the scene using the passes that have been configured.
	virtual void render(pgRenderEventArgs& e);

};