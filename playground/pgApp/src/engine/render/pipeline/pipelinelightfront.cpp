#include "PipelineLightFront.h"

using namespace Diligent;

PipelineLightFront::PipelineLightFront(const pgPipelineCreateInfo& ci) 
	: base(ci)
{
	CreatePipelineState(ci);
}

PipelineLightFront::~PipelineLightFront() {

}

void PipelineLightFront::CreatePipelineState(const pgPipelineCreateInfo& ci)
{
	// Pipeline state object encompasses configuration of all GPU stages

	PipelineStateDesc PSODesc;
	// Pipeline state name is used by the engine to report issues.
	// It is always a good idea to give objects descriptive names.
	PSODesc.Name = "PipelineLightFront";

	// This is a graphics pipeline
	PSODesc.IsComputePipeline = false;

	// This tutorial will render to a single render target
	PSODesc.GraphicsPipeline.NumRenderTargets = 0;
	// Set render target format which is the format of the swap chain's color buffer
	//PSODesc.GraphicsPipeline.RTVFormats[0] = m_desc.ColorBufferFormat;
	// Set depth buffer format which is the format of the swap chain's back buffer
	PSODesc.GraphicsPipeline.DSVFormat = m_desc.DepthBufferFormat;
	// Primitive topology defines what kind of primitives will be rendered by this pipeline state
	PSODesc.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	// Cull back faces
	PSODesc.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_BACK;
	PSODesc.GraphicsPipeline.RasterizerDesc.DepthClipEnable = True;

	// Enable depth testing
	PSODesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;
	PSODesc.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = False;
	PSODesc.GraphicsPipeline.DepthStencilDesc.DepthFunc = COMPARISON_FUNC_GREATER;
	PSODesc.GraphicsPipeline.DepthStencilDesc.StencilEnable = True;
	PSODesc.GraphicsPipeline.DepthStencilDesc.FrontFace.StencilPassOp = STENCIL_OP_DECR_SAT;

	ShaderCreateInfo ShaderCI;
	// Tell the system that the shader source code is in HLSL.
	// For OpenGL, the engine will convert this into GLSL under the hood.
	ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

	// OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
	ShaderCI.UseCombinedTextureSamplers = true;

	// Create a shader source stream factory to load shaders from files.
	RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
	m_pEngineFactory->CreateDefaultShaderSourceStreamFactory("./resources/shaders", &pShaderSourceFactory);
	ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
	// Create a vertex shader
	RefCntAutoPtr<IShader> pVS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
		ShaderCI.EntryPoint = "VS_main";
		ShaderCI.Desc.Name = "VS";
		ShaderCI.FilePath = "ForwardRendering.hlsl";
		m_pDevice->CreateShader(ShaderCI, &pVS);
		// Create dynamic uniform buffer that will store our transformation matrix
		// Dynamic buffers can be frequently updated by the CPU
		//CreateUniformBuffer(m_pDevice, sizeof(float4x4), "VS constants CB", &m_VSConstants);
	}

	// Create a pixel shader
	RefCntAutoPtr<IShader> pPS;
	//{
	//	ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
	//	ShaderCI.EntryPoint = "main";
	//	ShaderCI.Desc.Name = "Cube PS";
	//	ShaderCI.FilePath = "DeferredRendering.psh";
	//	m_pDevice->CreateShader(ShaderCI, &pPS);
	//}

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

	//// Shader variables should typically be mutable, which means they are expected
	//// to change on a per-instance basis
	//ShaderResourceVariableDesc Vars[] =
	//{
	//	{SHADER_TYPE_PIXEL, "g_Texture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
	//};
	//PSODesc.ResourceLayout.Variables = Vars;
	//PSODesc.ResourceLayout.NumVariables = _countof(Vars);

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

	m_pDevice->CreatePipelineState(PSODesc, &m_pPSO);

	//// Since we did not explcitly specify the type for 'Constants' variable, default
	//// type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables 
	//// never change and are bound directly through the pipeline state object.
	//m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(m_VSConstants);

	// Since we are using mutable variable, we must create a shader resource binding object
	// http://diligentgraphics.com/2016/03/23/resource-binding-model-in-diligent-engine-2-0/
	m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);

	//// Set texture SRV in the SRB
	//m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_Texture")->Set(m_TextureSRV);
}


void PipelineLightFront::updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag) {
	m_pImmediateContext->SetStencilRef(1);
}
