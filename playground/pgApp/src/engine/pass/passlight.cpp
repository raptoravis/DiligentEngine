#include "passlight.h"
#include "../scene/sceneass.h"

#include "../utils/mathutils.h"

PassLight::PassLight(IBuffer* PerObjectConstants, 
	IBuffer* LightParamsCB,
	IBuffer* ScreenToViewParamsCB,
	std::shared_ptr<pgPipeline> front,
	std::shared_ptr<pgPipeline> back,
	std::shared_ptr<pgPipeline> dir,
	const std::vector<pgLight>*	Lights)
	: base(0)
	, m_PerObjectConstants(PerObjectConstants)
	, m_LightParamsCB(LightParamsCB)
	, m_ScreenToViewParamsCB(ScreenToViewParamsCB)
	, m_pLights(Lights)
	, m_LightPipeline0(front)
	, m_LightPipeline1(back)
	, m_DirectionalLightPipeline(dir)
{
	m_pPointLightScene = pgSceneAss::CreateSphere(1.0f);
	m_pSpotLightScene = pgSceneAss::CreateCylinder(0.0f, 1.0f, 1.0f, float3(0, 0, 1));
	m_pDirectionalLightScene = pgSceneAss::CreateScreenQuad(-1, 1, -1, 1, -1);

	m_pSubPassSphere0 = std::make_shared<pgPassPilpeline>(m_pPointLightScene, m_LightPipeline0);
	m_pSubPassSphere1 = std::make_shared<pgPassPilpeline>(m_pPointLightScene, m_LightPipeline1);

	m_pSubPassSpot0 = std::make_shared<pgPassPilpeline>(m_pSpotLightScene, m_LightPipeline0);
	m_pSubPassSpot1 = std::make_shared<pgPassPilpeline>(m_pSpotLightScene, m_LightPipeline1);

	m_pSubPassDir = std::make_shared<pgPassPilpeline>(m_pDirectionalLightScene, m_DirectionalLightPipeline);

	m_pTechniqueSphere = std::make_shared<pgTechnique>(nullptr, nullptr);
	m_pTechniqueSphere->addPass(m_pSubPassSphere0);
	m_pTechniqueSphere->addPass(m_pSubPassSphere1);

	m_pTechniqueSpot = std::make_shared<pgTechnique>(nullptr, nullptr);
	m_pTechniqueSpot->addPass(m_pSubPassSpot0);
	m_pTechniqueSpot->addPass(m_pSubPassSpot1);


	m_pTechniqueDir = std::make_shared<pgTechnique>(nullptr, nullptr);
	m_pTechniqueDir->addPass(m_pSubPassDir);

}

PassLight::~PassLight()
{
}

bool PassLight::meshFilter(pgMesh* mesh) {
	auto mat = mesh->getMaterial();
	auto bTransparent = mat->IsTransparent();
	return !bTransparent;
}


void PassLight::updateLightParams(pgRenderEventArgs& e, const LightParams& lightParam, const pgLight& light) {
	{
		// Map the buffer and write current world-view-projection matrix
		MapHelper<LightParams> CBConstants(e.pDeviceContext, m_LightParamsCB, MAP_WRITE, MAP_FLAG_DISCARD);

		CBConstants->m_LightIndex = lightParam.m_LightIndex;
	}

	{
		// Map the buffer and write current world-view-projection matrix
		MapHelper<PerObject> CBConstants(e.pDeviceContext, m_PerObjectConstants, MAP_WRITE, MAP_FLAG_DISCARD);

		if (light.m_Type == pgLight::LightType::Directional) {
			//CBConstants->ModelViewProjection = m_WorldViewProjMatrix.Transpose();
			//CBConstants->ModelView = m_WorldViewMatrix.Transpose();
			bool IsGL = pgApp::s_device->GetDeviceCaps().IsGLDevice();
			Diligent::float4x4 othoMat = Diligent::float4x4::Ortho((float)pgApp::s_desc.Width, (float)pgApp::s_desc.Height, 0.f, 1.f, IsGL);
			CBConstants->ModelViewProjection = othoMat;
			CBConstants->ModelView = float4x4::Identity();
		}
		else {
			auto& Proj = e.pCamera->getProjectionMatrix();
			//const float4x4 nodeTransform = e.pSceneNode->getLocalTransform();
			const float4x4 nodeTransform = Diligent::float4x4::Identity();

			Diligent::float4x4 translation = Diligent::float4x4::Translation(Diligent::float3(light.m_PositionWS));
			// Create a rotation matrix that rotates the model towards the direction of the light.
			Diligent::float4x4 rotation = MakeQuaternionFromTwoVec3(Diligent::float3(0, 0, 1), normalize(Diligent::float3(light.m_DirectionWS))).ToMatrix();

			// Compute the scale depending on the light type.
			float scaleX, scaleY, scaleZ;
			// For point lights, we want to scale the geometry by the range of the light.
			scaleX = scaleY = scaleZ = light.m_Range;
			if (light.m_Type == pgLight::LightType::Spot)
			{
				// For spotlights, we want to scale the base of the cone by the spotlight angle.
				scaleX = scaleY = std::tan((PI_F / 180.0f) * (light.m_SpotlightAngle)) * light.m_Range;
			}

			Diligent::float4x4 scale = Diligent::float4x4::Scale(Diligent::float3(scaleX, scaleY, scaleZ));

			Diligent::float4x4 modelViewMat = nodeTransform * scale * rotation * translation * e.pCamera->getViewMatrix();
			CBConstants->ModelView = modelViewMat;
			CBConstants->ModelViewProjection = modelViewMat * Proj;
		}
	}
}

void PassLight::updateScreenToViewParams(pgRenderEventArgs& e, pgBindFlag flag) {
	{
		// Map the buffer and write current world-view-projection matrix
		MapHelper<ScreenToViewParams> CBConstants(e.pDeviceContext, m_ScreenToViewParamsCB, MAP_WRITE, MAP_FLAG_DISCARD);

		auto& Proj = e.pCamera->getProjectionMatrix();
		//CBConstants->ModelViewProjection = m_WorldViewProjMatrix.Transpose();
		//CBConstants->ModelView = m_WorldViewMatrix.Transpose();
		CBConstants->m_InverseProjectionMatrix = Proj.Inverse();
		CBConstants->m_ScreenDimensions = float2((float)pgApp::s_desc.Width, (float)pgApp::s_desc.Height);
	}
}

// Render a frame
void PassLight::render(pgRenderEventArgs& e) {
	updateScreenToViewParams(e, pgBindFlag::pgBindFlag_Pass);

	if (m_pLights) {
		LightParams	lightParams;

		lightParams.m_LightIndex = 0;

		for (const pgLight& light : *m_pLights) {
			if (light.m_Enabled) {
				// Update the constant buffer for the per-light data.
				updateLightParams(e, lightParams, light);

				// Clear the stencil buffer for the next light
				m_LightPipeline0->getRenderTarget()->Clear(pgClearFlags::Stencil, 
					Diligent::float4(0,0,0,0), 1.0f, 1);
				// The other pipelines should have the same render target.. so no need to clear it 3 times.

				switch (light.m_Type)
				{
				case pgLight::LightType::Point:
					m_pTechniqueSphere->_render(e);
					break;
				case pgLight::LightType::Spot:
					m_pTechniqueSpot->_render(e);
					break;
				case pgLight::LightType::Directional:
					m_pTechniqueDir->_render(e);
					break;
				}
			}

			lightParams.m_LightIndex++;
		}
	}
}

void PassLight::update(pgRenderEventArgs& e) {
	//
}

void PassLight::bind(pgRenderEventArgs& e, pgBindFlag flag) {
	base::bind(e, flag);
}

void PassLight::unbind(pgRenderEventArgs& e, pgBindFlag flag) {
	base::unbind(e, flag);
}

void PassLight::Render() {

}
