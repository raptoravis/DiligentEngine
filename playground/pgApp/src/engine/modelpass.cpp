#pragma once

#include "engine.h"
#include "modelpass.h"

pgModelPass::pgModelPass(Diligent::IRenderDevice* device)
	: base()
{
}

pgModelPass::~pgModelPass()
{
}


// Render the pass. This should only be called by the pgTechnique.
void pgModelPass::Update(float CurrTime, float ElapsedTime) {
	//
}

void pgModelPass::Render(Camera* pCamera) {
	//
}
