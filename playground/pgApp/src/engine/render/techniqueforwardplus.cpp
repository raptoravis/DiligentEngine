#include "techniqueforwardplus.h"

#include "pass/passopaque.h"
#include "pass/passtransparent.h"

TechniqueForwardPlus::TechniqueForwardPlus(const pgTechniqueCreateInfo& ci)
	: base(ci)
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


void TechniqueForwardPlus::init(const pgPassRenderCreateInfo& rpci, const std::vector<pgLight>& lights) {
	OpaquePassCreateInfo opci{ rpci };
	std::shared_ptr<PassOpaque> pOpaquePass = std::make_shared<PassOpaque>(opci);
	addPass(pOpaquePass);

	std::shared_ptr<PassTransparent> pTransparentPass = std::make_shared<PassTransparent>(rpci);
	addPass(pTransparentPass);
}