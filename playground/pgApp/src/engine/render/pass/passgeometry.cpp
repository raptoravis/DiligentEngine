#include "passgeometry.h"

PassGeometry::PassGeometry(const GeometryPassCreateInfo& ci)
	: base(ci)
	, m_pGBufferRT(ci.rt)
{
	PipelineStateDesc PSODesc;

	CreatePipelineState(ci, PSODesc);
}

PassGeometry::~PassGeometry() {
	//
}

void PassGeometry::CreatePipelineState(const pgPassRenderCreateInfo& ci, PipelineStateDesc& PSODesc) {
	// Pipeline state object encompasses configuration of all GPU stages

	// Pipeline state name is used by the engine to report issues.
	// It is always a good idea to give objects descriptive names.
	PSODesc.Name = "PSO";

	// This is a graphics pipeline
	PSODesc.IsComputePipeline = false;

	// This tutorial will render to a single render target
	PSODesc.GraphicsPipeline.NumRenderTargets = 3;
	// Set render target format which is the format of the swap chain's color buffer
	PSODesc.GraphicsPipeline.RTVFormats[0] = m_desc.ColorBufferFormat;
	PSODesc.GraphicsPipeline.RTVFormats[1] = TEX_FORMAT_RGBA8_UNORM;
	PSODesc.GraphicsPipeline.RTVFormats[2] = TEX_FORMAT_RGBA32_FLOAT;

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
	}

	// Create a pixel shader
	RefCntAutoPtr<IShader> pPS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
		ShaderCI.EntryPoint = "PS_Geometry";
		ShaderCI.Desc.Name = "GeometryPS";
		ShaderCI.FilePath = "DeferredRendering.hlsl";
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

	//// Define static sampler for g_Texture. Static samplers should be used whenever possible
	//StaticSamplerDesc StaticSamplers[] =
	//{
	//	{ SHADER_TYPE_PIXEL, "g_Texture", Sam_LinearClamp }
	//};
	//RTPSODesc.ResourceLayout.StaticSamplers = StaticSamplers;
	//RTPSODesc.ResourceLayout.NumStaticSamplers = _countof(StaticSamplers);

	//
	m_pDevice->CreatePipelineState(PSODesc, &m_pPSO);

	// Since we did not explcitly specify the type for 'Constants' variable, default
	// type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never 
	// change and are bound directly through the pipeline state object.
	m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "PerObject")->Set(ci.PerObjectConstants);
	m_pPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "Material")->Set(ci.MaterialConstants);
	//m_pPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "Lights")->Set(pLightsBufferSRV);

	// Create a shader resource binding object and bind all static resources in it
	m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);

	// no light used
	//m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "Lights")->Set(ci.LightsBufferSRV);
}

bool PassGeometry::meshFilter(pgMesh* mesh) {
	auto mat = mesh->getMaterial();
	auto bTransparent = mat->IsTransparent();
	return !bTransparent;
}

// Render a frame
void PassGeometry::render(pgRenderEventArgs& e) {
	m_pGBufferRT->Bind();
	m_pGBufferRT->Clear(pgClearFlags::All, float4(0.39f, 0.58f, 0.93f, 1.0f), 1.0f, 0);

	m_scene->render(e);
}

void PassGeometry::update(pgRenderEventArgs& e) {
	//
}

void PassGeometry::bind(pgRenderEventArgs& e, pgBindFlag flag) {
	base::bind(e, flag);
}

void PassGeometry::unbind(pgRenderEventArgs& e, pgBindFlag flag) {
	base::unbind(e, flag);
}