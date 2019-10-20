#include "pipelinetexvertex.h"

using namespace Diligent;

PipelineTexVertex::PipelineTexVertex(std::shared_ptr<pgRenderTarget> rt) : base(rt)
{
    LoadTexture();

    // CreatePipelineState();
    m_pVS = std::make_shared<Shader>();
    m_pVS->LoadShaderFromFile(Shader::Shader::VertexShader, "cubetex.vsh", "main");

    m_pPS = std::make_shared<Shader>();
    m_pPS->LoadShaderFromFile(Shader::Shader::PixelShader, "cubetex.psh", "main");

    m_pPS->GetShaderParameterByName("g_Texture").SetResource(m_Texture);

    SetShader(Shader::Shader::VertexShader, m_pVS);
    SetShader(Shader::Shader::PixelShader, m_pPS);

    // Define vertex shader input layout
    static LayoutElement LayoutElems[] = { // Attribute 0 - vertex position
                                           LayoutElement{ 0, 0, 3, VT_FLOAT32, False },
                                           // Attribute 1 - texture coordinates
                                           LayoutElement{ 1, 0, 2, VT_FLOAT32, False }
    };

    m_PSODesc.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
    m_PSODesc.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

    m_PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = True;
    m_PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend = BLEND_FACTOR_SRC_ALPHA;
    m_PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend = BLEND_FACTOR_INV_SRC_ALPHA;
    m_PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlendAlpha = BLEND_FACTOR_ZERO;
    m_PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlendAlpha = BLEND_FACTOR_ONE;

    // Variables will be assigned automatically by pgPipeline::Bind
    //// Shader variables should typically be mutable, which means they are expected
    //// to change on a per-instance basis
    // static ShaderResourceVariableDesc Vars[] =
    //{
    //	{SHADER_TYPE_PIXEL, "g_Texture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
    //};
    // m_PSODesc.ResourceLayout.Variables = Vars;
    // m_PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    // Define static sampler for g_Texture. Static samplers should be used whenever possible
    SamplerDesc SamLinearClampDesc{ FILTER_TYPE_LINEAR,    FILTER_TYPE_LINEAR,
                                    FILTER_TYPE_LINEAR,    TEXTURE_ADDRESS_CLAMP,
                                    TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP };
    static StaticSamplerDesc StaticSamplers[] = { { SHADER_TYPE_PIXEL, "g_Texture",
                                                    SamLinearClampDesc } };
    m_PSODesc.ResourceLayout.StaticSamplers = StaticSamplers;
    m_PSODesc.ResourceLayout.NumStaticSamplers = _countof(StaticSamplers);
}

PipelineTexVertex::~PipelineTexVertex() {}

// void PipelineTexVertex::CreatePipelineState()
//{
//	pgApp::s_device->CreatePipelineState(PSODesc, &m_pPSO);
//
//	// Since we did not explcitly specify the type for 'Constants' variable, default
//	// type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables
//	// never change and are bound directly through the pipeline state object.
//	m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(m_VSConstants);
//
//	// Since we are using mutable variable, we must create a shader resource binding object
//	// http://diligentgraphics.com/2016/03/23/resource-binding-model-in-diligent-engine-2-0/
//	m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);
//
//	// Set texture SRV in the SRB
//	m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_Texture")->Set(m_TextureSRV);
//}

void PipelineTexVertex::LoadTexture()
{
    TextureLoadInfo loadInfo;
    loadInfo.IsSRGB = false;
    RefCntAutoPtr<ITexture> Tex;
    // CreateTextureFromFile("DGLogo.png", loadInfo, pgApp::s_device, &Tex);
    CreateTextureFromFile("apple-logo.png", loadInfo, pgApp::s_device, &Tex);

    m_Texture = std::make_shared<pgTexture>(Tex);
}
