#include "PipelineFPOpaque.h"

using namespace Diligent;

PipelineFPOpaque::PipelineFPOpaque(std::shared_ptr<pgRenderTarget> rt) 
	: base(rt)
{
	LoadTexture();
	CreatePipelineState();
}

PipelineFPOpaque::~PipelineFPOpaque() {

}

void PipelineFPOpaque::CreatePipelineState()
{
	// Pipeline state object encompasses configuration of all GPU stages

	PipelineStateDesc PSODesc;
	// Pipeline state name is used by the engine to report issues.
	// It is always a good idea to give objects descriptive names.
	PSODesc.Name = "CubeTex PSO";

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
	pgApp::s_engineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
	ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
	// Create a vertex shader
	RefCntAutoPtr<IShader> pVS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
		ShaderCI.EntryPoint = "main";
		ShaderCI.Desc.Name = "Cube VS";
		ShaderCI.FilePath = "cubetex.vsh";
		pgApp::s_device->CreateShader(ShaderCI, &pVS);
		// Create dynamic uniform buffer that will store our transformation matrix
		// Dynamic buffers can be frequently updated by the CPU
		CreateUniformBuffer(pgApp::s_device, sizeof(float4x4), "VS constants CB", &m_VSConstants);
	}

	// Create a pixel shader
	RefCntAutoPtr<IShader> pPS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
		ShaderCI.EntryPoint = "main";
		ShaderCI.Desc.Name = "Cube PS";
		ShaderCI.FilePath = "cubetex.psh";
		pgApp::s_device->CreateShader(ShaderCI, &pPS);
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

	pgApp::s_device->CreatePipelineState(PSODesc, &m_pPSO);

	// Since we did not explcitly specify the type for 'Constants' variable, default
	// type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables 
	// never change and are bound directly through the pipeline state object.
	m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(m_VSConstants);

	// Since we are using mutable variable, we must create a shader resource binding object
	// http://diligentgraphics.com/2016/03/23/resource-binding-model-in-diligent-engine-2-0/
	m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);

	// Set texture SRV in the SRB
	m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_Texture")->Set(m_TextureSRV);
}

void PipelineFPOpaque::LoadTexture()
{
	TextureLoadInfo loadInfo;
	loadInfo.IsSRGB = false;
	RefCntAutoPtr<ITexture> Tex;
	//CreateTextureFromFile("DGLogo.png", loadInfo, pgApp::s_device, &Tex);
	CreateTextureFromFile("apple-logo.png", loadInfo, pgApp::s_device, &Tex);
	// Get shader resource view from the texture
	m_TextureSRV = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
}

