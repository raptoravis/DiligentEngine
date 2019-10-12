#pragma once

#include "engine.h"

class pgModelPass : public pgBasePass {
	typedef pgBasePass base;

	pgModelPass(Diligent::IRenderDevice* device, Diligent::IDeviceContext* pCtx, Diligent::IEngineFactory* factory);
	virtual ~pgModelPass();

	// Render the pass. This should only be called by the pgTechnique.
	virtual void Update(float CurrTime, float ElapsedTime);
	virtual void Render(Camera* pCamera);

};

