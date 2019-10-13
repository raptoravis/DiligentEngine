#pragma once

#include "opaquepass.h"

pgOpaquePass::pgOpaquePass(const pgPassCreateInfo& ci)
	: base(ci)
{
	LoadTexture();

	CreatePipelineState();
}

pgOpaquePass::~pgOpaquePass()
{
}

void pgOpaquePass::CreatePipelineState()
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
	// Enable depth testing
	PSODesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;

	ShaderCreateInfo ShaderCI;
	// Tell the system that the shader source code is in HLSL.
	// For OpenGL, the engine will convert this into GLSL under the hood.
	ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

	// OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
	ShaderCI.UseCombinedTextureSamplers = false;

	// In this tutorial, we will load shaders from file. To be able to do that,
	// we need to create a shader source stream factory
	RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
	m_pEngineFactory->CreateDefaultShaderSourceStreamFactory("./resources/shaders", &pShaderSourceFactory);
	ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
	// Create a vertex shader
	RefCntAutoPtr<IShader> pVS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
		ShaderCI.EntryPoint = "VS_main";
		ShaderCI.Desc.Name = "OpaqueVS";
		ShaderCI.FilePath = "ForwardRendering.hlsl";
		m_pDevice->CreateShader(ShaderCI, &pVS);
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

	// Create a pixel shader
	RefCntAutoPtr<IShader> pPS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
		ShaderCI.EntryPoint = "PS_main";
		ShaderCI.Desc.Name = "OpaquePS";
		ShaderCI.FilePath = "ForwardRendering.hlsl";
		m_pDevice->CreateShader(ShaderCI, &pPS);
	}

	// Define vertex shader input layout
	LayoutElement LayoutElems[] =
	{
		LayoutElement{0, 0, 3, VT_FLOAT32, False}, //position
		LayoutElement{0, 1, 3, VT_FLOAT32, False}, //tangent
		LayoutElement{0, 2, 3, VT_FLOAT32, False}, //binormal
		LayoutElement{0, 3, 3, VT_FLOAT32, False}, //normal
		LayoutElement{0, 4, 2, VT_FLOAT32, False}, //tex
	};

	LayoutElems[0].SemanticName = "POSITION";
	LayoutElems[1].SemanticName = "TANGENT";
	LayoutElems[2].SemanticName = "BINORMAL";
	LayoutElems[3].SemanticName = "NORMAL";
	LayoutElems[4].SemanticName = "TEXCOORD";

	PSODesc.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
	PSODesc.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

	PSODesc.GraphicsPipeline.pVS = pVS;
	PSODesc.GraphicsPipeline.pPS = pPS;

	// Define variable type that will be used by default
	PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

	m_pDevice->CreatePipelineState(PSODesc, &m_pPSO);

	// Since we did not explcitly specify the type for 'Constants' variable, default
	// type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never 
	// change and are bound directly through the pipeline state object.
	m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "PerObject")->Set(m_PerObjectConstants);

	// Create a shader resource binding object and bind all static resources in it
	m_pPSO->CreateShaderResourceBinding(&m_SRB, true);
}

void pgOpaquePass::LoadTexture()
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
void pgOpaquePass::render(pgRenderEventArgs& e) {
	// Set the pipeline state
	m_pImmediateContext->SetPipelineState(m_pPSO);

	m_scene->render(e);
}

void pgOpaquePass::update(pgRenderEventArgs& e) {
	//
}

void pgOpaquePass::updateSRB(pgRenderEventArgs& e) {
	const float4x4 view = e.pCamera->getViewMatrix();
	const float4x4 local = e.pSceneNode->getLocalTransform();

	// Set cube world view matrix
	m_WorldViewMatrix = local * view;

	auto& Proj = e.pCamera->getProjectionMatrix();

	// Compute world-view-projection matrix
	m_WorldViewProjMatrix = m_WorldViewMatrix * Proj;

	{
		// Map the buffer and write current world-view-projection matrix
		MapHelper<PerObject> CBConstants(m_pImmediateContext, m_PerObjectConstants, MAP_WRITE, MAP_FLAG_DISCARD);

		CBConstants->ModelViewProjection = m_WorldViewProjMatrix.Transpose();
		CBConstants->ModelView = m_WorldViewMatrix.Transpose();
	}

	// Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode 
	// makes sure that resources are transitioned to required states.
	m_pImmediateContext->CommitShaderResources(m_SRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}