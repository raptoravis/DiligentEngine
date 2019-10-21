#pragma once

#include "SampleBase.h"

#include "../engine.h"

class PassSetRT : public pgPass
{
    typedef pgPass base;

    std::shared_ptr<pgRenderTarget> m_pRenderTarget;

  public:
    PassSetRT(pgTechnique* parentTechnique, std::shared_ptr<pgRenderTarget> rt);
    virtual ~PassSetRT();


    virtual void Render();
};
