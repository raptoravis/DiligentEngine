#include "techniquedeferred.h"

#include "../pass/passtransparent.h"
#include "../pass/passgeometry.h"
#include "../pass/passlight.h"
#include "../pass/passsetrt.h"
#include "../pass/passclearrt.h"
#include "../pass/passcopytexture.h"

#include "../pipeline/pipelinelightfront.h"
#include "../pipeline/pipelinelightback.h"
#include "../pipeline/pipelinelightdir.h"


TechniqueDeferred::TechniqueDeferred(std::shared_ptr<pgRenderTarget> rt, std::shared_ptr<pgTexture> backBuffer)
	: base(rt, backBuffer)
{
	createGBuffers();
	createBuffers();
}

TechniqueDeferred::~TechniqueDeferred() {

}

void TechniqueDeferred::createGBuffers() {
	// Create window-size offscreen render target
	TextureDesc RTColorDesc;
	RTColorDesc.Name = "GBuffer diffuse";
	RTColorDesc.Type = RESOURCE_DIM_TEX_2D;
	RTColorDesc.Width = pgApp::s_desc.Width;
	RTColorDesc.Height = pgApp::s_desc.Height;
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
	pgApp::s_device->CreateTexture(RTColorDesc, nullptr, &pDiffuseTex);

	std::shared_ptr<pgTexture> diffuseTexture = std::make_shared<pgTexture>(pDiffuseTex);

	RTColorDesc.Name = "GBuffer specular";
	RefCntAutoPtr<ITexture> pSpecularTex;
	pgApp::s_device->CreateTexture(RTColorDesc, nullptr, &pSpecularTex);
	std::shared_ptr<pgTexture> specularTexture = std::make_shared<pgTexture>(pSpecularTex);

	RTColorDesc.Name = "GBuffer normal";
	RTColorDesc.Format = TEX_FORMAT_RGBA32_FLOAT;

	RefCntAutoPtr<ITexture> pNormalTex;
	pgApp::s_device->CreateTexture(RTColorDesc, nullptr, &pNormalTex);

	std::shared_ptr<pgTexture> normalTexture = std::make_shared<pgTexture>(pNormalTex);

	// Create depth buffer
	TextureDesc DepthBufferDesc;
	DepthBufferDesc.Name = "GBuffer depth";
	DepthBufferDesc.Type = RESOURCE_DIM_TEX_2D;
	DepthBufferDesc.Width = pgApp::s_desc.Width;
	DepthBufferDesc.Height = pgApp::s_desc.Height;
	DepthBufferDesc.MipLevels = 1;
	DepthBufferDesc.ArraySize = 1;
	DepthBufferDesc.Format = TEX_FORMAT_D24_UNORM_S8_UINT;
	DepthBufferDesc.SampleCount = pgApp::s_desc.SamplesCount;
	DepthBufferDesc.Usage = USAGE_DEFAULT;
	DepthBufferDesc.BindFlags = BIND_DEPTH_STENCIL | BIND_SHADER_RESOURCE;
	DepthBufferDesc.CPUAccessFlags = CPU_ACCESS_NONE;
	DepthBufferDesc.MiscFlags = MISC_TEXTURE_FLAG_NONE;

	RefCntAutoPtr<ITexture> pDepthStencilTexture;
	pgApp::s_device->CreateTexture(DepthBufferDesc, nullptr, &pDepthStencilTexture);
	m_depthStencilTexture = std::make_shared<pgTexture>(pDepthStencilTexture);

	//
	m_pGBufferRT = std::make_shared<pgRenderTarget>();

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

void TechniqueDeferred::createBuffers() {
	{
		BufferDesc CBDesc;
		CBDesc.Name = "LightParams CB";
		uint32_t bufferSize = sizeof(LightParams);
		CBDesc.uiSizeInBytes = bufferSize;
		CBDesc.Usage = USAGE_DYNAMIC;
		CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
		CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
		pgApp::s_device->CreateBuffer(CBDesc, nullptr, &m_LightParamsCB);
	}

	{
		BufferDesc CBDesc;
		CBDesc.Name = "LightParams CB";
		uint32_t bufferSize = sizeof(ScreenToViewParams);
		CBDesc.uiSizeInBytes = bufferSize;
		CBDesc.Usage = USAGE_DYNAMIC;
		CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
		CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
		pgApp::s_device->CreateBuffer(CBDesc, nullptr, &m_ScreenToViewParamsCB);
	}
}

void TechniqueDeferred::init(const pgPassRenderCreateInfo& prci, const std::vector<pgLight>& lights) {
	//PassSetRTCreateInfo psrtci{ *(pgCreateInfo*)&rpci };
	//psrtci.rt = m_pGBufferRT;
	//std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(psrtci);
	//addPass(pSetRTPass);

	//PassClearRTCreateInfo pcrtci{ *(pgCreateInfo*)&rpci };
	//pcrtci.rt = m_pGBufferRT;
	//std::shared_ptr<PassClearRT> pClearRTPass = std::make_shared<PassClearRT>(pcrtci);
	//addPass(pClearRTPass);
	std::shared_ptr<PassGeometry> pGeometryPass = std::make_shared<PassGeometry>(prci, m_pGBufferRT);
	addPass(pGeometryPass);

	{
		auto srcTexture = m_depthStencilTexture;
		auto dstTexture = m_pRT->GetTexture(pgRenderTarget::AttachmentPoint::DepthStencil);

		std::shared_ptr<PassCopyTexture> pCopyTexPass = std::make_shared<PassCopyTexture>(dstTexture, srcTexture);
		addPass(pCopyTexPass);
	}

	m_pDepthOnlyRT = std::make_shared<pgRenderTarget>();
	m_pDepthOnlyRT->AttachTexture(pgRenderTarget::AttachmentPoint::DepthStencil, 
		m_pRT->GetTexture(pgRenderTarget::AttachmentPoint::DepthStencil));

	std::shared_ptr<pgPipeline>			pFront = std::make_shared<PipelineLightFront>(m_pDepthOnlyRT, prci.PerObjectConstants);

	std::shared_ptr<pgPipeline>			pBack = std::make_shared<PipelineLightBack>(m_pRT, m_pGBufferRT, prci.PerObjectConstants, 
		prci.LightsBufferSRV, m_LightParamsCB.RawPtr(), m_ScreenToViewParamsCB.RawPtr());
	std::shared_ptr<pgPipeline>			pDir = std::make_shared<PipelineLightDir>(m_pRT, m_pGBufferRT, prci.PerObjectConstants,
		prci.LightsBufferSRV, m_LightParamsCB.RawPtr(), m_ScreenToViewParamsCB.RawPtr());

	std::shared_ptr<PassLight> pLightPass = std::make_shared<PassLight>(prci.PerObjectConstants, 
		m_LightParamsCB.RawPtr(), m_ScreenToViewParamsCB.RawPtr(), 
		pFront, pBack, pDir, &lights);
	addPass(pLightPass);

	std::shared_ptr<PassTransparent> pTransparentPass = std::make_shared<PassTransparent>(prci);
	addPass(pTransparentPass);

	{
		auto srcTexture = m_pRT->GetTexture(pgRenderTarget::AttachmentPoint::Color0);
		auto dstTexture = m_pBackBuffer;

		std::shared_ptr<PassCopyTexture> pCopyTexPass = std::make_shared<PassCopyTexture>(dstTexture, srcTexture);
		addPass(pCopyTexPass);
	}
}