#include "passrender.h"

pgPassRender::pgPassRender(const pgPassRenderCreateInfo& ci)
	: base(ci.scene)
	, m_PerObjectConstants(ci.PerObjectConstants)
	, m_MaterialConstants(ci.MaterialConstants)
	, m_LightsStructuredBuffer(ci.LightsStructuredBuffer)
	, m_LightsBufferSRV(ci.LightsBufferSRV)
{
	//LoadTexture();

	//CreatePipelineState(ci);
}

pgPassRender::~pgPassRender()
{
}

void pgPassRender::CreatePipelineState(PipelineStateDesc& PSODesc) {
	// Pipeline state object encompasses configuration of all GPU stages

	// Pipeline state name is used by the engine to report issues.
	// It is always a good idea to give objects descriptive names.
	PSODesc.Name = "PSO";

	// This is a graphics pipeline
	PSODesc.IsComputePipeline = false;

	// This tutorial will render to a single render target
	PSODesc.GraphicsPipeline.NumRenderTargets = 1;
	// Set render target format which is the format of the swap chain's color buffer
	PSODesc.GraphicsPipeline.RTVFormats[0] = pgApp::s_desc.ColorBufferFormat;
	// Set depth buffer format which is the format of the swap chain's back buffer
	PSODesc.GraphicsPipeline.DSVFormat = pgApp::s_desc.DepthBufferFormat;
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
	pgApp::s_engineFactory->CreateDefaultShaderSourceStreamFactory("./resources/shaders", &pShaderSourceFactory);
	ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
	// Create a vertex shader
	RefCntAutoPtr<IShader> pVS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
		ShaderCI.EntryPoint = "VS_main";
		ShaderCI.Desc.Name = "OpaqueVS";
		ShaderCI.FilePath = "ForwardRendering.hlsl";
		pgApp::s_device->CreateShader(ShaderCI, &pVS);
	}

	// Create a pixel shader
	RefCntAutoPtr<IShader> pPS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
		ShaderCI.EntryPoint = "PS_main";
		ShaderCI.Desc.Name = "OpaquePS";
		ShaderCI.FilePath = "ForwardRendering.hlsl";
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
	pgApp::s_device->CreatePipelineState(PSODesc, &m_pPSO);

	// Since we did not explcitly specify the type for 'Constants' variable, default
	// type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never 
	// change and are bound directly through the pipeline state object.
	m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "PerObject")->Set(m_PerObjectConstants);
	m_pPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "Material")->Set(m_MaterialConstants);
	//m_pPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "Lights")->Set(pLightsBufferSRV);

	// Create a shader resource binding object and bind all static resources in it
	m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);

	m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "Lights")->Set(m_LightsBufferSRV);
}

void pgPassRender::LoadTexture()
{
	TextureLoadInfo loadInfo;
	loadInfo.IsSRGB = false;
	RefCntAutoPtr<ITexture> Tex;
	//CreateTextureFromFile("DGLogo.png", loadInfo, pgApp::s_device, &Tex);
	CreateTextureFromFile("apple-logo.png", loadInfo, pgApp::s_device, &Tex);
	// Get shader resource view from the texture
	m_TextureSRV = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
}


// Render a frame
void pgPassRender::render(pgRenderEventArgs& e) {
	m_pScene->_render(e);
}

void pgPassRender::update(pgRenderEventArgs& e) {
	//
}

void pgPassRender::bind(pgRenderEventArgs& e, pgBindFlag flag) {
	e.pApp->bind(e, flag);

	if (flag == pgBindFlag::pgBindFlag_Mesh) {
		// Set the pipeline state
		pgApp::s_ctx->SetPipelineState(m_pPSO);

		// Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode 
		// makes sure that resources are transitioned to required states.
		pgApp::s_ctx->CommitShaderResources(m_pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}
}

void pgPassRender::unbind(pgRenderEventArgs& e, pgBindFlag flag) {
	//
}

void pgPassRender::Render() {

}