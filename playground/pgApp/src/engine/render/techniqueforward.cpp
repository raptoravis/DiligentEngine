#include "techniqueforward.h"

#include "pass/passopaque.h"
#include "pass/passtransparent.h"

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

	std::shared_ptr<PassOpaque> pOpaquePass = std::make_shared<PassOpaque>(rpci);
	addPass(pOpaquePass);

	std::shared_ptr<PassTransparent> pTransparentPass = std::make_shared<PassTransparent>(rpci);
	addPass(pTransparentPass);
}