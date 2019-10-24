#include "PassInvokeFunction.h"

namespace ade
{

PassInvokeFunction::PassInvokeFunction(pgTechnique* parentTechnique, std::function<void(void)> func)
    : base(parentTechnique), m_Func(func)
{
}

PassInvokeFunction::~PassInvokeFunction() {}

void PassInvokeFunction::Render(pgPipeline* pipeline)
{
    if (m_Func) {
        m_Func();
    }
}
}    // namespace ade