#include "../../app.h"
#include "passclearrt.h"

PassClearRT::PassClearRT(const PassClearRTCreateInfo& ci)
	: base(ci)
{
}

PassClearRT::~PassClearRT()
{
}

// Render a frame
void PassClearRT::render(pgRenderEventArgs& e) {
}

void PassClearRT::update(pgRenderEventArgs& e) {
	//
}

void PassClearRT::updateSRB(pgRenderEventArgs& e, pgUpdateSRB_Flag flag) {
}