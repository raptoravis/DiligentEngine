#pragma once

#include <cmath>
#include <array>

#include "gltfmodelpass.h"

using namespace Diligent;

namespace Diligent {
#include "BasicStructures.fxh"
#include "ToneMappingStructures.fxh"
}

namespace
{
	struct EnvMapRenderAttribs
	{
		ToneMappingAttribs TMAttribs;

		float AverageLogLum;
		float MipLevel;
		float Unusued1;
		float Unusued2;
	};
}


const std::pair<const char*, const char*> pgGLTFPass::GLTFModels[] =
{
	{"Sponza",				"models/Sponza/Sponza.gltf"},
	{"VirtualCity",			"models/VirtualCity/VC.gltf"},

	{"Damaged Helmet",      "models/DamagedHelmet/DamagedHelmet.gltf"},
	{"Metal Rough Spheres", "models/MetalRoughSpheres/MetalRoughSpheres.gltf"},
	{"Flight Helmet",       "models/FlightHelmet/FlightHelmet.gltf"},
	{"Cesium Man",          "models/CesiumMan/CesiumMan.gltf"},
	{"Boom Box",            "models/BoomBoxWithAxes/BoomBoxWithAxes.gltf"},
	{"Normal Tangent Test", "models/NormalTangentTest/NormalTangentTest.gltf"}
};

void pgGLTFPass::LoadModel(const char* Path)
{
	if (m_Model)
	{
		m_GLTFRenderer->ReleaseResourceBindings(*m_Model);
		m_PlayAnimation = false;
		m_AnimationIndex = 0;
		m_AnimationTimers.clear();
	}

	m_Model.reset(new GLTF::Model(m_pDevice, m_pImmediateContext, Path));
	m_GLTFRenderer->InitializeResourceBindings(*m_Model, m_CameraAttribsCB, m_LightAttribsCB);

	// Center and scale model
	float3 ModelDim{ m_Model->aabb[0][0], m_Model->aabb[1][1], m_Model->aabb[2][2] };
	//float Scale = (1.0f / std::max(std::max(ModelDim.x, ModelDim.y), ModelDim.z)) * 0.5f;
	float Scale = 1.0;
	auto Translate = -float3(m_Model->aabb[3][0], m_Model->aabb[3][1], m_Model->aabb[3][2]);
	Translate += -0.5f * ModelDim;
	m_ModelTransform = float4x4::Translation(Translate) * float4x4::Scale(Scale);

	if (!m_Model->Animations.empty())
	{
		m_AnimationTimers.resize(m_Model->Animations.size());
		m_AnimationIndex = 0;
		m_PlayAnimation = true;
	}
}


pgGLTFPass::pgGLTFPass(const pgPassCreateInfo& ci)
	: base(ci)
{
	RefCntAutoPtr<ITexture> EnvironmentMap;

	CreateTextureFromFile("textures/papermill.ktx", TextureLoadInfo{ "Environment map" }, m_pDevice, &EnvironmentMap);
	m_EnvironmentMapSRV = EnvironmentMap->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

	CreateUniformBuffer(m_pDevice, sizeof(EnvMapRenderAttribs), "Env map render attribs buffer", &m_EnvMapRenderAttribsCB);

	GLTF_PBR_Renderer::CreateInfo RendererCI;
	RendererCI.RTVFmt = m_desc.ColorBufferFormat;
	RendererCI.DSVFmt = m_desc.DepthBufferFormat;
	RendererCI.AllowDebugView = true;
	RendererCI.UseIBL = true;
	RendererCI.FrontCCW = true;

	m_GLTFRenderer.reset(new GLTF_PBR_Renderer(m_pDevice, m_pImmediateContext, RendererCI));

	CreateUniformBuffer(m_pDevice, sizeof(CameraAttribs), "Camera attribs buffer", &m_CameraAttribsCB);
	CreateUniformBuffer(m_pDevice, sizeof(LightAttribs), "Light attribs buffer", &m_LightAttribsCB);

	StateTransitionDesc Barriers[] =
	{
		{m_CameraAttribsCB,        RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, true},
		{m_LightAttribsCB,         RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, true},
		{m_EnvMapRenderAttribsCB,  RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, true},
		{EnvironmentMap,           RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_SHADER_RESOURCE, true}
	};
	m_pImmediateContext->TransitionResourceStates(_countof(Barriers), Barriers);

	m_GLTFRenderer->PrecomputeCubemaps(m_pDevice, m_pImmediateContext, m_EnvironmentMapSRV);

	CreateEnvMapPSO();

	m_LightDirection = normalize(float3(0.5f, -0.6f, -0.2f));

	LoadModel(GLTFModels[m_SelectedModel].second);
}

pgGLTFPass::~pgGLTFPass()
{
}

void pgGLTFPass::UpdateUI()
{
	{
		ImGui::SameLine();
		ImGui::gizmo3D("Light direction", m_LightDirection, ImGui::GetTextLineHeight() * 10);

		ImGui::Separator();

		ImGui::SetNextTreeNodeOpen(true, ImGuiCond_FirstUseEver);
		if (ImGui::TreeNode("Lighting"))
		{
			ImGui::ColorEdit3("Light Color", &m_LightColor.r);
			ImGui::SliderFloat("Light Intensity", &m_LightIntensity, 0.f, 50.f);
			ImGui::SliderFloat("Occlusion strength", &m_RenderParams.OcclusionStrength, 0.f, 1.f);
			ImGui::SliderFloat("Emission scale", &m_RenderParams.EmissionScale, 0.f, 1.f);
			ImGui::SliderFloat("IBL scale", &m_RenderParams.IBLScale, 0.f, 1.f);
			ImGui::TreePop();
		}

		if (!m_Model->Animations.empty())
		{
			ImGui::SetNextTreeNodeOpen(true, ImGuiCond_FirstUseEver);
			if (ImGui::TreeNode("Animation"))
			{
				ImGui::Checkbox("Play", &m_PlayAnimation);
				std::vector<const char*> Animations(m_Model->Animations.size());
				for (int i = 0; i < m_Model->Animations.size(); ++i)
					Animations[i] = m_Model->Animations[i].Name.c_str();
				ImGui::Combo("Active Animation", reinterpret_cast<int*>(&m_AnimationIndex), Animations.data(), static_cast<int>(Animations.size()));
				ImGui::TreePop();
			}
		}

		ImGui::SetNextTreeNodeOpen(true, ImGuiCond_FirstUseEver);
		if (ImGui::TreeNode("Tone mapping"))
		{
			ImGui::SliderFloat("Average log lum", &m_RenderParams.AverageLogLum, 0.01f, 10.0f);
			ImGui::SliderFloat("Middle gray", &m_RenderParams.MiddleGray, 0.01f, 1.0f);
			ImGui::SliderFloat("White point", &m_RenderParams.WhitePoint, 0.1f, 20.0f);
			ImGui::TreePop();
		}

		{
			std::array<const char*, static_cast<size_t>(BackgroundMode::NumModes)> BackgroundModes;
			BackgroundModes[static_cast<size_t>(BackgroundMode::None)] = "None";
			BackgroundModes[static_cast<size_t>(BackgroundMode::EnvironmentMap)] = "Environmen Map";
			BackgroundModes[static_cast<size_t>(BackgroundMode::Irradiance)] = "Irradiance";
			BackgroundModes[static_cast<size_t>(BackgroundMode::PrefilteredEnvMap)] = "PrefilteredEnvMap";
			if (ImGui::Combo("Background mode", reinterpret_cast<int*>(&m_BackgroundMode), BackgroundModes.data(), static_cast<int>(BackgroundModes.size())))
			{
				CreateEnvMapSRB();
			}
		}

		ImGui::SliderFloat("Env map mip", &m_EnvMapMipLevel, 0.0f, 7.0f);

		{
			std::array<const char*, static_cast<size_t>(GLTF_PBR_Renderer::RenderInfo::DebugViewType::NumDebugViews)> DebugViews;

			DebugViews[static_cast<size_t>(GLTF_PBR_Renderer::RenderInfo::DebugViewType::None)] = "None";
			DebugViews[static_cast<size_t>(GLTF_PBR_Renderer::RenderInfo::DebugViewType::BaseColor)] = "Base Color";
			DebugViews[static_cast<size_t>(GLTF_PBR_Renderer::RenderInfo::DebugViewType::Transparency)] = "Transparency";
			DebugViews[static_cast<size_t>(GLTF_PBR_Renderer::RenderInfo::DebugViewType::NormalMap)] = "Normal Map";
			DebugViews[static_cast<size_t>(GLTF_PBR_Renderer::RenderInfo::DebugViewType::Occlusion)] = "Occlusion";
			DebugViews[static_cast<size_t>(GLTF_PBR_Renderer::RenderInfo::DebugViewType::Emissive)] = "Emissive";
			DebugViews[static_cast<size_t>(GLTF_PBR_Renderer::RenderInfo::DebugViewType::Metallic)] = "Metallic";
			DebugViews[static_cast<size_t>(GLTF_PBR_Renderer::RenderInfo::DebugViewType::Roughness)] = "Roughness";
			DebugViews[static_cast<size_t>(GLTF_PBR_Renderer::RenderInfo::DebugViewType::DiffuseColor)] = "Diffuse color";
			DebugViews[static_cast<size_t>(GLTF_PBR_Renderer::RenderInfo::DebugViewType::SpecularColor)] = "Specular color (R0)";
			DebugViews[static_cast<size_t>(GLTF_PBR_Renderer::RenderInfo::DebugViewType::Reflectance90)] = "Reflectance90";
			DebugViews[static_cast<size_t>(GLTF_PBR_Renderer::RenderInfo::DebugViewType::MeshNormal)] = "Mesh normal";
			DebugViews[static_cast<size_t>(GLTF_PBR_Renderer::RenderInfo::DebugViewType::PerturbedNormal)] = "Perturbed normal";
			DebugViews[static_cast<size_t>(GLTF_PBR_Renderer::RenderInfo::DebugViewType::NdotV)] = "n*v";
			DebugViews[static_cast<size_t>(GLTF_PBR_Renderer::RenderInfo::DebugViewType::DiffuseIBL)] = "Diffuse IBL";
			DebugViews[static_cast<size_t>(GLTF_PBR_Renderer::RenderInfo::DebugViewType::SpecularIBL)] = "Specular IBL";
			ImGui::Combo("Debug view", reinterpret_cast<int*>(&m_RenderParams.DebugView), DebugViews.data(), static_cast<int>(DebugViews.size()));
		}
	}
}

void pgGLTFPass::CreateEnvMapPSO()
{
	ShaderCreateInfo ShaderCI;
	RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
	m_pEngineFactory->CreateDefaultShaderSourceStreamFactory("shaders", &pShaderSourceFactory);
	ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
	ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
	ShaderCI.UseCombinedTextureSamplers = true;

	ShaderMacroHelper Macros;
	Macros.AddShaderMacro("TONE_MAPPING_MODE", "TONE_MAPPING_MODE_UNCHARTED2");
	ShaderCI.Macros = Macros;

	ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
	ShaderCI.Desc.Name = "Environment map VS";
	ShaderCI.EntryPoint = "main";
	ShaderCI.FilePath = "env_map.vsh";
	RefCntAutoPtr<IShader> pVS;
	m_pDevice->CreateShader(ShaderCI, &pVS);

	ShaderCI.Desc.Name = "Environment map PS";
	ShaderCI.EntryPoint = "main";
	ShaderCI.FilePath = "env_map.psh";
	ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
	RefCntAutoPtr<IShader> pPS;
	m_pDevice->CreateShader(ShaderCI, &pPS);

	PipelineStateDesc PSODesc;
	PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

	StaticSamplerDesc StaticSamplers[] =
	{
		{SHADER_TYPE_PIXEL, "EnvMap", Sam_LinearClamp}
	};
	PSODesc.ResourceLayout.StaticSamplers = StaticSamplers;
	PSODesc.ResourceLayout.NumStaticSamplers = _countof(StaticSamplers);

	ShaderResourceVariableDesc Vars[] =
	{
		{SHADER_TYPE_PIXEL, "cbCameraAttribs",       SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		{SHADER_TYPE_PIXEL, "cbEnvMapRenderAttribs", SHADER_RESOURCE_VARIABLE_TYPE_STATIC}
	};
	PSODesc.ResourceLayout.Variables = Vars;
	PSODesc.ResourceLayout.NumVariables = _countof(Vars);

	PSODesc.Name = "EnvMap PSO";
	PSODesc.GraphicsPipeline.pVS = pVS;
	PSODesc.GraphicsPipeline.pPS = pPS;

	PSODesc.GraphicsPipeline.RTVFormats[0] = m_desc.ColorBufferFormat;
	PSODesc.GraphicsPipeline.NumRenderTargets = 1;
	PSODesc.GraphicsPipeline.DSVFormat = m_desc.DepthBufferFormat;
	PSODesc.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	PSODesc.GraphicsPipeline.DepthStencilDesc.DepthFunc = COMPARISON_FUNC_LESS_EQUAL;

	m_pDevice->CreatePipelineState(PSODesc, &m_EnvMapPSO);
	m_EnvMapPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbCameraAttribs")->Set(m_CameraAttribsCB);
	m_EnvMapPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbEnvMapRenderAttribs")->Set(m_EnvMapRenderAttribsCB);
	CreateEnvMapSRB();
}

void pgGLTFPass::CreateEnvMapSRB()
{
	if (m_BackgroundMode != BackgroundMode::None)
	{
		m_EnvMapSRB.Release();
		m_EnvMapPSO->CreateShaderResourceBinding(&m_EnvMapSRB, true);
		ITextureView* pEnvMapSRV = nullptr;
		switch (m_BackgroundMode)
		{
		case BackgroundMode::EnvironmentMap:
			pEnvMapSRV = m_EnvironmentMapSRV;
			break;

		case BackgroundMode::Irradiance:
			pEnvMapSRV = m_GLTFRenderer->GetIrradianceCubeSRV();
			break;

		case BackgroundMode::PrefilteredEnvMap:
			pEnvMapSRV = m_GLTFRenderer->GetPrefilteredEnvMapSRV();
			break;

		default:
			UNEXPECTED("Unexpected background mode");
		}
		m_EnvMapSRB->GetVariableByName(SHADER_TYPE_PIXEL, "EnvMap")->Set(pEnvMapSRV);
	}
}


void pgGLTFPass::Update(RenderEventArgs& e) {
	UpdateUI();

	if (!m_Model->Animations.empty() && m_PlayAnimation)
	{
		float& AnimationTimer = m_AnimationTimers[m_AnimationIndex];
		AnimationTimer += static_cast<float>(e.ElapsedTime);
		AnimationTimer = std::fmod(AnimationTimer, m_Model->Animations[m_AnimationIndex].End);
		m_Model->UpdateAnimation(m_AnimationIndex, AnimationTimer);
	}
}

void pgGLTFPass::Render(RenderEventArgs& e) {
	float4x4 CameraView = e.pCamera->getTransform();

	float4x4 CameraWorld = CameraView.Inverse();
	float3 CameraWorldPos = float3::MakeVector(CameraWorld[3]);
	float NearPlane = 0.1f;
	float FarPlane = 100.f;
	float aspectRatio = static_cast<float>(m_desc.Width) / static_cast<float>(m_desc.Height);
	// Projection matrix differs between DX and OpenGL
	auto Proj = float4x4::Projection(PI_F / 4.f, aspectRatio, NearPlane, FarPlane, m_pDevice->GetDeviceCaps().IsGLDevice());
	// Compute world-view-projection matrix
	auto CameraViewProj = CameraView * Proj;

	{
		MapHelper<CameraAttribs> CamAttribs(m_pImmediateContext, m_CameraAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD);
		CamAttribs->mProjT = Proj.Transpose();
		CamAttribs->mViewProjT = CameraViewProj.Transpose();
		CamAttribs->mViewProjInvT = CameraViewProj.Inverse().Transpose();
		CamAttribs->f4Position = float4(CameraWorldPos, 1);
	}

	{
		MapHelper<LightAttribs> lightAttribs(m_pImmediateContext, m_LightAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD);
		lightAttribs->f4Direction = m_LightDirection;
		lightAttribs->f4Intensity = m_LightColor * m_LightIntensity;
	}

	m_RenderParams.ModelTransform = m_ModelTransform;
	m_GLTFRenderer->Render(m_pImmediateContext, *m_Model, m_RenderParams);

	if (m_BackgroundMode != BackgroundMode::None)
	{
		{
			MapHelper<EnvMapRenderAttribs> EnvMapAttribs(m_pImmediateContext, m_EnvMapRenderAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD);
			EnvMapAttribs->TMAttribs.iToneMappingMode = TONE_MAPPING_MODE_UNCHARTED2;
			EnvMapAttribs->TMAttribs.bAutoExposure = 0;
			EnvMapAttribs->TMAttribs.fMiddleGray = m_RenderParams.MiddleGray;
			EnvMapAttribs->TMAttribs.bLightAdaptation = 0;
			EnvMapAttribs->TMAttribs.fWhitePoint = m_RenderParams.WhitePoint;
			EnvMapAttribs->TMAttribs.fLuminanceSaturation = 1.0;
			EnvMapAttribs->AverageLogLum = m_RenderParams.AverageLogLum;
			EnvMapAttribs->MipLevel = m_EnvMapMipLevel;
		}
		m_pImmediateContext->SetPipelineState(m_EnvMapPSO);
		m_pImmediateContext->CommitShaderResources(m_EnvMapSRB, RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		DrawAttribs drawAttribs(3, DRAW_FLAG_VERIFY_ALL);
		m_pImmediateContext->Draw(drawAttribs);
	}
}

