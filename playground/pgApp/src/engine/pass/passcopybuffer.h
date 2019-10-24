#pragma once

#include "SampleBase.h"

#include "../engine.h"

namespace ade
{

class PassCopyBuffer : public Pass
{
    typedef Pass base;

    std::shared_ptr<Buffer> m_srcBuffer;
    std::shared_ptr<Buffer> m_dstBuffer;

  public:
    PassCopyBuffer(std::shared_ptr<Buffer> dstBuffer, std::shared_ptr<Buffer> srcBuffer);
    virtual ~PassCopyBuffer();

    virtual void Render(Pipeline* pipeline);
};
}    // namespace ade