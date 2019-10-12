#pragma once

#include "SampleBase.h"
#include "GLTFLoader.h"
#include "GLTF_PBR_Renderer.h"

#include "engine.h"

using namespace Diligent;

class pgEnvMapPass : public pgBasePass {
	typedef pgBasePass base;

	const TEXTURE_FORMAT		m_backbufferFormat;
	const TEXTURE_FORMAT		m_depthFormat;
	const int					width;
	const int					height;


	Diligent::RefCntAutoPtr<Diligent::IPipelineState>         m_EnvMapPSO;
	Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_EnvMapSRB;
	Diligent::RefCntAutoPtr<Diligent::ITextureView>           m_EnvironmentMapSRV;
	Diligent::RefCntAutoPtr<Diligent::IBuffer>                m_EnvMapRenderAttribsCB;

	void CreateEnvMapPSO();
	void CreateEnvMapSRB();
	void LoadModel(const char* Path);

	enum class BackgroundMode : int
	{
		None,
		EnvironmentMap,
		Irradiance,
		PrefilteredEnvMap,
		NumModes
	}m_BackgroundMode = BackgroundMode::PrefilteredEnvMap;


	float3     m_LightDirection;
	float4     m_LightColor = float4(1, 1, 1, 1);
	float      m_LightIntensity = 3.f;
	float      m_EnvMapMipLevel = 1.f;

	GLTF_PBR_Renderer::RenderInfo m_RenderParams;
	float4x4   m_ModelTransform;

	int        m_SelectedModel = 0;
	static const std::pair<const char*, const char*> GLTFModels[];

	bool               m_PlayAnimation = false;
	int                m_AnimationIndex = 0;
	std::vector<float> m_AnimationTimers;

	std::unique_ptr<GLTF_PBR_Renderer> m_GLTFRenderer;
	std::unique_ptr<GLTF::Model>       m_Model;

	Diligent::RefCntAutoPtr<Diligent::IBuffer> m_CameraAttribsCB;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> m_LightAttribsCB;

public:
	pgEnvMapPass(Diligent::IRenderDevice* pDevice, IDeviceContext* pCtx, IEngineFactory* factory, 
		TEXTURE_FORMAT BackBufferFmt, TEXTURE_FORMAT DepthBufferFmt, 
		int w, int h);

	virtual ~pgEnvMapPass();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void Update(float CurrTime, float ElapsedTime);
	virtual void Render(Camera* pCamera);

	void UpdateUI();

};

