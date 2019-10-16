#pragma once

#include "SampleBase.h"
#include "BasicMath.h"

#include "engine.h"
#include "render/light.h"

class pgApp : public Diligent::SampleBase {
protected:
	std::shared_ptr<pgCamera>							m_pCamera;
	pgRenderEventArgs									m_evtArgs;

public:
	pgApp();
	virtual ~pgApp();

	virtual void Initialize(Diligent::IEngineFactory*   pEngineFactory,
		Diligent::IRenderDevice*    pDevice,
		Diligent::IDeviceContext**  ppContexts,
		Diligent::Uint32            NumDeferredCtx,
		Diligent::ISwapChain*       pSwapChain) override;

	virtual void Render() override;
	virtual void Update(double CurrTime, double ElapsedTime) override ;
	virtual const Diligent::Char* GetSampleName()const override { return "pgApp"; }

	virtual void bind(pgRenderEventArgs& e, pgBindFlag flag) = 0;
};