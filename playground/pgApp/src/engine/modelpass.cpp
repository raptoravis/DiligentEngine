#pragma once

#include "engine.h"
#include "modelpass.h"

pgModelPass::pgModelPass(Diligent::IRenderDevice* device, Diligent::IDeviceContext* pCtx, Diligent::IEngineFactory* factory)
	: base(device, pCtx, factory)
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
