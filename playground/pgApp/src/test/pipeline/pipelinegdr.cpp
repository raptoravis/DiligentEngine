#include "pipelinegdr.h"

using namespace Diligent;
using namespace ade;

const char* PipelineGdr::kPerObjectName = "PerObject";
const char* PipelineGdr::kColorsMaterialName = "ColorsMaterial";

PipelineGdr::PipelineGdr(std::shared_ptr<ade::RenderTarget> rt,
                         std::shared_ptr<ConstantBuffer> perObjectCB,
                         std::shared_ptr<ConstantBuffer> colors)
    : base(rt)
{
    m_pVS = std::make_shared<ade::Shader>();
    m_pVS->LoadShaderFromFile(ade::Shader::Shader::VertexShader,
                              "vs_gdr_instanced_indirect_rendering.vsh", "main", "./gdr", false);

    m_pPS = std::make_shared<ade::Shader>();
    m_pPS->LoadShaderFromFile(ade::Shader::Shader::PixelShader,
                              "fs_gdr_instanced_indirect_rendering.psh", "main", "./gdr", false);

	m_pVS->GetShaderParameterByName(kPerObjectName).Set(perObjectCB);
    m_pPS->GetShaderParameterByName(kColorsMaterialName).Set(colors);

    SetShader(ade::Shader::Shader::VertexShader, m_pVS);
    SetShader(ade::Shader::Shader::PixelShader, m_pPS);

    static LayoutElement LayoutElems[] = {
        // Attribute 0 - vertex position
        LayoutElement{ 0, 0, 3, VT_FLOAT32, False },
    };

    m_PSODesc.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
    m_PSODesc.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);
}

PipelineGdr::~PipelineGdr() {}