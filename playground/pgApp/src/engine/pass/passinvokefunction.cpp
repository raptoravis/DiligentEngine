#include "PassInvokeFunction.h"

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
