#include "techniqueforward.h"

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

void TechniqueForward::update(pgRenderEventArgs& e)
{
    base::update(e);
}

// Render the scene using the passes that have been configured.
void TechniqueForward::render(pgRenderEventArgs& e)
{
    base::render(e);
}


void TechniqueForward::init(pgPassRenderCreateInfo& prci, const std::vector<pgLight>& lights)
{
    pgApp::s_reourceNames[pgApp::RESOURCE_SLOT_CB_PEROBJECT] = "PerObject";
    pgApp::s_reources[pgApp::RESOURCE_SLOT_CB_PEROBJECT] = prci.PerObjectConstants;
    pgApp::s_reourceNames[pgApp::RESOURCE_SLOT_CB_MATERIAL] = "Material";
    pgApp::s_reources[pgApp::RESOURCE_SLOT_CB_MATERIAL] = prci.MaterialConstants;
    pgApp::s_reourceNames[pgApp::RESOURCE_SLOT_SB_LIGHTS] = "Lights";
    pgApp::s_reources[pgApp::RESOURCE_SLOT_SB_LIGHTS] = prci.LightsStructuredBuffer;

    std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(m_pRT);
    addPass(pSetRTPass);

    std::shared_ptr<PassClearRT> pClearRTPass = std::make_shared<PassClearRT>(m_pRT);
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

    prci.pipeline = g_pOpaquePipeline;
    std::shared_ptr<PassOpaque> pOpaquePass = std::make_shared<PassOpaque>(prci);
    addPass(pOpaquePass);

    g_pTransparentPipeline = std::make_shared<PipelineBase>(m_pRT);
    g_pTransparentPipeline->SetShader(Shader::VertexShader, g_pVertexShader);
    g_pTransparentPipeline->SetShader(Shader::PixelShader, g_pPixelShader);

    Diligent::BlendStateDesc alphaBlending;

    alphaBlending.RenderTargets[0].BlendEnable = True;
    alphaBlending.RenderTargets[0].SrcBlend = BLEND_FACTOR_SRC_ALPHA;
    alphaBlending.RenderTargets[0].DestBlend = BLEND_FACTOR_INV_SRC_ALPHA;
    alphaBlending.RenderTargets[0].SrcBlendAlpha = BLEND_FACTOR_ZERO;
    alphaBlending.RenderTargets[0].DestBlendAlpha = BLEND_FACTOR_ONE;

    g_pTransparentPipeline->SetBlendState(alphaBlending);

    Diligent::DepthStencilStateDesc DepthStencilDesc;
    DepthStencilDesc.DepthEnable = True;
    DepthStencilDesc.DepthWriteEnable = False;
    DepthStencilDesc.DepthFunc = COMPARISON_FUNC_LESS_EQUAL;

    g_pTransparentPipeline->SetDepthStencilState(DepthStencilDesc);

    Diligent::RasterizerStateDesc RasterizerDesc;

    RasterizerDesc.CullMode = CULL_MODE_NONE;
    g_pTransparentPipeline->SetRasterizerState(RasterizerDesc);
    g_pTransparentPipeline->SetRenderTarget(m_pRT);

    prci.pipeline = g_pTransparentPipeline;

    std::shared_ptr<PassTransparent> pTransparentPass = std::make_shared<PassTransparent>(prci);
    addPass(pTransparentPass);

    {
        auto srcTexture = m_pRT->GetTexture(pgRenderTarget::AttachmentPoint::Color0);
        auto dstTexture = m_pBackBuffer;

        std::shared_ptr<PassCopyTexture> pCopyTexPass =
            std::make_shared<PassCopyTexture>(dstTexture, srcTexture);
        addPass(pCopyTexPass);
    }
}