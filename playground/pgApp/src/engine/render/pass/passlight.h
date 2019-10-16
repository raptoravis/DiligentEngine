#pragma once

#include "SampleBase.h"

#include "../../engine.h"

#include "passrender.h"
#include "../light.h"

using namespace Diligent;

struct LightPassCreateInfo : public pgPassRenderCreateInfo {
	LightPassCreateInfo(const pgPassRenderCreateInfo& ci)
		: pgPassRenderCreateInfo(ci)
	{
	}

	std::shared_ptr<pgRenderTarget> rt;

	std::shared_ptr<pgPipeline> front;
	std::shared_ptr<pgPipeline> back;
	std::shared_ptr<pgPipeline> dir;

	const std::vector<pgLight>*	Lights;

};

__declspec(align(16)) struct ScreenToViewParams
{
	float4x4 m_InverseProjectionMatrix;
	float2 m_ScreenDimensions;
	float2 padding;
};

__declspec(align(16)) struct LightParams
{
	uint32_t m_LightIndex;
	uint32_t padding[3];
};

class PassLight : public pgPassRender {
	typedef pgPassRender base;

protected:
	void CreatePipelineState(const pgPassRenderCreateInfo& ci, PipelineStateDesc& PSODesc);

	std::shared_ptr<pgRenderTarget>		m_pGBufferRT;

	const std::vector<pgLight>*			m_pLights;

	RefCntAutoPtr<IBuffer>				m_LightParamsCB;
	RefCntAutoPtr<IBuffer>				m_ScreenToViewParamsCB;

	std::shared_ptr<pgPipeline>			m_LightPipeline0;
	std::shared_ptr<pgPipeline>			m_LightPipeline1;
	std::shared_ptr<pgPipeline>			m_DirectionalLightPipeline;

	std::shared_ptr<pgScene>			m_pPointLightScene;
	std::shared_ptr<pgScene>			m_pSpotLightScene;
	std::shared_ptr<pgScene>			m_pDirectionalLightScene;

	const pgLight*						m_pCurrentLight;

	void updateLightParams(pgRenderEventArgs& e, const LightParams& lightParam);
	void updateScreenToViewParams(pgRenderEventArgs& e);

	void RenderSubPass(pgRenderEventArgs& e, std::shared_ptr<pgScene> scene, std::shared_ptr<pgPipeline> pipeline);
public:
	PassLight(const LightPassCreateInfo& ci);

	virtual ~PassLight();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag);

	virtual bool meshFilter(pgMesh* mesh);
};

