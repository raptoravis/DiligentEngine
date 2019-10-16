#pragma once

#include "SampleBase.h"

#include "../../engine.h"

#include "passrender.h"
#include "../light.h"

using namespace Diligent;

struct GeometryPassCreateInfo : public pgPassRenderCreateInfo {
	GeometryPassCreateInfo(const pgPassRenderCreateInfo& ci)
		: pgPassRenderCreateInfo(ci)
	{
	}

	ITextureView*           ColorRTV;
	ITextureView*           DSRTV;

	ITextureView*           DiffuseRTV;
	ITextureView*           SpecularRTV;
	ITextureView*           NormalRTV;
};


class PassGeometry : public pgPassRender {
	typedef pgPassRender base;

protected:
	void CreatePipelineState(const pgPassRenderCreateInfo& ci, PipelineStateDesc& PSODesc);

	RefCntAutoPtr<ITextureView>           m_pColorRTV;
	RefCntAutoPtr<ITextureView>           m_pDSRTV;

	RefCntAutoPtr<ITextureView>           m_pDiffuseRTV;
	RefCntAutoPtr<ITextureView>           m_pSpecularRTV;
	RefCntAutoPtr<ITextureView>           m_pNormalRTV;

public:
	PassGeometry(const GeometryPassCreateInfo& ci);

	virtual ~PassGeometry();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag);

	virtual bool meshFilter(pgMesh* mesh);
};

