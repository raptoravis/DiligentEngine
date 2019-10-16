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
	createGBuffers(*(pgCreateInfo*)&ci);
}

TechniqueDeferred::~TechniqueDeferred() {

}

void TechniqueDeferred::createGBuffers(const pgCreateInfo& ci) {
	// Create window-size offscreen render target
	TextureDesc RTColorDesc;
	RTColorDesc.Name = "GBuffer diffuse";
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

	pgTextureCreateInfo texci{ ci };

	texci.texture = pDiffuseTex;
	std::shared_ptr<pgTexture> diffuseTexture = std::make_shared<pgTexture>(texci);

	RTColorDesc.Name = "GBuffer specular";
	RefCntAutoPtr<ITexture> pSpecularTex;
	m_pDevice->CreateTexture(RTColorDesc, nullptr, &pSpecularTex);
	texci.texture = pSpecularTex;
	std::shared_ptr<pgTexture> specularTexture = std::make_shared<pgTexture>(texci);

	RTColorDesc.Name = "GBuffer normal";
	RTColorDesc.Format = TEX_FORMAT_RGBA32_FLOAT;

	RefCntAutoPtr<ITexture> pNormalTex;
	m_pDevice->CreateTexture(RTColorDesc, nullptr, &pNormalTex);

	texci.texture = pNormalTex;
	std::shared_ptr<pgTexture> normalTexture = std::make_shared<pgTexture>(texci);

	// Create depth buffer
	TextureDesc DepthBufferDesc;
	DepthBufferDesc.Name = "GBuffer depth";
	DepthBufferDesc.Type = RESOURCE_DIM_TEX_2D;
	DepthBufferDesc.Width = m_desc.Width;
	DepthBufferDesc.Height = m_desc.Height;
	DepthBufferDesc.MipLevels = 1;
	DepthBufferDesc.ArraySize = 1;
	DepthBufferDesc.Format = TEX_FORMAT_D24_UNORM_S8_UINT;
	DepthBufferDesc.SampleCount = m_desc.SamplesCount;
	DepthBufferDesc.Usage = USAGE_DEFAULT;
	DepthBufferDesc.BindFlags = BIND_DEPTH_STENCIL | BIND_SHADER_RESOURCE;
	DepthBufferDesc.CPUAccessFlags = CPU_ACCESS_NONE;
	DepthBufferDesc.MiscFlags = MISC_TEXTURE_FLAG_NONE;

	RefCntAutoPtr<ITexture> pDepthStencilTexture;
	m_pDevice->CreateTexture(DepthBufferDesc, nullptr, &pDepthStencilTexture);
	texci.texture = pDepthStencilTexture;
	m_depthStencilTexture = std::make_shared<pgTexture>(texci);

	//
	pgRenderTargetCreateInfo rtci{ ci };
	m_pGBufferRT = std::make_shared<pgRenderTarget>(rtci);

	auto color0 = m_pRT->GetTexture(pgRenderTarget::AttachmentPoint::Color0);

	m_pGBufferRT->AttachTexture(pgRenderTarget::AttachmentPoint::Color0, color0);
	m_pGBufferRT->AttachTexture(pgRenderTarget::AttachmentPoint::Color1, diffuseTexture);
	m_pGBufferRT->AttachTexture(pgRenderTarget::AttachmentPoint::Color2, specularTexture);
	m_pGBufferRT->AttachTexture(pgRenderTarget::AttachmentPoint::Color3, normalTexture);
	m_pGBufferRT->AttachTexture(pgRenderTarget::AttachmentPoint::DepthStencil, m_depthStencilTexture);
}


void TechniqueDeferred::update(pgRenderEventArgs& e) {
	base::update(e);
}

// Render the scene using the passes that have been configured.
void TechniqueDeferred::render(pgRenderEventArgs& e) {
	base::render(e);
}


void TechniqueDeferred::init(const pgPassRenderCreateInfo& rpci, const std::vector<pgLight>& lights) {
	//PassSetRTCreateInfo psrtci{ *(pgCreateInfo*)&rpci };
	//psrtci.rt = m_pGBufferRT;
	//std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(psrtci);
	//addPass(pSetRTPass);

	//PassClearRTCreateInfo pcrtci{ *(pgCreateInfo*)&rpci };
	//pcrtci.rt = m_pGBufferRT;
	//std::shared_ptr<PassClearRT> pClearRTPass = std::make_shared<PassClearRT>(pcrtci);
	//addPass(pClearRTPass);

	GeometryPassCreateInfo gpci{ rpci };
	gpci.rt = m_pGBufferRT;

	std::shared_ptr<PassGeometry> pGeometryPass = std::make_shared<PassGeometry>(gpci);
	addPass(pGeometryPass);

	{
		CopyTexturePassCreateInfo ctpci{ *(pgCreateInfo*)&rpci };
		ctpci.srcTexture = m_depthStencilTexture;
		ctpci.dstTexture = m_pRT->GetTexture(pgRenderTarget::AttachmentPoint::DepthStencil); 

		std::shared_ptr<PassCopyTexture> pCopyTexPass = std::make_shared<PassCopyTexture>(ctpci);
		addPass(pCopyTexPass);
	}

	pgRenderTargetCreateInfo rtci{ rpci };
	m_pDepthOnlyRT = std::make_shared<pgRenderTarget>(rtci);
	m_pDepthOnlyRT->AttachTexture(pgRenderTarget::AttachmentPoint::DepthStencil, 
		m_pRT->GetTexture(pgRenderTarget::AttachmentPoint::DepthStencil));

	pgPipelineCreateInfo plci { *(pgCreateInfo*)&rpci};
	plci.rt = m_pDepthOnlyRT;

	std::shared_ptr<pgPipeline>			pFront = std::make_shared<PipelineLightFront>(plci);

	plci.rt = m_pRT;
	std::shared_ptr<pgPipeline>			pBack = std::make_shared<PipelineLightBack>(plci);
	std::shared_ptr<pgPipeline>			pDir = std::make_shared<PipelineLightDir>(plci);

	LightPassCreateInfo lpci{ rpci };
	lpci.rt = m_pGBufferRT;
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