#include "techniquedeferred.h"

#include "pass/passtransparent.h"
#include "pass/passgeometry.h"
#include "pass/passlight.h"
#include "pass/passsetrt.h"
#include "pass/passclearrt.h"
#include "pass/passcopytexture.h"

#include "pipeline/pipelinelightfront.h"
#include "pipeline/pipelinelightback.h"
#include "pipeline/pipelinelightdir.h"


TechniqueDeferred::TechniqueDeferred(const pgTechniqueCreateInfo& ci)
	: base(ci)
{
	createGBuffers();
}

TechniqueDeferred::~TechniqueDeferred() {

}

void TechniqueDeferred::createGBuffers() {
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


void TechniqueDeferred::update(pgRenderEventArgs& e) {
	base::update(e);
}

// Render the scene using the passes that have been configured.
void TechniqueDeferred::render(pgRenderEventArgs& e) {
	base::render(e);
}


void TechniqueDeferred::init(const pgPassRenderCreateInfo& rpci, const std::vector<pgLight>& lights) {
	PassSetRTCreateInfo psrtci{ *(pgCreateInfo*)&rpci };
	psrtci.rt = m_pRT;
	std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(psrtci);
	addPass(pSetRTPass);

	PassClearRTCreateInfo pcrtci{ *(pgCreateInfo*)&rpci };
	pcrtci.rt = m_pRT;
	std::shared_ptr<PassClearRT> pClearRTPass = std::make_shared<PassClearRT>(pcrtci);
	addPass(pClearRTPass);

	GeometryPassCreateInfo gpci{ rpci };
	gpci.ColorRTV = m_pColorRTV;
	gpci.DSRTV = m_pDSRTV;
	gpci.DiffuseRTV = m_pDiffuseRTV;
	gpci.SpecularRTV = m_pSpecularRTV;
	gpci.NormalRTV = m_pNormalRTV;

	std::shared_ptr<PassGeometry> pGeometryPass = std::make_shared<PassGeometry>(gpci);
	addPass(pGeometryPass);

	pgPipelineCreateInfo plci { *(pgCreateInfo*)&rpci};

	std::shared_ptr<pgPipeline>			pFront = std::make_shared<PipelineLightFront>(plci);
	std::shared_ptr<pgPipeline>			pBack = std::make_shared<PipelineLightBack>(plci);
	std::shared_ptr<pgPipeline>			pDir = std::make_shared<PipelineLightDir>(plci);

	LightPassCreateInfo lpci{ rpci };
	lpci.ColorRTV = m_pColorRTV;
	lpci.DSSRV = m_pDSSRV;
	lpci.DiffuseSRV = m_pDiffuseSRV;
	lpci.SpecularSRV = m_pSpecularSRV;
	lpci.NormalSRV = m_pNormalSRV;
	lpci.Lights = &lights;
	lpci.front = pFront;
	lpci.back = pBack;
	lpci.dir = pDir;

	std::shared_ptr<PassLight> pLightPass = std::make_shared<PassLight>(lpci);
	addPass(pLightPass);

	std::shared_ptr<PassTransparent> pTransparentPass = std::make_shared<PassTransparent>(rpci);
	addPass(pTransparentPass);

	{
		CopyTexturePassCreateInfo ctpci{ *(pgCreateInfo*)&rpci };
		ctpci.srcTexture = m_pRT->GetTexture(pgRenderTarget::AttachmentPoint::Color0);
		ctpci.dstTexture = m_pBackBuffer;

		std::shared_ptr<PassCopyTexture> pCopyTexPass = std::make_shared<PassCopyTexture>(ctpci);
		addPass(pCopyTexPass);
	}
}