/*     
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF ANY PROPRIETARY RIGHTS.
 *
 *  In no event and under no legal theory, whether in tort (including negligence), 
 *  contract, or otherwise, unless required by applicable law (such as deliberate 
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental, 
 *  or consequential damages of any character arising as a result of this License or 
 *  out of the use or inability to use the software (including but not limited to damages 
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and 
 *  all other commercial damages or losses), even if such Contributor has been advised 
 *  of the possibility of such damages.
 */

#include "app.h"

#include "engine/render/gltfmodelpass.h"
#include "engine/render/cubepass.h"
#include "engine/render/cubetexpass.h"
#include "engine/render/cube.h"
#include "engine/render/cubetex.h"

#include "engine/render/opaquepass.h"
#include "engine/render/transparentpass.h"

#include "MapHelper.h"
#include "BasicMath.h"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "CommonlyUsedStates.h"
#include "ShaderMacroHelper.h"
#include "FileSystem.h"
#include "imgui.h"
#include "imGuIZMO.h"

#include <windows.h>

#include "engine/mat2quat.h"

#include "testscene.h"

namespace Diligent
{
#include "BasicStructures.fxh"
//#include "ToneMappingStructures.fxh"

	SampleBase* CreateSample()
	{
		return new TestApp();
	}

	void TestApp::initLightData() {
		pgLight light1;

		light1.m_PositionVS = { -0.535466492f, -1.40531516f, 0.451306254f, 1.00000000f };
		light1.m_DirectionWS = { 0.0116977794f, -0.170993939f, -0.985219836f, 0.000000000f };
		light1.m_PositionVS = { -0.00858783722f, -0.924078941f, -32.9505081f, 1.00000000f };
		light1.m_DirectionVS = { 0.0165561363f, -0.176791593f, -0.984109104f, 0.000000000f };
		light1.m_Color = { 1.00000000f, 1.00000000f, 1.00000000f, 1.00000000f };
		light1.m_SpotlightAngle = 36.2999992f;
		light1.m_Range = 16.5599995f;
		light1.m_Intensity = 1.00000000f;
		light1.m_Enabled = 1;
		light1.m_Selected = 0;
		light1.m_Type = pgLight::LightType::Point;

		pgLight light2;

		light2.m_PositionVS = { -16.1441193f, 2.10133481f, 21.5686855f, 1.00000000f };
		light2.m_DirectionWS = { 0.585131526f, -0.282768548f, -0.760157943f, 0.000000000f };
		light2.m_PositionVS = { -15.7199173f, 2.60687327f, -11.9134722f, 1.00000000f };
		light2.m_DirectionVS = { 0.589790821f, -0.283460617f, -0.756172597f, 0.000000000f };
		light2.m_Color = { 1.00000000f, 1.00000000f, 1.00000000f, 1.00000000f };
		light2.m_SpotlightAngle = 45.f;
		light2.m_Range = 1.f;
		light2.m_Intensity = 1.00000000f;
		light2.m_Enabled = 1;
		light2.m_Selected = 0;
		light2.m_Type = pgLight::LightType::Directional;

		m_Lights.push_back(light1);
		m_Lights.push_back(light2);
	}

	void TestApp::initBuffers() {
		{
			// Create dynamic uniform buffer that will store our transformation matrix
			// Dynamic buffers can be frequently updated by the CPU
			BufferDesc CBDesc;
			CBDesc.Name = "PerObject CB";
			CBDesc.uiSizeInBytes = sizeof(PerObject);
			CBDesc.Usage = USAGE_DYNAMIC;
			CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
			CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
			m_pDevice->CreateBuffer(CBDesc, nullptr, &m_PerObjectConstants);
		}

		{
			BufferDesc CBDesc;
			CBDesc.Name = "Material CB";
			uint32_t bufferSize = pgMaterial::getConstantBufferSize();
			CBDesc.uiSizeInBytes = bufferSize;
			CBDesc.Usage = USAGE_DYNAMIC;
			CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
			CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
			m_pDevice->CreateBuffer(CBDesc, nullptr, &m_MaterialConstants);
		}

		//IBufferView* pLightsBufferUAV;

		{
			BufferDesc BuffDesc;
			BuffDesc.Name = "Lights StructuredBuffer";
			BuffDesc.Usage = USAGE_DYNAMIC;
			BuffDesc.BindFlags = BIND_SHADER_RESOURCE;
			BuffDesc.Mode = BUFFER_MODE_STRUCTURED;
			BuffDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
			BuffDesc.ElementByteStride = sizeof(pgLight);
			BuffDesc.uiSizeInBytes = (uint32_t)(sizeof(pgLight) * m_Lights.size());

			BufferData VBData;
			VBData.pData = m_Lights.data();
			VBData.DataSize = sizeof(pgLight) * static_cast<Uint32>(m_Lights.size());
			m_pDevice->CreateBuffer(BuffDesc, &VBData, &m_LightsStructuredBuffer);

			m_LightsBufferSRV = m_LightsStructuredBuffer->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE);
			//pLightsBufferUAV = m_LightsStructuredBuffer->GetDefaultView(BUFFER_VIEW_UNORDERED_ACCESS);
		}
	}

	void TestApp::updateSRB_Object(pgRenderEventArgs& e, Diligent::IDeviceContext* ctx) {
		{
			const float4x4 view = e.pCamera->getViewMatrix();
			const float4x4 local = e.pSceneNode->getLocalTransform();

			// Set cube world view matrix
			float4x4 worldViewMatrix = local * view;
			auto& Proj = e.pCamera->getProjectionMatrix();
			// Compute world-view-projection matrix
			float4x4 worldViewProjMatrix = worldViewMatrix * Proj;

			// Map the buffer and write current world-view-projection matrix
			MapHelper<PerObject> CBConstants(ctx, m_PerObjectConstants, MAP_WRITE, MAP_FLAG_DISCARD);

			//CBConstants->ModelViewProjection = m_WorldViewProjMatrix.Transpose();
			//CBConstants->ModelView = m_WorldViewMatrix.Transpose();
			CBConstants->ModelViewProjection = worldViewProjMatrix;
			CBConstants->ModelView = worldViewMatrix;
		}
	}

	void TestApp::updateSRB_Material(pgRenderEventArgs& e, Diligent::IDeviceContext* ctx) {
		{
			// Map the buffer and write current world-view-projection matrix
			MapHelper<pgMaterial::MaterialProperties> CBConstants(ctx, m_MaterialConstants, MAP_WRITE, MAP_FLAG_DISCARD);

			auto matProperties = e.pMaterial->getConstantBuffer();

			//CBConstants->ModelViewProjection = m_WorldViewProjMatrix.Transpose();
			//CBConstants->ModelView = m_WorldViewMatrix.Transpose();
			memcpy((void*)&CBConstants->m_GlobalAmbient, matProperties, sizeof(pgMaterial::MaterialProperties));
		}
	}

	void TestApp::updateSRB_Lights(pgRenderEventArgs& e, Diligent::IDeviceContext* ctx) {
		{
			const float4x4 viewMatrix = e.pCamera->getViewMatrix();

			// Update the viewspace vectors of the light.
			for (uint32_t i = 0; i < m_Lights.size(); i++)
			{
				// Update the lights so that their position and direction are in view space.
				pgLight& light = m_Lights[i];
				light.m_PositionVS = float4(light.m_PositionWS, 1) * viewMatrix;
				light.m_DirectionVS = normalize(float4(light.m_DirectionWS, 0) * viewMatrix);
			}

			{
				// Map the buffer and write current world-view-projection matrix
				MapHelper<pgLight> lightBuffer(m_pImmediateContext, m_LightsStructuredBuffer, MAP_WRITE, MAP_FLAG_DISCARD);

				memcpy(&lightBuffer->m_PositionWS, m_Lights.data(), sizeof(pgLight) * m_Lights.size());
			}
		}
	}

	void TestApp::Initialize(IEngineFactory* pEngineFactory, IRenderDevice* pDevice, IDeviceContext** ppContexts, Uint32 NumDeferredCtx, ISwapChain* pSwapChain)
	{
		SampleBase::Initialize(pEngineFactory, pDevice, ppContexts, NumDeferredCtx, pSwapChain);

		pgCreateInfo ci;
		ci.device = m_pDevice;
		ci.ctx = m_pImmediateContext;
		ci.factory = m_pEngineFactory;
		ci.desc = m_pSwapChain->GetDesc();

		pgCameraCreateInfo cci{ ci };

		m_renderingTechnique = RenderingTechnique::Forward;
		//m_renderingTechnique = RenderingTechnique::Test;

		if (m_renderingTechnique == RenderingTechnique::Forward) {
			cci.pos = float3(0, 0, -25);
		}

		m_pCamera = std::make_shared<pgCamera>(cci);

		// technique will clean up passed added in it
		m_pTechnique = std::make_shared<pgTechnique>();
		m_pForwardTechnique = std::make_shared<pgTechnique>();
		m_pDeferredTechnique = std::make_shared<pgTechnique>();
		m_pForwardPlusTechnique = std::make_shared<pgTechnique>();

		pgPassCreateInfo pci {ci};
		pgSceneCreateInfo sci{ ci };

		if (m_renderingTechnique == RenderingTechnique::Forward) {
			std::shared_ptr<TestScene> testScene = std::make_shared<TestScene>(sci);
			std::wstring filePath = L"resources/models/test/test_scene.nff";
			testScene->LoadFromFile(filePath);
			testScene->customMesh();

			initLightData();
			initBuffers();

			BasePassCreateInfo bpci{ pci };
			bpci.PerObjectConstants = m_PerObjectConstants.RawPtr();
			bpci.MaterialConstants = m_MaterialConstants.RawPtr();
			bpci.LightsStructuredBuffer = m_LightsStructuredBuffer.RawPtr();
			bpci.LightsBufferSRV = m_LightsBufferSRV.RawPtr();

			bpci.scene = testScene;
			std::shared_ptr<OpaquePass> pOpaquePass = std::make_shared<OpaquePass>(bpci);
			m_pForwardTechnique->addPass(pOpaquePass);

			//std::shared_ptr<TransparentPass> pTransparentPass = std::make_shared<TransparentPass>(bpci);
			//m_pForwardTechnique->addPass(pTransparentPass);
		}

		// always init test technique
		{
#if 0
			std::shared_ptr<pgGLTFPass> pGLTFPass = std::make_shared<pgGLTFPass>(pci);
			m_pTechnique->addPass(pGLTFPass);
#endif
			std::shared_ptr<Cube> cube = std::make_shared<Cube>(m_pDevice, m_pImmediateContext);
			std::shared_ptr<CubeTex> cubeTex = std::make_shared<CubeTex>(m_pDevice, m_pImmediateContext);

			float4x4 trans1 = float4x4::RotationX(-PI_F * 0.1f) *float4x4::Translation(0.f, 0.0f, 8.0f);
			std::shared_ptr<pgSceneNode> root1 = std::make_shared<pgSceneNode>(trans1);
			root1->addMesh(cube);
			std::shared_ptr<pgScene> sceneCube = std::make_shared<pgScene>(sci);
			sceneCube->setRootNode(root1);

			float4x4 trans2 = float4x4::Scale(0.6f) * float4x4::RotationX(-PI_F * 0.1f) *float4x4::Translation(0.f, 0.0f, 5.0f);
			std::shared_ptr<pgSceneNode> root2 = std::make_shared<pgSceneNode>(trans2);
			root2->addMesh(cubeTex);
			std::shared_ptr<pgScene> sceneCubeTex = std::make_shared<pgScene>(sci);
			sceneCubeTex->setRootNode(root2);

			pci.scene = sceneCube;
			std::shared_ptr<pgCubePass> pCubePass = std::make_shared<pgCubePass>(pci);
			m_pTechnique->addPass(pCubePass);

			pci.scene = sceneCubeTex;
			std::shared_ptr<pgCubeTexPass> pCubeTexPass = std::make_shared<pgCubeTexPass>(pci);
			m_pTechnique->addPass(pCubeTexPass);
		}
	}

	TestApp::~TestApp()
	{
	}

	// Render a frame
	void TestApp::Render()
	{
		// Clear the back buffer 
		const float ClearColor[] = { 0.032f,  0.032f,  0.032f, 1.0f };
		m_pImmediateContext->ClearRenderTarget(nullptr, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		m_pImmediateContext->ClearDepthStencil(nullptr, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		if (m_renderingTechnique == RenderingTechnique::Test) {
			m_pTechnique->render(m_evtArgs);
		}

		if (m_renderingTechnique == RenderingTechnique::Forward) {
			m_pForwardTechnique->render(m_evtArgs);
		}

		if (m_renderingTechnique == RenderingTechnique::Deferred) {
			m_pDeferredTechnique->render(m_evtArgs);
		}

		if (m_renderingTechnique == RenderingTechnique::ForwardPlus) {
			m_pForwardPlusTechnique->render(m_evtArgs);
		}
	}


	void TestApp::Update(double CurrTime, double ElapsedTime)
	{
		SampleBase::Update(CurrTime, ElapsedTime);

		m_pCamera->update(&m_InputController, (float)ElapsedTime);

		m_evtArgs.set(this, (float)CurrTime, (float)ElapsedTime, m_pCamera.get());

		int technique = (int)m_renderingTechnique;

		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
		//ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			const char* desc[] =
			{
				"test",
				"forward",
				"deferred",
				"forward+"
			};
			//ImGui::SetNextItemWidth(100);
			ImGui::Combo("technique", &technique, desc, IM_ARRAYSIZE(desc));

			//Quaternion rot = Quaternion(m_viewMatrix);
			//ImGui::gizmo3D("Model Rotation", rot, ImGui::GetTextLineHeight() * 10);
			const float3& pos = m_pCamera->getPos();
			const float3& look = m_pCamera->getLook();

			ImGui::Text("pos: %f %f %f", pos.x, pos.y, pos.z);

			// we need to negative it
			ImGui::Text("look: %f %f %f", -look.x, -look.y, -look.z);

			if (ImGui::Button("Reset view"))
			{
				m_pCamera->reset();
			}

			ImGui::Separator();

			float4x4 camTt = m_pCamera->getViewMatrix();

			//Quaternion rot = mRot2Quat(camTt);
			Quaternion rot = calculateRotation(camTt);

			ImGui::gizmo3D("Camera", rot, ImGui::GetTextLineHeight() * 10);
		}

		if ((int)m_renderingTechnique != technique) {
			m_renderingTechnique = (RenderingTechnique)technique;

			if (m_renderingTechnique == RenderingTechnique::Test) {
				m_pCamera->reset(float3(0, 0, 0), float3(0, 0, -1));
			}
			else if (m_renderingTechnique == RenderingTechnique::Forward) {
				m_pCamera->reset(float3(0, 0, -25), float3(0, 0, -1));
			}
		}
		
		if (m_renderingTechnique == RenderingTechnique::Test) {
			m_pTechnique->update(m_evtArgs);
		}

		if (m_renderingTechnique == RenderingTechnique::Forward) {
			m_pForwardTechnique->update(m_evtArgs);
		}

		if (m_renderingTechnique == RenderingTechnique::Deferred) {
			m_pDeferredTechnique->update(m_evtArgs);
		}

		if (m_renderingTechnique == RenderingTechnique::ForwardPlus) {
			m_pForwardPlusTechnique->update(m_evtArgs);
		}

		ImGui::End();
	}
}
