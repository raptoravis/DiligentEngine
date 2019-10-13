#pragma once

#include "cubetexpass.h"

pgCubeTexPass::pgCubeTexPass(const pgPassCreateInfo& ci)
	: base(ci)
{
	LoadTexture();

	CreatePipelineState();
}

pgCubeTexPass::~pgCubeTexPass()
{
}

void pgCubeTexPass::CreatePipelineState()
{
	// Pipeline state object encompasses configuration of all GPU stages

	PipelineStateDesc PSODesc;
	// Pipeline state name is used by the engine to report issues.
	// It is always a good idea to give objects descriptive names.
	PSODesc.Name = "Cube PSO";

	// This is a graphics pipeline
	PSODesc.IsComputePipeline = false;

	// This tutorial will render to a single render target
	PSODesc.GraphicsPipeline.NumRenderTargets = 1;
	// Set render target format which is the format of the swap chain's color buffer
	PSODesc.GraphicsPipeline.RTVFormats[0] = m_desc.ColorBufferFormat;
	// Set depth buffer format which is the format of the swap chain's back buffer
	PSODesc.GraphicsPipeline.DSVFormat = m_desc.DepthBufferFormat;
	// Primitive topology defines what kind of primitives will be rendered by this pipeline state
	PSODesc.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	// Cull back faces
	PSODesc.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_BACK;

	PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = True;
	PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend = BLEND_FACTOR_SRC_ALPHA;
	PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend = BLEND_FACTOR_INV_SRC_ALPHA;
	PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlendAlpha = BLEND_FACTOR_ZERO;
	PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlendAlpha = BLEND_FACTOR_ONE;

	// Enable depth testing
	PSODesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;

	ShaderCreateInfo ShaderCI;
	// Tell the system that the shader source code is in HLSL.
	// For OpenGL, the engine will convert this into GLSL under the hood.
	ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

	// OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
	ShaderCI.UseCombinedTextureSamplers = true;

	// Create a shader source stream factory to load shaders from files.
	RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
	m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
	ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
	// Create a vertex shader
	RefCntAutoPtr<IShader> pVS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
		ShaderCI.EntryPoint = "main";
		ShaderCI.Desc.Name = "Cube VS";
		ShaderCI.FilePath = "cubetex.vsh";
		m_pDevice->CreateShader(ShaderCI, &pVS);
		// Create dynamic uniform buffer that will store our transformation matrix
		// Dynamic buffers can be frequently updated by the CPU
		CreateUniformBuffer(m_pDevice, sizeof(float4x4), "VS constants CB", &m_VSConstants);
	}

	// Create a pixel shader
	RefCntAutoPtr<IShader> pPS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
		ShaderCI.EntryPoint = "main";
		ShaderCI.Desc.Name = "Cube PS";
		ShaderCI.FilePath = "cubetex.psh";
		m_pDevice->CreateShader(ShaderCI, &pPS);
	}

	// Define vertex shader input layout
	LayoutElement LayoutElems[] =
	{
		// Attribute 0 - vertex position
		LayoutElement{0, 0, 3, VT_FLOAT32, False},
		// Attribute 1 - texture coordinates
		LayoutElement{1, 0, 2, VT_FLOAT32, False}
	};

	PSODesc.GraphicsPipeline.pVS = pVS;
	PSODesc.GraphicsPipeline.pPS = pPS;
	PSODesc.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
	PSODesc.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

	// Define variable type that will be used by default
	PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

	// Shader variables should typically be mutable, which means they are expected
	// to change on a per-instance basis
	ShaderResourceVariableDesc Vars[] =
	{
		{SHADER_TYPE_PIXEL, "g_Texture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
	};
	PSODesc.ResourceLayout.Variables = Vars;
	PSODesc.ResourceLayout.NumVariables = _countof(Vars);

	// Define static sampler for g_Texture. Static samplers should be used whenever possible
	SamplerDesc SamLinearClampDesc
	{
		FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
		TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP
	};
	StaticSamplerDesc StaticSamplers[] =
	{
		{SHADER_TYPE_PIXEL, "g_Texture", SamLinearClampDesc}
	};
	PSODesc.ResourceLayout.StaticSamplers = StaticSamplers;
	PSODesc.ResourceLayout.NumStaticSamplers = _countof(StaticSamplers);

	m_pDevice->CreatePipelineState(PSODesc, &m_pPSO);

	// Since we did not explcitly specify the type for 'Constants' variable, default
	// type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables 
	// never change and are bound directly through the pipeline state object.
	m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(m_VSConstants);

	// Since we are using mutable variable, we must create a shader resource binding object
	// http://diligentgraphics.com/2016/03/23/resource-binding-model-in-diligent-engine-2-0/
	m_pPSO->CreateShaderResourceBinding(&m_SRB, true);

	// Set texture SRV in the SRB
	m_SRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_Texture")->Set(m_TextureSRV);
}

void pgCubeTexPass::LoadTexture()
{
	TextureLoadInfo loadInfo;
	loadInfo.IsSRGB = false;
	RefCntAutoPtr<ITexture> Tex;
	//CreateTextureFromFile("DGLogo.png", loadInfo, m_pDevice, &Tex);
	CreateTextureFromFile("apple-logo.png", loadInfo, m_pDevice, &Tex);
	// Get shader resource view from the texture
	m_TextureSRV = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
}


// Render a frame
void pgCubeTexPass::Render(RenderEventArgs& e)
{
	//// Clear the back buffer 
	//const float ClearColor[] = { 0.350f,  0.350f,  0.350f, 1.0f };
	//m_pImmediateContext->ClearRenderTarget(nullptr, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	//m_pImmediateContext->ClearDepthStencil(nullptr, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	{
		// Map the buffer and write current world-view-projection matrix
		MapHelper<float4x4> CBConstants(m_pImmediateContext, m_VSConstants, MAP_WRITE, MAP_FLAG_DISCARD);
		*CBConstants = m_WorldViewProjMatrix.Transpose();
	}

	// Set the pipeline state
	m_pImmediateContext->SetPipelineState(m_pPSO);
	// Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode 
	// makes sure that resources are transitioned to required states.
	m_pImmediateContext->CommitShaderResources(m_SRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

	m_scene->Render(e);
}

void pgCubeTexPass::Update(RenderEventArgs& e)
{
	const bool IsGL = m_pDevice->GetDeviceCaps().IsGLDevice();
	const float4x4 view = e.pCamera->getTransform();

	// Set cube world view matrix
	float4x4 CubeWorldView = float4x4::Scale(0.6f) * float4x4::RotationY(static_cast<float>(e.CurrTime) * 1.0f) * float4x4::RotationX(-PI_F * 0.1f) *
		float4x4::Translation(0.f, 0.0f, 5.0f) * view;
	float NearPlane = 0.1f;
	float FarPlane = 100.f;
	float aspectRatio = static_cast<float>(m_desc.Width) / static_cast<float>(m_desc.Height);
	// Projection matrix differs between DX and OpenGL
	auto Proj = float4x4::Projection(PI_F / 4.f, aspectRatio, NearPlane, FarPlane, IsGL);
	// Compute world-view-projection matrix
	m_WorldViewProjMatrix = CubeWorldView * Proj;
}

