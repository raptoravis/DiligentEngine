#pragma once

#include "pipelinebase.h"

class PipelineDebug : public PipelineBase
{
    typedef PipelineBase base;

  protected:
    virtual void InitPSODesc();

  public:
    PipelineDebug(std::shared_ptr<pgRenderTarget> rt);
    virtual ~PipelineDebug();
};