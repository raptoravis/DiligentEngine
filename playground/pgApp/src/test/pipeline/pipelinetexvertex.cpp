#include "pipelinetexvertex.h"

using namespace Diligent;


PipelineTexVertex::PipelineTexVertex(std::shared_ptr<ade::RenderTarget> rt) : base(rt)
{
    LoadTexture();

    // CreatePipelineState();
    m_pVS = std::make_shared<ade::Shader>();
    m_pVS->LoadShaderFromFile(ade::Shader::Shader::VertexShader, "cubetex.vsh", "main");

    m_pPS = std::make_shared<ade::Shader>();
    m_pPS->LoadShaderFromFile(ade::Shader::Shader::PixelShader, "cubetex.psh", "main", "", true);

    SetShader(ade::Shader::Shader::VertexShader, m_pVS);
    SetShader(ade::Shader::Shader::PixelShader, m_pPS);
}

PipelineTexVertex::~PipelineTexVertex() {}

void PipelineTexVertex::InitPSODesc()
{
    base::InitPSODesc();

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

    // Variables will be assigned automatically by Pipeline::Bind
    //// Shader variables should typically be mutable, which means they are expected
    //// to change on a per-instance basis
    // static ShaderResourceVariableDesc Vars[] =
    //{
    //	{SHADER_TYPE_PIXEL, "m_Texture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
    //};
    // m_PSODesc.ResourceLayout.Variables = Vars;
    // m_PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    // Define static sampler for m_Texture. Static samplers should be used whenever possible
    // SamplerDesc SamLinearClampDesc{ FILTER_TYPE_LINEAR,    FILTER_TYPE_LINEAR,
    //                                FILTER_TYPE_LINEAR,    TEXTURE_ADDRESS_CLAMP,
    //                                TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP };
    // static StaticSamplerDesc StaticSamplers[] = { { SHADER_TYPE_PIXEL, "m_Texture",
    //                                                SamLinearClampDesc } };
    // m_PSODesc.ResourceLayout.StaticSamplers = StaticSamplers;
    // m_PSODesc.ResourceLayout.NumStaticSamplers = _countof(StaticSamplers);

    SamplerDesc linearClampSampler{ FILTER_TYPE_LINEAR,    FILTER_TYPE_LINEAR,
                                    FILTER_TYPE_LINEAR,    TEXTURE_ADDRESS_CLAMP,
                                    TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP };

    StaticSamplerDesc m_LinearClampSamplerDesc{ SHADER_TYPE_PIXEL, "g_Texture",
                                                linearClampSampler };

    m_LinearClampSampler = std::make_shared<ade::SamplerState>(m_LinearClampSamplerDesc);

    m_pPS->GetShaderParameterByName("g_Texture_sampler").Set(m_LinearClampSampler);

    m_pPS->GetShaderParameterByName("g_Texture").Set(m_Texture);
}

void PipelineTexVertex::LoadTexture()
{
    TextureLoadInfo loadInfo;
    loadInfo.IsSRGB = false;
    RefCntAutoPtr<ITexture> Tex;
    // CreateTextureFromFile("DGLogo.png", loadInfo, App::s_device, &Tex);
    CreateTextureFromFile("apple-logo.png", loadInfo, ade::App::s_device, &Tex);

    m_Texture = std::make_shared<ade::Texture>(Tex);
}

