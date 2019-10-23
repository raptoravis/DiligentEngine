#include "pipelinebase.h"

using namespace Diligent;

PipelineBase::PipelineBase(std::shared_ptr<pgRenderTarget> rt) : base(rt) {}

PipelineBase::~PipelineBase() {}

void PipelineBase::InitPSODesc()
{
    // Pipeline state object encompasses configuration of all GPU stages

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    m_PSODesc.Name = "PipelineBase PSO";

    // This is a graphics pipeline
    m_PSODesc.IsComputePipeline = false;

    auto color0 = m_pRenderTarget->GetTexture(pgRenderTarget::AttachmentPoint::Color0);
    auto color0Format =
        color0 ? color0->GetTexture()->GetDesc().Format : Diligent::TEX_FORMAT_UNKNOWN;

    auto ds = m_pRenderTarget->GetTexture(pgRenderTarget::AttachmentPoint::DepthStencil);
    auto dsFormat = ds ? ds->GetTexture()->GetDesc().Format : pgApp::s_desc.DepthBufferFormat;

    // This tutorial will render to a single render target
    m_PSODesc.GraphicsPipeline.NumRenderTargets = (uint8_t)m_pRenderTarget->GetNumRTVs();

    // Set render target format which is the format of the swap chain's color buffer
    m_PSODesc.GraphicsPipeline.RTVFormats[0] = color0Format;
    // Set depth buffer format which is the format of the swap chain's back buffer
    m_PSODesc.GraphicsPipeline.DSVFormat = dsFormat;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    m_PSODesc.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Cull back faces
    m_PSODesc.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_BACK;

#if RIGHT_HANDED
    m_PSODesc.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = True;
#else
    m_PSODesc.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = False;
#endif

    // Enable depth testing
    m_PSODesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;

    ShaderCreateInfo ShaderCI;
    // Tell the system that the shader source code is in HLSL.
    // For OpenGL, the engine will convert this into GLSL under the hood.
    ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

    // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture +
    // g_Texture_sampler combination)
    ShaderCI.UseCombinedTextureSamplers = false;


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

    // m_PSODesc.GraphicsPipeline.pVS = pVS;
    // m_PSODesc.GraphicsPipeline.pPS = pPS;

    // Define variable type that will be used by default
    m_PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
}

