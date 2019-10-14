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

void pgOpaquePass::initLightData() {
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

	IBufferView* pLightsBufferSRV;
	//IBufferView* pLightsBufferUAV;

	initLightData();

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

		pLightsBufferSRV = m_LightsStructuredBuffer->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE);
		//pLightsBufferUAV = m_LightsStructuredBuffer->GetDefaultView(BUFFER_VIEW_UNORDERED_ACCESS);
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

	// Shader variables should typically be mutable, which means they are expected
	// to change on a per-instance basis
	ShaderResourceVariableDesc Vars[] =
	{
		{SHADER_TYPE_PIXEL, "Lights", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
	};
	PSODesc.ResourceLayout.Variables = Vars;
	PSODesc.ResourceLayout.NumVariables = _countof(Vars);

	m_pDevice->CreatePipelineState(PSODesc, &m_pPSO);

	// Since we did not explcitly specify the type for 'Constants' variable, default
	// type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never 
	// change and are bound directly through the pipeline state object.
	m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "PerObject")->Set(m_PerObjectConstants);
	m_pPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "Material")->Set(m_MaterialConstants);
	//m_pPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "Lights")->Set(pLightsBufferSRV);

	// Create a shader resource binding object and bind all static resources in it
	m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);

	m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "Lights")->Set(pLightsBufferSRV);
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
	m_scene->render(e);
}

void pgOpaquePass::update(pgRenderEventArgs& e) {
	//
}

void pgOpaquePass::updateLights(pgCamera* pCamera)
{
	const float4x4 viewMatrix = pCamera->getViewMatrix();

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

		//CBConstants->ModelViewProjection = m_WorldViewProjMatrix.Transpose();
		//CBConstants->ModelView = m_WorldViewMatrix.Transpose();
		memcpy(&lightBuffer->m_PositionWS, m_Lights.data(), sizeof(pgLight) * m_Lights.size());
	}
}

void pgOpaquePass::updateSRB(pgRenderEventArgs& e) {
	const float4x4 view = e.pCamera->getViewMatrix();
	const float4x4 local = e.pSceneNode->getLocalTransform();

	// Set cube world view matrix
	m_WorldViewMatrix = local * view;

	auto& Proj = e.pCamera->getProjectionMatrix();

	// Compute world-view-projection matrix
	m_WorldViewProjMatrix = m_WorldViewMatrix * Proj;

	// Set the pipeline state
	m_pImmediateContext->SetPipelineState(m_pPSO);

	{
		// Map the buffer and write current world-view-projection matrix
		MapHelper<PerObject> CBConstants(m_pImmediateContext, m_PerObjectConstants, MAP_WRITE, MAP_FLAG_DISCARD);

		//CBConstants->ModelViewProjection = m_WorldViewProjMatrix.Transpose();
		//CBConstants->ModelView = m_WorldViewMatrix.Transpose();
		CBConstants->ModelViewProjection = m_WorldViewProjMatrix;
		CBConstants->ModelView = m_WorldViewMatrix;
	}

	{
		// Map the buffer and write current world-view-projection matrix
		MapHelper<pgMaterial::MaterialProperties> CBConstants(m_pImmediateContext, m_MaterialConstants, MAP_WRITE, MAP_FLAG_DISCARD);

		auto matProperties = e.pMaterial->getConstantBuffer();

		//CBConstants->ModelViewProjection = m_WorldViewProjMatrix.Transpose();
		//CBConstants->ModelView = m_WorldViewMatrix.Transpose();
		memcpy((void*)&CBConstants->m_GlobalAmbient, matProperties, sizeof(pgMaterial::MaterialProperties));
	}

	updateLights(e.pCamera);

	// Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode 
	// makes sure that resources are transitioned to required states.
	m_pImmediateContext->CommitShaderResources(m_pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}