#include "forwardplustechnique.h"

#include "opaquepass.h"
#include "transparentpass.h"

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

	std::shared_ptr<OpaquePass> pOpaquePass = std::make_shared<OpaquePass>(rpci);
	addPass(pOpaquePass);

	std::shared_ptr<TransparentPass> pTransparentPass = std::make_shared<TransparentPass>(rpci);
	addPass(pTransparentPass);
}