#include "techniqueforward.h"

#include "../pipeline/pipelinetransparent.h"
#include "../pipeline/pipelinebase.h"

#include "../pass/passclearrt.h"
#include "../pass/passcopytexture.h"
#include "../pass/passopaque.h"
#include "../pass/passsetrt.h"
#include "../pass/passtransparent.h"

TechniqueForward::TechniqueForward(std::shared_ptr<pgRenderTarget> rt,
                                   std::shared_ptr<pgTexture> backBuffer)
    : base(rt, backBuffer)
{
}

TechniqueForward::~TechniqueForward() {}

void TechniqueForward::init(std::shared_ptr<pgScene> scene, const std::vector<pgLight>& lights)
{
    std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(this, m_pRT);
    addPass(pSetRTPass);

    std::shared_ptr<PassClearRT> pClearRTPass = std::make_shared<PassClearRT>(this, m_pRT);
    addPass(pClearRTPass);

    g_pVertexShader = std::make_shared<Shader>();
    g_pVertexShader->LoadShaderFromFile(Shader::VertexShader, "ForwardRendering.hlsl", "VS_main",
                                        "./resources/shaders");

    g_pPixelShader = std::make_shared<Shader>();
    g_pPixelShader->LoadShaderFromFile(Shader::PixelShader, "ForwardRendering.hlsl", "PS_main",
                                       "./resources/shaders");

    g_pOpaquePipeline = std::make_shared<PipelineBase>(m_pRT);
    g_pOpaquePipeline->SetShader(Shader::VertexShader, g_pVertexShader);
    g_pOpaquePipeline->SetShader(Shader::PixelShader, g_pPixelShader);
    g_pOpaquePipeline->SetRenderTarget(m_pRT);

    std::shared_ptr<PassOpaque> pOpaquePass =
        std::make_shared<PassOpaque>(this, scene, g_pOpaquePipeline, lights);
    addPass(pOpaquePass);

    g_pTransparentPipeline = std::make_shared<PipelineTransparent>(m_pRT);
    g_pTransparentPipeline->SetShader(Shader::VertexShader, g_pVertexShader);
    g_pTransparentPipeline->SetShader(Shader::PixelShader, g_pPixelShader);
    g_pTransparentPipeline->SetRenderTarget(m_pRT);

    std::shared_ptr<PassTransparent> pTransparentPass =
        std::make_shared<PassTransparent>(this, scene, g_pTransparentPipeline, lights);
    addPass(pTransparentPass);

    {
        auto srcTexture = m_pRT->GetTexture(pgRenderTarget::AttachmentPoint::Color0);
        auto dstTexture = m_pBackBuffer;

        std::shared_ptr<PassCopyTexture> pCopyTexPass =
            std::make_shared<PassCopyTexture>(this, dstTexture, srcTexture);
        addPass(pCopyTexPass);
    }
}