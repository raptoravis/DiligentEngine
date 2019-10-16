#include "techniqueforward.h"

#include "pass/passopaque.h"
#include "pass/passtransparent.h"
#include "pass/passsetrt.h"
#include "pass/passclearrt.h"
#include "pass/passcopytexture.h"

TechniqueForward::TechniqueForward(const pgTechniqueCreateInfo& ci)
	: base(ci)
{
}

TechniqueForward::~TechniqueForward() {

}

void TechniqueForward::update(pgRenderEventArgs& e) {
	base::update(e);
}

// Render the scene using the passes that have been configured.
void TechniqueForward::render(pgRenderEventArgs& e) {
	base::render(e);
}


void TechniqueForward::init(const pgPassRenderCreateInfo& rpci, const std::vector<pgLight>& lights) {
	PassSetRTCreateInfo psrtci{ *(pgCreateInfo*)&rpci };
	psrtci.rt = m_pRT;
	std::shared_ptr<PassSetRT> pSetRTPass = std::make_shared<PassSetRT>(psrtci);
	addPass(pSetRTPass);

	PassClearRTCreateInfo pcrtci{ *(pgCreateInfo*)&rpci };
	pcrtci.rt = m_pRT;
	std::shared_ptr<PassClearRT> pClearRTPass = std::make_shared<PassClearRT>(pcrtci);
	addPass(pClearRTPass);

	std::shared_ptr<PassOpaque> pOpaquePass = std::make_shared<PassOpaque>(rpci);
	addPass(pOpaquePass);

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