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

#include "pgApp.h"
#include "engine/render/gltfmodelpass.h"
#include "engine/render/cubepass.h"
#include "engine/render/cubetexpass.h"
#include "engine/render/opaquepass.h"
#include "engine/render/cube.h"
#include "engine/render/cubetex.h"

#include "engine/scene/sceneass.h"

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

#define PG_USE_TEST 0

namespace Diligent
{
#include "BasicStructures.fxh"
//#include "ToneMappingStructures.fxh"

	SampleBase* CreateSample()
	{
		return new pgApp();
	}

	void pgApp::Initialize(IEngineFactory* pEngineFactory, IRenderDevice* pDevice, IDeviceContext** ppContexts, Uint32 NumDeferredCtx, ISwapChain* pSwapChain)
	{
		SampleBase::Initialize(pEngineFactory, pDevice, ppContexts, NumDeferredCtx, pSwapChain);

		pgCreateInfo ci;
		ci.device = m_pDevice;
		ci.ctx = m_pImmediateContext;
		ci.factory = m_pEngineFactory;
		ci.desc = m_pSwapChain->GetDesc();

		pgCameraCreateInfo cci{ ci };
#if !PG_USE_TEST
		cci.pos = float3(0, 0, -25);
#endif
		m_pCamera = std::make_shared<pgCamera>(cci);

		// technique will clean up passed added in it
		m_pTechnique = std::make_shared<pgTechnique>();

		pgPassCreateInfo pci {ci};
		pgSceneCreateInfo sci {ci};


#if !PG_USE_TEST
		std::shared_ptr<pgSceneAss> sceneAss = std::make_shared<pgSceneAss>(sci);
		std::wstring filePath = L"resources/models/test/test_scene.nff";
		sceneAss->LoadFromFile(filePath);

		pci.scene = sceneAss;
		std::shared_ptr<pgOpaquePass> pOpaquePass = std::make_shared<pgOpaquePass>(pci);
		m_pTechnique->addPass(pOpaquePass);
#endif

#if PG_USE_TEST && 0
		std::shared_ptr<pgGLTFPass> pGLTFPass = std::make_shared<pgGLTFPass>(pci);
		m_pTechnique->addPass(pGLTFPass);
#else
		//
#endif

#if PG_USE_TEST
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
#endif
	}

	pgApp::~pgApp()
	{
	}

	// Render a frame
	void pgApp::Render()
	{
		// Clear the back buffer 
		const float ClearColor[] = { 0.032f,  0.032f,  0.032f, 1.0f };
		m_pImmediateContext->ClearRenderTarget(nullptr, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		m_pImmediateContext->ClearDepthStencil(nullptr, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		m_pTechnique->render(m_evtArgs);
	}


	void pgApp::Update(double CurrTime, double ElapsedTime)
	{
		SampleBase::Update(CurrTime, ElapsedTime);

		m_pCamera->update(&m_InputController, (float)ElapsedTime);

		m_evtArgs.set(this, (float)CurrTime, (float)ElapsedTime, m_pCamera.get());

		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
		//ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
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

		m_pTechnique->update(m_evtArgs);

		ImGui::End();
	}
}
