#pragma once

#include "SampleBase.h"

#include "engine/engine.h"

using namespace Diligent;

namespace ade
{

class PassInvokeFunction : public Pass
{
    typedef Pass base;

    std::function<void(void)> m_Func;

  public:
    PassInvokeFunction(Technique* parentTechnique, std::function<void(void)> func);

    virtual ~PassInvokeFunction();

    virtual void Render(Pipeline* pipeline);
};
}    // namespace ade