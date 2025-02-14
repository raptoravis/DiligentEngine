#include "techniqueforward.h"

#include "engine/pass/passclearrt.h"
#include "engine/pass/passcopytexture.h"
#include "engine/pass/passsetrt.h"
#include "engine/pipeline/pipelinebase.h"

#include "../pipeline/pipelinetransparent.h"

#include "../pass/passopaque.h"
#include "../pass/passtransparent.h"


TechniqueForward::TechniqueForward(std::shared_ptr<RenderTarget> rt,
                                   std::shared_ptr<Texture> backBuffer)
    : base(rt, backBuffer)
{
}

TechniqueForward::~TechniqueForward() {}

void TechniqueForward::init(std::shared_ptr<Scene> scene, std::vector<Light>* lights)
{
    std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(this, m_pRenderTarget);
    AddPass(pSetRTPass);

    std::shared_ptr<PassClearRT> pClearRTPass =
        std::make_shared<PassClearRT>(this, m_pRenderTarget);
    AddPass(pClearRTPass);

    uint32_t numLights = (uint32_t)lights->size();
    Diligent::ShaderMacroHelper shaderMacros;
    shaderMacros.AddShaderMacro("NUM_LIGHTS", numLights);

    bool bRightHanded = false;

    shaderMacros.AddShaderMacro("RIGHT_HANDED", bRightHanded);

    m_pVertexShader = std::make_shared<Shader>();
    m_pVertexShader->LoadShaderFromFile(Shader::VertexShader, "ForwardRendering.hlsl", "VS_main",
                                        "./resources/shaders", false, shaderMacros);

    m_pPixelShader = std::make_shared<Shader>();
    m_pPixelShader->LoadShaderFromFile(Shader::PixelShader, "ForwardRendering.hlsl", "PS_main",
                                       "./resources/shaders", false, shaderMacros);

    SamplerDesc linearRepeatSampler{ FILTER_TYPE_LINEAR,   FILTER_TYPE_LINEAR,
                                     FILTER_TYPE_LINEAR,   TEXTURE_ADDRESS_WRAP,
                                     TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP };
    SamplerDesc linearClampSampler{ FILTER_TYPE_LINEAR,    FILTER_TYPE_LINEAR,
                                    FILTER_TYPE_LINEAR,    TEXTURE_ADDRESS_CLAMP,
                                    TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP };

    StaticSamplerDesc m_LinearRepeatSamplerDesc{ SHADER_TYPE_PIXEL, "LinearRepeatSampler",
                                                 linearRepeatSampler };
    StaticSamplerDesc m_LinearClampSamplerDesc{ SHADER_TYPE_PIXEL, "LinearClampSampler",
                                                linearClampSampler };

    m_LinearRepeatSampler = std::make_shared<SamplerState>(m_LinearRepeatSamplerDesc);
    m_LinearClampSampler = std::make_shared<SamplerState>(m_LinearClampSamplerDesc);

    m_pPixelShader->GetShaderParameterByName("LinearRepeatSampler").Set(m_LinearRepeatSampler);

    //////////////////////////////////////////////////////////////////////////
    m_pOpaquePipeline = std::make_shared<PipelineBase>(m_pRenderTarget);
    m_pOpaquePipeline->SetShader(Shader::VertexShader, m_pVertexShader);
    m_pOpaquePipeline->SetShader(Shader::PixelShader, m_pPixelShader);
    // m_pOpaquePipeline->SetRenderTarget(m_pRenderTarget);

    std::shared_ptr<PassOpaque> pOpaquePass =
        std::make_shared<PassOpaque>(this, scene, m_pOpaquePipeline, lights);
    AddPass(pOpaquePass);

    //////////////////////////////////////////////////////////////////////////
    m_pTransparentPipeline = std::make_shared<PipelineTransparent>(m_pRenderTarget);
    m_pTransparentPipeline->SetShader(Shader::VertexShader, m_pVertexShader);
    m_pTransparentPipeline->SetShader(Shader::PixelShader, m_pPixelShader);
    // m_pTransparentPipeline->SetRenderTarget(m_pRenderTarget);

    std::shared_ptr<PassTransparent> pTransparentPass =
        std::make_shared<PassTransparent>(this, scene, m_pTransparentPipeline, lights);
    AddPass(pTransparentPass);

    //////////////////////////////////////////////////////////////////////////
    {
        auto srcTexture = m_pRenderTarget->GetTexture(RenderTarget::AttachmentPoint::Color0);
        auto dstTexture = m_pBackBuffer;

        std::shared_ptr<PassCopyTexture> pCopyTexPass =
            std::make_shared<PassCopyTexture>(this, dstTexture, srcTexture);
        AddPass(pCopyTexPass);
    }
}

