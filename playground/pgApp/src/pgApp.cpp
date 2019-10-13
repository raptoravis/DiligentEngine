/*     Copyright 2019 Diligent Graphics LLC
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

#include "mat2quat.h"

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

		m_pCamera = new Camera();

		// technique will clean up passed added in it
		m_pTechnique = new pgTechnique();

		pgPassCreateInfo ci;
		ci.device = m_pDevice;
		ci.ctx = m_pImmediateContext;
		ci.factory = m_pEngineFactory;
		ci.desc = m_pSwapChain->GetDesc();
#if 0	
		ci.scene = sceneCubeTex;
		std::shared_ptr<pgOpaquePass> pOpaquePass = std::make_shared<pgOpaquePass>(ci);
		m_pTechnique->addPass(pOpaquePass);
#endif

#if 0
		std::shared_ptr<pgGLTFPass> pGLTFPass = std::make_shared<pgGLTFPass>(ci);
		m_pTechnique->addPass(pGLTFPass);
#else
		//
#endif

#if 1
		std::shared_ptr<Cube> cube = std::make_shared<Cube>(m_pDevice, m_pImmediateContext);
		std::shared_ptr<CubeTex> cubeTex = std::make_shared<CubeTex>(m_pDevice, m_pImmediateContext);

		std::shared_ptr<SceneNode> root1 = std::make_shared<SceneNode>();
		root1->AddMesh(cube);
		std::shared_ptr<Scene> sceneCube = std::make_shared<Scene>(root1);

		std::shared_ptr<SceneNode> root2 = std::make_shared<SceneNode>();
		root2->AddMesh(cubeTex);
		std::shared_ptr<Scene> sceneCubeTex = std::make_shared<Scene>(root2);

		ci.scene = sceneCube;
		std::shared_ptr<pgCubePass> pCubePass = std::make_shared<pgCubePass>(ci);
		m_pTechnique->addPass(pCubePass);

		ci.scene = sceneCubeTex;
		std::shared_ptr<pgCubeTexPass> pCubeTexPass = std::make_shared<pgCubeTexPass>(ci);
		m_pTechnique->addPass(pCubeTexPass);
#endif
	}

	pgApp::~pgApp()
	{
		delete m_pCamera;

		delete m_pTechnique;
	}

	// Render a frame
	void pgApp::Render()
	{
		// Clear the back buffer 
		const float ClearColor[] = { 0.032f,  0.032f,  0.032f, 1.0f };
		m_pImmediateContext->ClearRenderTarget(nullptr, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		m_pImmediateContext->ClearDepthStencil(nullptr, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		m_pTechnique->Render(m_evtArgs);
	}


	void pgApp::Update(double CurrTime, double ElapsedTime)
	{
		SampleBase::Update(CurrTime, ElapsedTime);

		m_pCamera->update(&m_InputController, (float)ElapsedTime);

		m_evtArgs.set(this, (float)CurrTime, (float)ElapsedTime, m_pCamera);

		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
		//ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			//Quaternion rot = Quaternion(m_cameraTransform);
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

			float4x4 camTt = m_pCamera->getTransform();

			//Quaternion rot = mRot2Quat(camTt);
			Quaternion rot = calculateRotation(camTt);

			ImGui::gizmo3D("Camera", rot, ImGui::GetTextLineHeight() * 10);
		}

		m_pTechnique->Update(m_evtArgs);

		ImGui::End();
	}
}
