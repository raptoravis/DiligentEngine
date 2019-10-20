#include "PipelineBase.h"

using namespace Diligent;

PipelineBase::PipelineBase(std::shared_ptr<pgRenderTarget> rt) 
	: base(rt)
{
}

PipelineBase::~PipelineBase() {

}

void PipelineBase::InitPSODesc()
{
    // Pipeline state object encompasses configuration of all GPU stages

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    m_PSODesc.Name = "PSO";

    // This is a graphics pipeline
    m_PSODesc.IsComputePipeline = false;

    // This tutorial will render to a single render target
    m_PSODesc.GraphicsPipeline.NumRenderTargets = 1;

    auto color0 = m_pRT->GetTexture(pgRenderTarget::AttachmentPoint::Color0);
    auto color0Format = color0->GetTexture()->GetDesc().Format;

    auto ds = m_pRT->GetTexture(pgRenderTarget::AttachmentPoint::DepthStencil);
    auto dsFormat = ds->GetTexture()->GetDesc().Format;

    // Set render target format which is the format of the swap chain's color buffer
    m_PSODesc.GraphicsPipeline.RTVFormats[0] = color0Format;
    // Set depth buffer format which is the format of the swap chain's back buffer
    m_PSODesc.GraphicsPipeline.DSVFormat = dsFormat;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    m_PSODesc.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    // Cull back faces
    m_PSODesc.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_BACK;
    // Enable depth testing
    m_PSODesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;

    ShaderCreateInfo ShaderCI;
    // Tell the system that the shader source code is in HLSL.
    // For OpenGL, the engine will convert this into GLSL under the hood.
    ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

    // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture +
    // g_Texture_sampler combination)
    ShaderCI.UseCombinedTextureSamplers = false;

    // In this tutorial, we will load shaders from file. To be able to do that,
    // we need to create a shader source stream factory
    //RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
    //pgApp::s_engineFactory->CreateDefaultShaderSourceStreamFactory("./resources/shaders",
    //                                                               &pShaderSourceFactory);
    //ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    //// Create a vertex shader
    //RefCntAutoPtr<IShader> pVS;
    //{
    //    ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
    //    ShaderCI.EntryPoint = "VS_main";
    //    ShaderCI.Desc.Name = "OpaqueVS";
    //    ShaderCI.FilePath = "ForwardRendering.hlsl";
    //    pgApp::s_device->CreateShader(ShaderCI, &pVS);
    //}

    //// Create a pixel shader
    //RefCntAutoPtr<IShader> pPS;
    //{
    //    ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
    //    ShaderCI.EntryPoint = "PS_main";
    //    ShaderCI.Desc.Name = "OpaquePS";
    //    ShaderCI.FilePath = "ForwardRendering.hlsl";
    //    pgApp::s_device->CreateShader(ShaderCI, &pPS);
    //}

    // Define vertex shader input layout
    static LayoutElement LayoutElems[] = {
        LayoutElement{ 0, 0, 3, VT_FLOAT32, False },    // position
        LayoutElement{ 0, 1, 3, VT_FLOAT32, False },    // tangent
        LayoutElement{ 0, 2, 3, VT_FLOAT32, False },    // binormal
        LayoutElement{ 0, 3, 3, VT_FLOAT32, False },    // normal
        LayoutElement{ 0, 4, 2, VT_FLOAT32, False },    // tex
    };

    LayoutElems[0].SemanticName = "POSITION";
    LayoutElems[1].SemanticName = "TANGENT";
    LayoutElems[2].SemanticName = "BINORMAL";
    LayoutElems[3].SemanticName = "NORMAL";
    LayoutElems[4].SemanticName = "TEXCOORD";

    m_PSODesc.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
    m_PSODesc.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

    //m_PSODesc.GraphicsPipeline.pVS = pVS;
    //m_PSODesc.GraphicsPipeline.pPS = pPS;

    // Define variable type that will be used by default
    m_PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    //// Shader variables should typically be mutable, which means they are expected
    //// to change on a per-instance basis
    //ShaderResourceVariableDesc Vars[] = { { SHADER_TYPE_PIXEL, "Lights",
    //                                        SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE } };
    //m_PSODesc.ResourceLayout.Variables = Vars;
    //m_PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    //// Define static sampler for g_Texture. Static samplers should be used whenever possible
    // StaticSamplerDesc StaticSamplers[] =
    //{
    //	{ SHADER_TYPE_PIXEL, "g_Texture", Sam_LinearClamp }
    //};
    // RTm_PSODesc.ResourceLayout.StaticSamplers = StaticSamplers;
    // RTm_PSODesc.ResourceLayout.NumStaticSamplers = _countof(StaticSamplers);

    //
    //pgApp::s_device->CreatePipelineState(m_PSODesc, &m_pPSO);

    //// Since we did not explcitly specify the type for 'Constants' variable, default
    //// type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
    //// change and are bound directly through the pipeline state object.
    //m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "PerObject")->Set(m_PerObjectConstants);
    //m_pPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "Material")->Set(m_MaterialConstants);
    //// m_pPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "Lights")->Set(pLightsBufferSRV);

    //// Create a shader resource binding object and bind all static resources in it
    //m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);

    //m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "Lights")->Set(m_LightsBufferSRV);
}
