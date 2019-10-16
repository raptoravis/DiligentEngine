#include "forwardtechnique.h"

#include "pass/passopaque.h"
#include "pass/passtransparent.h"

ForwardTechnique::ForwardTechnique(const pgTechniqueCreateInfo& ci)
	: base(ci)
{
}

ForwardTechnique::~ForwardTechnique() {

}

void ForwardTechnique::update(pgRenderEventArgs& e) {
	base::update(e);
}

// Render the scene using the passes that have been configured.
void ForwardTechnique::render(pgRenderEventArgs& e) {
	base::render(e);
}


void ForwardTechnique::init(const RenderPassCreateInfo& rpci, const std::vector<pgLight>& lights) {

	std::shared_ptr<OpaquePass> pOpaquePass = std::make_shared<OpaquePass>(rpci);
	addPass(pOpaquePass);

	std::shared_ptr<TransparentPass> pTransparentPass = std::make_shared<TransparentPass>(rpci);
	addPass(pTransparentPass);
}