#include "forwardplustechnique.h"

#include "pass/passopaque.h"
#include "pass/passtransparent.h"

ForwardPlusTechnique::ForwardPlusTechnique(const pgTechniqueCreateInfo& ci)
	: base(ci)
{
}

ForwardPlusTechnique::~ForwardPlusTechnique() {

}

void ForwardPlusTechnique::update(pgRenderEventArgs& e) {
	base::update(e);
}

// Render the scene using the passes that have been configured.
void ForwardPlusTechnique::render(pgRenderEventArgs& e) {
	base::render(e);
}


void ForwardPlusTechnique::init(const RenderPassCreateInfo& rpci, const std::vector<pgLight>& lights) {
	OpaquePassCreateInfo opci{ rpci };
	std::shared_ptr<OpaquePass> pOpaquePass = std::make_shared<OpaquePass>(opci);
	addPass(pOpaquePass);

	std::shared_ptr<TransparentPass> pTransparentPass = std::make_shared<TransparentPass>(rpci);
	addPass(pTransparentPass);
}