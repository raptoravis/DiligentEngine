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
#include "MapHelper.h"
#include "BasicMath.h"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "CommonlyUsedStates.h"
#include "ShaderMacroHelper.h"
#include "FileSystem.h"
#include "imgui.h"
#include "imGuIZMO.h"

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

		auto BackBufferFmt = m_pSwapChain->GetDesc().ColorBufferFormat;
		auto DepthBufferFmt = m_pSwapChain->GetDesc().DepthBufferFormat;
		auto width = m_pSwapChain->GetDesc().Width;
		auto height = m_pSwapChain->GetDesc().Height;

		m_pEnvMapPass = new pgEnvMapPass(m_pDevice, m_pImmediateContext, m_pEngineFactory, 
			BackBufferFmt, DepthBufferFmt, width, height);
	}

	void pgApp::UpdateUI()
	{
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			//Quaternion rot = Quaternion(m_cameraTransform);
			//ImGui::gizmo3D("Model Rotation", rot, ImGui::GetTextLineHeight() * 10);
			const float3& pos = m_pCamera->getPos();
			const float3& look = m_pCamera->getLook();

			ImGui::Text("pos: %f %f %f", pos.x, pos.y, pos.z);
			ImGui::Text("look: %f %f %f", look.x, look.y, look.z);

			if (ImGui::Button("Reset view"))
			{
				m_pCamera->reset();
			}

			ImGui::Separator();

			float4x4 camTt = m_pCamera->getTransform();
			//camTt.m10 *= -1.0f;
			//camTt.m11 *= -1.0f;
			//camTt.m12 *= -1.0f;
			//camTt.m20 *= -1.0f;
			//camTt.m21 *= -1.0f;
			//camTt.m22 *= -1.0f;

			Quaternion rot = mRot2Quat(camTt);
			ImGui::gizmo3D("Camera", rot, ImGui::GetTextLineHeight() * 10);
			ImGui::SameLine();
			//ImGui::gizmo3D("Light direction", m_LightDirection, ImGui::GetTextLineHeight() * 10);

			m_pEnvMapPass->UpdateUI();
		}
		ImGui::End();
	}

	pgApp::~pgApp()
	{
		delete m_pCamera;
		m_pCamera = 0;
	}

	// Render a frame
	void pgApp::Render()
	{
		// Clear the back buffer 
		const float ClearColor[] = { 0.032f,  0.032f,  0.032f, 1.0f };
		m_pImmediateContext->ClearRenderTarget(nullptr, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		m_pImmediateContext->ClearDepthStencil(nullptr, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		

		m_pEnvMapPass->Render(m_pCamera);
	}


	void pgApp::Update(double CurrTime, double ElapsedTime)
	{
		SampleBase::Update(CurrTime, ElapsedTime);

		m_pCamera->update(&m_InputController, (float)ElapsedTime);

		UpdateUI();

		m_pEnvMapPass->Update((float)CurrTime, (float)ElapsedTime);
	}
}
