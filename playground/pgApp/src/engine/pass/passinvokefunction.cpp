#include "PassInvokeFunction.h"

PassInvokeFunction::PassInvokeFunction(pgTechnique* parentTechnique, std::function<void(void)> func)
    : base(parentTechnique), m_Func(func)
{
}

PassInvokeFunction::~PassInvokeFunction() {}

void PassInvokeFunction::Render()
{
    if (m_Func) {
        m_Func();
    }
}
