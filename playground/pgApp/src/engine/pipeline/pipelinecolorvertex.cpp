#include "pipelinecolorvertex.h"

using namespace Diligent;

namespace ade
{

PipelineColorVertex::PipelineColorVertex(std::shared_ptr<pgRenderTarget> rt) : base(rt)
{
    // CreatePipelineState();
    m_pVS = std::make_shared<Shader>();
    m_pVS->LoadShaderFromFile(Shader::Shader::VertexShader, "cube.vsh", "main", "", true);

    m_pPS = std::make_shared<Shader>();
    m_pPS->LoadShaderFromFile(Shader::Shader::PixelShader, "cube.psh", "main", "", true);

    SetShader(Shader::Shader::VertexShader, m_pVS);
    SetShader(Shader::Shader::PixelShader, m_pPS);

    static LayoutElement LayoutElems[] = { // Attribute 0 - vertex position
                                           LayoutElement{ 0, 0, 3, VT_FLOAT32, False },
                                           // Attribute 1 - vertex color
                                           LayoutElement{ 1, 0, 4, VT_FLOAT32, False }
    };

    m_PSODesc.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
    m_PSODesc.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);
}

PipelineColorVertex::~PipelineColorVertex() {}

}    // namespace ade