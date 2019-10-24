#pragma once

#include "pipelinebase.h"

namespace ade
{

class PipelineDebug : public PipelineBase
{
    typedef PipelineBase base;

  protected:
    virtual void InitPSODesc();

  public:
    PipelineDebug(std::shared_ptr<RenderTarget> rt);
    virtual ~PipelineDebug();
};

}    // namespace ade