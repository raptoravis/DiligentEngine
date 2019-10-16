#pragma once

#include "SampleBase.h"

#include "../../engine.h"

#include "passrender.h"
#include "../light.h"

using namespace Diligent;

struct GeometryPassCreateInfo : public RenderPassCreateInfo {
	//
	GeometryPassCreateInfo() {
		//
	}

	GeometryPassCreateInfo(const RenderPassCreateInfo& ci)
		: RenderPassCreateInfo(ci)
	{
	}

	ITextureView*           ColorRTV;
	ITextureView*           DSRTV;

	ITextureView*           DiffuseRTV;
	ITextureView*           SpecularRTV;
	ITextureView*           NormalRTV;
};


class GeometryPass : public pgRenderPass {
	typedef pgRenderPass base;

protected:
	void CreatePipelineState(const RenderPassCreateInfo& ci, PipelineStateDesc& PSODesc);

	RefCntAutoPtr<ITextureView>           m_pColorRTV;
	RefCntAutoPtr<ITextureView>           m_pDSRTV;

	RefCntAutoPtr<ITextureView>           m_pDiffuseRTV;
	RefCntAutoPtr<ITextureView>           m_pSpecularRTV;
	RefCntAutoPtr<ITextureView>           m_pNormalRTV;

public:
	GeometryPass(const GeometryPassCreateInfo& ci);

	virtual ~GeometryPass();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag);

	virtual bool meshFilter(pgMesh* mesh);
};

