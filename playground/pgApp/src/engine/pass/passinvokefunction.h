#pragma once

#include "SampleBase.h"

#include "../engine.h"

#include "passrender.h"

using namespace Diligent;

class PassInvokeFunction : public pgPass
{
    typedef pgPass base;

    std::function<void(void)> m_Func;

  public:
    PassInvokeFunction(pgTechnique* parentTechnique, std::function<void(void)> func);

    virtual ~PassInvokeFunction();

    virtual void Render(pgPipeline* pipeline);
};
