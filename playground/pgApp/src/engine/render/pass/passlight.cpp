#include "passlight.h"
#include "../../scene/sceneass.h"

#include "../../mat2quat.h"

PassLight::PassLight(const LightPassCreateInfo& ci)
	: base(ci)
	, m_pGBufferRT(ci.rt)
	, m_pLights(ci.Lights)
	, m_LightPipeline0(ci.front)
	, m_LightPipeline1(ci.back)
	, m_DirectionalLightPipeline(ci.dir)
{
	PipelineStateDesc PSODesc;

	CreatePipelineState(ci, PSODesc);

	pgSceneCreateInfo sci{*(pgCreateInfo*)&ci};

	m_pPointLightScene = pgSceneAss::CreateSphere(sci, 1.0f);
	m_pSpotLightScene = pgSceneAss::CreateCylinder(sci, 0.0f, 1.0f, 1.0f, float3(0, 0, 1));
	m_pDirectionalLightScene = pgSceneAss::CreateScreenQuad(sci, -1, 1, -1, 1, -1);
}

PassLight::~PassLight()
{
}

void PassLight::CreatePipelineState(const pgPassRenderCreateInfo& ci, PipelineStateDesc& PSODesc) {
	// Pipeline state object encompasses configuration of all GPU stages

	// Pipeline state name is used by the engine to report issues.
	// It is always a good idea to give objects descriptive names.
	PSODesc.Name = "PSO";

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
	}

	// Create a pixel shader
	RefCntAutoPtr<IShader> pPS;
	{
		ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
		ShaderCI.EntryPoint = "PS_DeferredLighting";
		ShaderCI.Desc.Name = "LightPS";
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
		{SHADER_TYPE_PIXEL, "Lights", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}, 
		{SHADER_TYPE_PIXEL, "DiffuseTextureVS", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
		{SHADER_TYPE_PIXEL, "SpecularTextureVS", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
		{SHADER_TYPE_PIXEL, "NormalTextureVS", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
		{SHADER_TYPE_PIXEL, "DepthTextureVS", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
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

	{
		BufferDesc CBDesc;
		CBDesc.Name = "LightParams CB";
		uint32_t bufferSize = sizeof(LightParams);
		CBDesc.uiSizeInBytes = bufferSize;
		CBDesc.Usage = USAGE_DYNAMIC;
		CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
		CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
		m_pDevice->CreateBuffer(CBDesc, nullptr, &m_LightParamsCB);
	}

	{
		BufferDesc CBDesc;
		CBDesc.Name = "LightParams CB";
		uint32_t bufferSize = sizeof(ScreenToViewParams);
		CBDesc.uiSizeInBytes = bufferSize;
		CBDesc.Usage = USAGE_DYNAMIC;
		CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
		CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
		m_pDevice->CreateBuffer(CBDesc, nullptr, &m_ScreenToViewParamsCB);
	}

	// Since we did not explcitly specify the type for 'Constants' variable, default
	// type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never 
	// change and are bound directly through the pipeline state object.
	m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "PerObject")->Set(ci.PerObjectConstants);
	m_pPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "LightIndexBuffer")->Set(m_LightParamsCB);
	m_pPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "ScreenToViewParams")->Set(m_ScreenToViewParamsCB);

	// Create a shader resource binding object and bind all static resources in it
	m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);

	m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "Lights")->Set(ci.LightsBufferSRV);
	auto diffuseTex = m_pGBufferRT->GetTexture(pgRenderTarget::AttachmentPoint::Color1);
	m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "DiffuseTextureVS")->Set(diffuseTex->getTexture());

	auto specularTex = m_pGBufferRT->GetTexture(pgRenderTarget::AttachmentPoint::Color2);
	m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "SpecularTextureVS")->Set(specularTex->getTexture());

	auto normalTex = m_pGBufferRT->GetTexture(pgRenderTarget::AttachmentPoint::Color3);
	m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "NormalTextureVS")->Set(normalTex->getTexture());

	auto depthTex = m_pGBufferRT->GetTexture(pgRenderTarget::AttachmentPoint::DepthStencil);
	m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "DepthTextureVS")->Set(depthTex->getTexture());
}

bool PassLight::meshFilter(pgMesh* mesh) {
	auto mat = mesh->getMaterial();
	auto bTransparent = mat->IsTransparent();
	return !bTransparent;
}


void PassLight::updateLightParams(pgRenderEventArgs& e, const LightParams& lightParam, const pgLight& light) {
	{
		// Map the buffer and write current world-view-projection matrix
		MapHelper<LightParams> CBConstants(e.pDeviceContext, m_LightParamsCB, MAP_WRITE, MAP_FLAG_DISCARD);

		CBConstants->m_LightIndex = lightParam.m_LightIndex;
	}

	{
		// Map the buffer and write current world-view-projection matrix
		MapHelper<PerObject> CBConstants(e.pDeviceContext, m_PerObjectConstants, MAP_WRITE, MAP_FLAG_DISCARD);

		if (light.m_Type == pgLight::LightType::Directional) {
			//CBConstants->ModelViewProjection = m_WorldViewProjMatrix.Transpose();
			//CBConstants->ModelView = m_WorldViewMatrix.Transpose();
			bool IsGL = m_pDevice->GetDeviceCaps().IsGLDevice();
			Diligent::float4x4 othoMat = Diligent::float4x4::Ortho((float)m_desc.Width, (float)m_desc.Height, 0.f, 1.f, IsGL);
			CBConstants->ModelViewProjection = othoMat;
			CBConstants->ModelView = float4x4::Identity();
		}
		else {
			auto& Proj = e.pCamera->getProjectionMatrix();
			//const float4x4 nodeTransform = e.pSceneNode->getLocalTransform();
			const float4x4 nodeTransform = Diligent::float4x4::Identity();

			Diligent::float4x4 translation = Diligent::float4x4::Translation(Diligent::float3(light.m_PositionWS));
			// Create a rotation matrix that rotates the model towards the direction of the light.
			Diligent::float4x4 rotation = MakeQuaternionFromTwoVec3(Diligent::float3(0, 0, 1), normalize(Diligent::float3(light.m_DirectionWS))).ToMatrix();

			// Compute the scale depending on the light type.
			float scaleX, scaleY, scaleZ;
			// For point lights, we want to scale the geometry by the range of the light.
			scaleX = scaleY = scaleZ = light.m_Range;
			if (light.m_Type == pgLight::LightType::Spot)
			{
				// For spotlights, we want to scale the base of the cone by the spotlight angle.
				scaleX = scaleY = std::tan((PI_F / 180.0f) * (light.m_SpotlightAngle)) * light.m_Range;
			}

			Diligent::float4x4 scale = Diligent::float4x4::Scale(Diligent::float3(scaleX, scaleY, scaleZ));

			Diligent::float4x4 modelViewMat = nodeTransform * scale * rotation * translation * e.pCamera->getViewMatrix();
			CBConstants->ModelView = modelViewMat;
			CBConstants->ModelViewProjection = modelViewMat * Proj;
		}
	}
}

void PassLight::updateScreenToViewParams(pgRenderEventArgs& e) {
	{
		// Map the buffer and write current world-view-projection matrix
		MapHelper<ScreenToViewParams> CBConstants(e.pDeviceContext, m_ScreenToViewParamsCB, MAP_WRITE, MAP_FLAG_DISCARD);

		auto& Proj = e.pCamera->getProjectionMatrix();
		//CBConstants->ModelViewProjection = m_WorldViewProjMatrix.Transpose();
		//CBConstants->ModelView = m_WorldViewMatrix.Transpose();
		CBConstants->m_InverseProjectionMatrix = Proj.Inverse();
		CBConstants->m_ScreenDimensions = float2((float)m_desc.Width, (float)m_desc.Height);
	}
}

// Render a frame
void PassLight::render(pgRenderEventArgs& e) {
	updateScreenToViewParams(e);

	if (m_pLights) {
		LightParams	lightParams;

		lightParams.m_LightIndex = 0;

		for (const pgLight& light : *m_pLights) {
			if (light.m_Enabled) {
				// Update the constant buffer for the per-light data.
				updateLightParams(e, lightParams, light);

				// Clear the stencil buffer for the next light
				m_LightPipeline0->getRenderTarget()->Clear(pgClearFlags::Stencil, 
					Diligent::float4(0,0,0,0), 1.0f, 1);
				// The other pipelines should have the same render target.. so no need to clear it 3 times.

				switch (light.m_Type)
				{
				case pgLight::LightType::Point:
					RenderSubPass(e, m_pPointLightScene, m_LightPipeline0);
					RenderSubPass(e, m_pPointLightScene, m_LightPipeline1);
					break;
				case pgLight::LightType::Spot:
					RenderSubPass(e, m_pSpotLightScene, m_LightPipeline0);
					RenderSubPass(e, m_pSpotLightScene, m_LightPipeline1);
					break;
				case pgLight::LightType::Directional:
					RenderSubPass(e, m_pDirectionalLightScene, m_DirectionalLightPipeline);
					break;
				}
			}
			lightParams.m_LightIndex++;
		}
	}
}

void PassLight::RenderSubPass(pgRenderEventArgs& e, std::shared_ptr<pgScene> scene, std::shared_ptr<pgPipeline> pipeline)
{
	pipeline->bind(e, pgBindFlag::pgBindFlag_Pipeline);

	scene->render(e);
}

void PassLight::update(pgRenderEventArgs& e) {
	//
}

void PassLight::bind(pgRenderEventArgs& e, pgBindFlag flag) {
	base::bind(e, flag);
}
