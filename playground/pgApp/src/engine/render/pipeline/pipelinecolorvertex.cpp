#include "pipelinecolorvertex.h"

using namespace Diligent;

PipelineColorVertex::PipelineColorVertex(const pgPipelineCreateInfo& ci) 
	: base(ci)
{
	CreatePipelineState(ci);
}

PipelineColorVertex::~PipelineColorVertex() {

}

void PipelineColorVertex::CreatePipelineState(const pgPipelineCreateInfo& ci)
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
	ShaderCI.UseCombinedTextureSamplers = true;

	// In this tutorial, we will load shaders from file. To be able to do that,
	// we need to create a shader source stream factory
	RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
	m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
	ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
	// Create a vertex shader
	RefCntAutoPtr<IShader> pVS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
		ShaderCI.EntryPoint = "main";
		ShaderCI.Desc.Name = "Cube VS";
		ShaderCI.FilePath = "cube.vsh";
		m_pDevice->CreateShader(ShaderCI, &pVS);
		// Create dynamic uniform buffer that will store our transformation matrix
		// Dynamic buffers can be frequently updated by the CPU
		BufferDesc CBDesc;
		CBDesc.Name = "VS constants CB";
		CBDesc.uiSizeInBytes = sizeof(float4x4);
		CBDesc.Usage = USAGE_DYNAMIC;
		CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
		CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
		m_pDevice->CreateBuffer(CBDesc, nullptr, &m_VSConstants);
	}

	// Create a pixel shader
	RefCntAutoPtr<IShader> pPS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
		ShaderCI.EntryPoint = "main";
		ShaderCI.Desc.Name = "Cube PS";
		ShaderCI.FilePath = "cube.psh";
		m_pDevice->CreateShader(ShaderCI, &pPS);
	}

	// Define vertex shader input layout
	LayoutElement LayoutElems[] =
	{
		// Attribute 0 - vertex position
		LayoutElement{0, 0, 3, VT_FLOAT32, False},
		// Attribute 1 - vertex color
		LayoutElement{1, 0, 4, VT_FLOAT32, False}
	};
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
	m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(m_VSConstants);

	// Create a shader resource binding object and bind all static resources in it
	m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);
}

void PipelineColorVertex::bind(pgRenderEventArgs& e, pgBindFlag flag) {
	{
		if (flag == pgBindFlag::pgBindFlag_Mesh) {
			const float4x4 view = e.pCamera->getViewMatrix();

			const float4x4 local = e.pSceneNode->getLocalTransform();

			// Set cube world view matrix
			float4x4 CubeWorldView = float4x4::RotationY(static_cast<float>(e.CurrTime) * -1.0f) * local * view;

			auto& Proj = e.pCamera->getProjectionMatrix();

			// Compute world-view-projection matrix
			float4x4 worldViewProjMatrix = CubeWorldView * Proj;

			{
				// Map the buffer and write current world-view-projection matrix
				MapHelper<float4x4> CBConstants(m_pImmediateContext, m_VSConstants, MAP_WRITE, MAP_FLAG_DISCARD);
				*CBConstants = worldViewProjMatrix.Transpose();
			}

			// Set the pipeline state
			m_pImmediateContext->SetPipelineState(m_pPSO);

			// Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode 
			// makes sure that resources are transitioned to required states.
			m_pImmediateContext->CommitShaderResources(m_pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		}
	}
}

