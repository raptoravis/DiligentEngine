#include "pipelinelightback.h"

using namespace Diligent;

PipelineLightBack::PipelineLightBack(std::shared_ptr<pgRenderTarget> rt, 
	std::shared_ptr<pgRenderTarget> GBufferRT,
	IBuffer* PerObjectConstants, 
	IBufferView* LightsBufferSRV,
	Diligent::IBuffer* LightParamsCB,
	Diligent::IBuffer* ScreenToViewParamsCB)
	: base(rt)
	, m_pGBufferRT(GBufferRT)
	, m_PerObjectConstants(PerObjectConstants)
	, m_LightParamsCB(LightParamsCB)
	, m_ScreenToViewParamsCB(ScreenToViewParamsCB)
	, m_LightsBufferSRV(LightsBufferSRV)
{
	CreatePipelineState();
}

PipelineLightBack::~PipelineLightBack() {

}

void PipelineLightBack::CreatePipelineState()
{
	// Pipeline state object encompasses configuration of all GPU stages

	PipelineStateDesc PSODesc;
	// Pipeline state name is used by the engine to report issues.
	// It is always a good idea to give objects descriptive names.
	PSODesc.Name = "PipelineLightBack";

	// This is a graphics pipeline
	PSODesc.IsComputePipeline = false;

	// This tutorial will render to a single render target
	PSODesc.GraphicsPipeline.NumRenderTargets = 0;
	// Set render target format which is the format of the swap chain's color buffer
	PSODesc.GraphicsPipeline.RTVFormats[0] = pgApp::s_desc.ColorBufferFormat;
	// Set depth buffer format which is the format of the swap chain's back buffer
	PSODesc.GraphicsPipeline.DSVFormat = pgApp::s_desc.DepthBufferFormat;
	// Primitive topology defines what kind of primitives will be rendered by this pipeline state
	PSODesc.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	// Cull back faces
	PSODesc.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_FRONT;
	PSODesc.GraphicsPipeline.RasterizerDesc.DepthClipEnable = False;
	//PSODesc.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = True;

	PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = True;
	PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend = BLEND_FACTOR_ONE;
	PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend = BLEND_FACTOR_ONE;

	// Enable depth testing and Disable depth writes
	PSODesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;
	PSODesc.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = False;
	PSODesc.GraphicsPipeline.DepthStencilDesc.DepthFunc = COMPARISON_FUNC_GREATER_EQUAL;

	// Render pixel if the depth function passes and the stencil was not un-marked in the previous pass.
	PSODesc.GraphicsPipeline.DepthStencilDesc.StencilEnable = True;
	PSODesc.GraphicsPipeline.DepthStencilDesc.BackFace.StencilFunc = COMPARISON_FUNC_EQUAL;

	ShaderCreateInfo ShaderCI;
	// Tell the system that the shader source code is in HLSL.
	// For OpenGL, the engine will convert this into GLSL under the hood.
	ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

	// OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
	ShaderCI.UseCombinedTextureSamplers = true;

	// Create a shader source stream factory to load shaders from files.
	RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
	pgApp::s_engineFactory->CreateDefaultShaderSourceStreamFactory("./resources/shaders", &pShaderSourceFactory);
	ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
	// Create a vertex shader
	RefCntAutoPtr<IShader> pVS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
		ShaderCI.EntryPoint = "VS_main";
		ShaderCI.Desc.Name = "VS";
		ShaderCI.FilePath = "ForwardRendering.hlsl";
		pgApp::s_device->CreateShader(ShaderCI, &pVS);
		// Create dynamic uniform buffer that will store our transformation matrix
		// Dynamic buffers can be frequently updated by the CPU
		//CreateUniformBuffer(pgApp::s_device, sizeof(float4x4), "VS constants CB", &m_VSConstants);
	}

	// Create a pixel shader
	RefCntAutoPtr<IShader> pPS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
		ShaderCI.EntryPoint = "PS_DeferredLighting";
		ShaderCI.Desc.Name = "PS_DeferredLighting";
		ShaderCI.FilePath = "DeferredRendering.hlsl";
		pgApp::s_device->CreateShader(ShaderCI, &pPS);
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

	PSODesc.GraphicsPipeline.pVS = pVS;
	PSODesc.GraphicsPipeline.pPS = pPS;
	PSODesc.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
	PSODesc.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

	// Define variable type that will be used by default
	PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

	ShaderResourceVariableDesc Vars[] =
	{
		{SHADER_TYPE_PIXEL, "Lights", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
		{SHADER_TYPE_PIXEL, "DiffuseTextureVS", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
		{SHADER_TYPE_PIXEL, "SpecularTextureVS", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
		{SHADER_TYPE_PIXEL, "NormalTextureVS", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
		{SHADER_TYPE_PIXEL, "DepthTextureVS", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
	};
	PSODesc.ResourceLayout.Variables = Vars;
	PSODesc.ResourceLayout.NumVariables = _countof(Vars);

	//// Define static sampler for g_Texture. Static samplers should be used whenever possible
	//SamplerDesc SamLinearClampDesc
	//{
	//	FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
	//	TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP
	//};
	//StaticSamplerDesc StaticSamplers[] =
	//{
	//	{SHADER_TYPE_PIXEL, "g_Texture", SamLinearClampDesc}
	//};
	//PSODesc.ResourceLayout.StaticSamplers = StaticSamplers;
	//PSODesc.ResourceLayout.NumStaticSamplers = _countof(StaticSamplers);

	pgApp::s_device->CreatePipelineState(PSODesc, &m_pPSO);

	m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "PerObject")->Set(m_PerObjectConstants);
	m_pPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "LightIndexBuffer")->Set(m_LightParamsCB);
	m_pPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "ScreenToViewParams")->Set(m_ScreenToViewParamsCB);

	// Create a shader resource binding object and bind all static resources in it
	m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);

	m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "Lights")->Set(m_LightsBufferSRV);
	auto GbufferRT = m_pGBufferRT;
	auto diffuseTex = GbufferRT->GetTexture(pgRenderTarget::AttachmentPoint::Color1);
	m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "DiffuseTextureVS")->Set(diffuseTex->getShaderResourceView());

	auto specularTex = GbufferRT->GetTexture(pgRenderTarget::AttachmentPoint::Color2);
	m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "SpecularTextureVS")->Set(specularTex->getShaderResourceView());

	auto normalTex = GbufferRT->GetTexture(pgRenderTarget::AttachmentPoint::Color3);
	m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "NormalTextureVS")->Set(normalTex->getShaderResourceView());

	auto depthTex = GbufferRT->GetTexture(pgRenderTarget::AttachmentPoint::DepthStencil);
	m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "DepthTextureVS")->Set(depthTex->getShaderResourceView());
}


void PipelineLightBack::bind(pgRenderEventArgs& e, pgBindFlag flag) {
	if (flag == pgBindFlag::pgBindFlag_Mesh) {
		//
	}
	else if (flag == pgBindFlag::pgBindFlag_Pass) {
		pgApp::s_ctx->SetStencilRef(1);
	}

	base::bind(e, flag);
}

void PipelineLightBack::unbind(pgRenderEventArgs& e, pgBindFlag flag) {
	pgApp::s_ctx->SetStencilRef(1);

	base::unbind(e, flag);
}
