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

#pragma warning(disable : 4244)
#include "argparse.h"

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

	static void argparse_test(int argc, char* argv[]) {
		//const auto* cmdLine = GetCommandLineA();

		//run as : [program name] "0 -c" abc - a 1 - sdfl --flag - v 1 2.7 3 4 9 8.12 87
		ArgumentParser parser("Argument parser example");

		parser.add_argument("-a", "an integer");
		parser.add_argument("-s", "an combined flag", true);
		parser.add_argument("-d", "an combined flag", true);
		parser.add_argument("-f", "an combined flag", true);
		parser.add_argument("--flag", "a flag");
		parser.add_argument("-v", "a vector", true);
		parser.add_argument("-l", "--long", "a long argument", false);

		try {
			parser.parse(argc, argv);
		}
		catch (const ArgumentParser::ArgumentNotFound& ex) {
			std::cout << ex.what() << std::endl;
			return;
		}
		if (parser.is_help()) {
			return;
		}
		std::cout << "a: " << parser.get<int>("a") << std::endl;
		std::cout << "flag: " << std::boolalpha << parser.get<bool>("flag")
			<< std::endl;
		std::cout << "d: " << std::boolalpha << parser.get<bool>("d") << std::endl;
		std::cout << "long flag: " << std::boolalpha << parser.get<bool>("l")
			<< std::endl;
		auto v = parser.getv<double>("v");
		std::cout << "v: ";
	}
	
	void pgApp::Initialize(IEngineFactory* pEngineFactory, IRenderDevice* pDevice, IDeviceContext** ppContexts, Uint32 NumDeferredCtx, ISwapChain* pSwapChain)
	{
		SampleBase::Initialize(pEngineFactory, pDevice, ppContexts, NumDeferredCtx, pSwapChain);

		m_pCamera = new Camera();

		auto BackBufferFmt = m_pSwapChain->GetDesc().ColorBufferFormat;
		auto DepthBufferFmt = m_pSwapChain->GetDesc().DepthBufferFormat;
		auto width = m_pSwapChain->GetDesc().Width;
		auto height = m_pSwapChain->GetDesc().Height;

		m_pTechnique = new pgTechnique();

#if 0
		m_pGLTFPass = new pgGLTFPass(m_pDevice, m_pImmediateContext, m_pEngineFactory,
			BackBufferFmt, DepthBufferFmt, width, height);
		m_pTechnique->AddPass(m_pGLTFPass);
#else
		m_pGLTFPass = 0;
#endif
		m_pCubePass = new pgCubePass(m_pDevice, m_pImmediateContext, m_pEngineFactory,
			BackBufferFmt, DepthBufferFmt, width, height);
		m_pTechnique->AddPass(m_pCubePass);

		m_pCubeTexPass = new pgCubeTexPass(m_pDevice, m_pImmediateContext, m_pEngineFactory,
			BackBufferFmt, DepthBufferFmt, width, height);
		m_pTechnique->AddPass(m_pCubeTexPass);
	}

	pgApp::~pgApp()
	{
		delete m_pCamera;
		delete m_pGLTFPass;
		delete m_pCubeTexPass;
		delete m_pCubePass;
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
		ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
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
