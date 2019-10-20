#pragma once

#include "engine/engine.h"

class PipelineBase : public pgPipeline
{
    typedef pgPipeline base;

  protected:
    virtual void InitPSODesc();

  public:
    PipelineBase(std::shared_ptr<pgRenderTarget> rt);
    virtual ~PipelineBase();
};