#pragma once

#include "engine/pipeline/pipelinebase.h"

class PipelineLightFront : public PipelineBase
{
    typedef PipelineBase base;

  private:
    virtual void InitPSODesc();

  public:
    PipelineLightFront(std::shared_ptr<pgRenderTarget> rt);
    virtual ~PipelineLightFront();
};