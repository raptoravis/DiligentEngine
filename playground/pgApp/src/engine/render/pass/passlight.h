#pragma once

#include "SampleBase.h"

#include "../../engine.h"

#include "passrender.h"
#include "../light.h"

using namespace Diligent;

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
	void CreatePipelineState(PipelineStateDesc& PSODesc);

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

	std::shared_ptr<pgPassPilpeline>	m_pSubPassSphere0;
	std::shared_ptr<pgPassPilpeline>	m_pSubPassSphere1;
	std::shared_ptr<pgPassPilpeline>	m_pSubPassSpot0;
	std::shared_ptr<pgPassPilpeline>	m_pSubPassSpot1;
	std::shared_ptr<pgPassPilpeline>	m_pSubPassDir;
		

	void updateLightParams(pgRenderEventArgs& e, const LightParams& lightParam, const pgLight& light);
	void updateScreenToViewParams(pgRenderEventArgs& e, pgBindFlag flag);
public:
	PassLight(const pgPassRenderCreateInfo& ci,
		std::shared_ptr<pgRenderTarget> rt, 
		std::shared_ptr<pgPipeline> front, 
		std::shared_ptr<pgPipeline> back, 
		std::shared_ptr<pgPipeline> dir, 
		const std::vector<pgLight>*	Lights);

	virtual ~PassLight();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void update(pgRenderEventArgs& e);
	virtual void render(pgRenderEventArgs& e);
	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag);
	virtual void unbind(pgRenderEventArgs& e, pgBindFlag flag);

	virtual bool meshFilter(pgMesh* mesh);
};

