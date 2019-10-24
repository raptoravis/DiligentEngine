#pragma once

#include "SampleBase.h"

#include "../engine.h"

namespace ade
{

class PassCopyBuffer : public pgPass
{
    typedef pgPass base;

    std::shared_ptr<pgBuffer> m_srcBuffer;
    std::shared_ptr<pgBuffer> m_dstBuffer;

  public:
    PassCopyBuffer(std::shared_ptr<pgBuffer> dstBuffer, std::shared_ptr<pgBuffer> srcBuffer);
    virtual ~PassCopyBuffer();

    virtual void Render(pgPipeline* pipeline);
};
}    // namespace ade