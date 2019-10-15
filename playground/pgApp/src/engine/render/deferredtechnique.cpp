#include "deferredtechnique.h"

#include "opaquepass.h"
#include "transparentpass.h"
#include "geometrypass.h"
#include "lightpass.h"

DeferredTechnique::DeferredTechnique(const pgTechniqueCreateInfo& ci)
	: base(ci)
{
	createGBuffers();
}

DeferredTechnique::~DeferredTechnique() {

}

void DeferredTechnique::createGBuffers() {
	// Create window-size offscreen render target
	TextureDesc RTColorDesc;
	RTColorDesc.Type = RESOURCE_DIM_TEX_2D;
	RTColorDesc.Width = m_desc.Width;
	RTColorDesc.Height = m_desc.Height;
	RTColorDesc.MipLevels = 1;
	RTColorDesc.Format = TEX_FORMAT_RGBA8_UNORM;
	// The render target can be bound as a shader resource and as a render target
	RTColorDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
	// Define optimal clear value
	RTColorDesc.ClearValue.Format = RTColorDesc.Format;
	RTColorDesc.ClearValue.Color[0] = 0.f;
	RTColorDesc.ClearValue.Color[1] = 0.f;
	RTColorDesc.ClearValue.Color[2] = 0.f;
	RTColorDesc.ClearValue.Color[3] = 1.f;

	RefCntAutoPtr<ITexture> pDiffuseTex;
	m_pDevice->CreateTexture(RTColorDesc, nullptr, &pDiffuseTex);
	// Store the render target view
	m_pDiffuseRTV = pDiffuseTex->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
	m_pDiffuseSRV = pDiffuseTex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

	RefCntAutoPtr<ITexture> pSpecularTex;
	m_pDevice->CreateTexture(RTColorDesc, nullptr, &pSpecularTex);
	// Store the render target view
	m_pSpecularRTV = pSpecularTex->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
	m_pSpecularSRV = pSpecularTex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

	RTColorDesc.Format = TEX_FORMAT_RGBA32_FLOAT;

	RefCntAutoPtr<ITexture> pNormalTex;
	m_pDevice->CreateTexture(RTColorDesc, nullptr, &pNormalTex);
	// Store the render target view
	m_pNormalRTV = pNormalTex->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
	m_pNormalSRV = pNormalTex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

	m_pColorRTV = m_pSwapChain->GetCurrentBackBufferRTV();
	//m_pDSRTV = m_pSwapChain->GetDepthBufferDSV();

	// Create depth buffer
	TextureDesc DepthBufferDesc;
	DepthBufferDesc.Name = "Main depth buffer";
	DepthBufferDesc.Type = RESOURCE_DIM_TEX_2D;
	DepthBufferDesc.Width = m_desc.Width;
	DepthBufferDesc.Height = m_desc.Height;
	DepthBufferDesc.MipLevels = 1;
	DepthBufferDesc.ArraySize = 1;
	DepthBufferDesc.Format = m_desc.DepthBufferFormat;
	DepthBufferDesc.SampleCount = m_desc.SamplesCount;
	DepthBufferDesc.Usage = USAGE_DEFAULT;
	DepthBufferDesc.BindFlags = BIND_DEPTH_STENCIL | BIND_SHADER_RESOURCE;
	DepthBufferDesc.CPUAccessFlags = CPU_ACCESS_NONE;
	DepthBufferDesc.MiscFlags = MISC_TEXTURE_FLAG_NONE;

	m_pDevice->CreateTexture(DepthBufferDesc, nullptr, &m_pDepthBuffer);
	m_pDSRTV = m_pDepthBuffer->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL);
	m_pDSSRV = m_pDepthBuffer->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
}


void DeferredTechnique::update(pgRenderEventArgs& e) {
	base::update(e);
}

// Render the scene using the passes that have been configured.
void DeferredTechnique::render(pgRenderEventArgs& e) {
	base::render(e);
}


void DeferredTechnique::init(const RenderPassCreateInfo& rpci, const std::vector<pgLight>& lights) {
	GeometryPassCreateInfo gpci{ rpci };
	gpci.ColorRTV = m_pColorRTV;
	gpci.DSRTV = m_pDSRTV;
	gpci.DiffuseRTV = m_pDiffuseRTV;
	gpci.SpecularRTV = m_pSpecularRTV;
	gpci.NormalRTV = m_pNormalRTV;

	std::shared_ptr<GeometryPass> pGeometryPass = std::make_shared<GeometryPass>(gpci);
	addPass(pGeometryPass);

	LightPassCreateInfo lpci{ rpci };
	lpci.ColorRTV = m_pColorRTV;
	lpci.DSSRV = m_pDSSRV;
	lpci.DiffuseSRV = m_pDiffuseSRV;
	lpci.SpecularSRV = m_pSpecularSRV;
	lpci.NormalSRV = m_pNormalSRV;
	lpci.Lights = &lights;

	std::shared_ptr<LightPass> pLightPass = std::make_shared<LightPass>(lpci);
	addPass(pLightPass);

	std::shared_ptr<TransparentPass> pTransparentPass = std::make_shared<TransparentPass>(rpci);
	addPass(pTransparentPass);
}