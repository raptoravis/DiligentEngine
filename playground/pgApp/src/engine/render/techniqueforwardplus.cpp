#include "TechniqueForwardPlus.h"

#include "pass/passopaque.h"
#include "pass/passtransparent.h"
#include "pass/passsetrt.h"
#include "pass/passclearrt.h"
#include "pass/passcopytexture.h"

TechniqueForwardPlus::TechniqueForwardPlus(std::shared_ptr<pgRenderTarget> rt, std::shared_ptr<pgTexture> backBuffer)
	: base(rt, backBuffer)
{
}

TechniqueForwardPlus::~TechniqueForwardPlus() {

}

void TechniqueForwardPlus::update(pgRenderEventArgs& e) {
	base::update(e);
}

// Render the scene using the passes that have been configured.
void TechniqueForwardPlus::render(pgRenderEventArgs& e) {
	base::render(e);
}


void TechniqueForwardPlus::init(const pgPassRenderCreateInfo& prci, const std::vector<pgLight>& lights) {
	g_pVertexShader = std::make_shared<Shader>();
	g_pVertexShader->LoadShaderFromFile(Shader::VertexShader, "ForwardRendering.hlsl", "VS_main");

	g_pForwardPlusPixelShader = std::make_shared<Shader>();
	g_pForwardPlusPixelShader->LoadShaderFromFile(Shader::Shader::VertexShader, "ForwardPlusRendering.hlsl", "PS_main");

	g_pForwardPlusOpaquePipeline = std::make_shared<pgPipeline>(m_pRT);

	g_pForwardPlusOpaquePipeline->SetShader(Shader::VertexShader, g_pVertexShader);
	g_pForwardPlusOpaquePipeline->SetShader(Shader::PixelShader, g_pForwardPlusPixelShader);
	g_pForwardPlusOpaquePipeline->SetRenderTarget(m_pRT);

	Diligent::DepthStencilStateDesc DepthStencilDesc;
	DepthStencilDesc.DepthEnable = True;
	DepthStencilDesc.DepthWriteEnable = True;
	// We need to set depth mode to <= because we have a depth prepass in Forward+
	DepthStencilDesc.DepthFunc = COMPARISON_FUNC_LESS_EQUAL;

	g_pForwardPlusOpaquePipeline->SetDepthStencilState(DepthStencilDesc);


	g_pForwardPlusTransparentPipeline = std::make_shared<pgPipeline>(m_pRT);
	g_pForwardPlusTransparentPipeline->SetShader(Shader::VertexShader, g_pVertexShader);
	g_pForwardPlusTransparentPipeline->SetShader(Shader::PixelShader, g_pForwardPlusPixelShader);
	g_pForwardPlusTransparentPipeline->SetRenderTarget(m_pRT);

	DepthStencilDesc.DepthEnable = True;
	DepthStencilDesc.DepthWriteEnable = False;
	// We need to set depth mode to <= because we have a depth prepass in Forward+
	DepthStencilDesc.DepthFunc = COMPARISON_FUNC_LESS_EQUAL;

	g_pForwardPlusTransparentPipeline->SetDepthStencilState(DepthStencilDesc);

	Diligent::RasterizerStateDesc RasterizerDesc;

	RasterizerDesc.CullMode = CULL_MODE_NONE;
	g_pForwardPlusTransparentPipeline->SetRasterizerState(RasterizerDesc);

	Diligent::BlendStateDesc BlendDesc;

	BlendDesc.RenderTargets[0].BlendEnable = True;
	BlendDesc.RenderTargets[0].SrcBlend = BLEND_FACTOR_SRC_ALPHA;
	BlendDesc.RenderTargets[0].DestBlend = BLEND_FACTOR_INV_SRC_ALPHA;
	BlendDesc.RenderTargets[0].SrcBlendAlpha = BLEND_FACTOR_ZERO;
	BlendDesc.RenderTargets[0].DestBlendAlpha = BLEND_FACTOR_ONE;

	g_pForwardPlusTransparentPipeline->SetBlendState(BlendDesc);

}