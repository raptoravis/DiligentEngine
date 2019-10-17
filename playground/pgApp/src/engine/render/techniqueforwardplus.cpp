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
	std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(m_pRT);
	addPass(pSetRTPass);

	std::shared_ptr<PassClearRT> pClearRTPass = std::make_shared<PassClearRT>(m_pRT);
	addPass(pClearRTPass);

	std::shared_ptr<PassOpaque> pOpaquePass = std::make_shared<PassOpaque>(prci);
	addPass(pOpaquePass);

	std::shared_ptr<PassTransparent> pTransparentPass = std::make_shared<PassTransparent>(prci);
	addPass(pTransparentPass);

	{
		auto srcTexture = m_pRT->GetTexture(pgRenderTarget::AttachmentPoint::Color0);
		auto dstTexture = m_pBackBuffer;

		std::shared_ptr<PassCopyTexture> pCopyTexPass = std::make_shared<PassCopyTexture>(dstTexture, srcTexture);
		addPass(pCopyTexPass);
	}
}