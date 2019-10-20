#pragma once

#include "SampleBase.h"

#include "../engine.h"

#include "passrender.h"
#include "../utils/light.h"

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

class PassLight : public pgPass {
	typedef pgPass base;

protected:

	const std::vector<pgLight>*			m_pLights;

	RefCntAutoPtr<IBuffer>				m_LightParamsCB;
	RefCntAutoPtr<IBuffer>				m_ScreenToViewParamsCB;

	RefCntAutoPtr<IBuffer>              m_PerObjectConstants;

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

	std::shared_ptr<pgTechnique>		m_pTechniqueSphere;
	std::shared_ptr<pgTechnique>		m_pTechniqueSpot;
	std::shared_ptr<pgTechnique>		m_pTechniqueDir;
		
	void createBuffers();

	void updateLightParams(const LightParams& lightParam, const pgLight& light);
	void updateScreenToViewParams();
public:
    PassLight(pgTechnique* parentTechnique, std::shared_ptr<pgPipeline> front, 
		std::shared_ptr<pgPipeline> back, 
		std::shared_ptr<pgPipeline> dir, 
		const std::vector<pgLight>*	Lights);

	virtual ~PassLight();

	virtual void Render();

};

